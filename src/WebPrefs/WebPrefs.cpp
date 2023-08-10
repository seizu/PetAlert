#include "WebPrefs.h"

WebPrefs::WebPrefs(int port, void *_prefs, const std::vector<input_field> &_fields) {
    isAuth = false;
    prefs = _prefs;
    fields = _fields;
    save = nullptr;
    done = nullptr;
    server = new AsyncWebServer(port);
}

WebPrefs::~WebPrefs() {
    stop();
    server->end();
    delete server;
}

void WebPrefs::start() {
    if (server != nullptr) {
        server->on("/postForm", HTTP_POST, std::bind(&WebPrefs::onDataReceive, this, std::placeholders::_1));
        server->on("/getJson", HTTP_GET, std::bind(&WebPrefs::onDataReuqest, this, std::placeholders::_1));
        server->on("/done", HTTP_GET, std::bind(&WebPrefs::onDone, this, std::placeholders::_1));
        server->on("/", HTTP_GET, std::bind(&WebPrefs::onIndex, this, std::placeholders::_1));
        server->onNotFound(std::bind(&WebPrefs::notFound, this, std::placeholders::_1));
        server->begin();
    }
}

void WebPrefs::stop() {
    if (server != nullptr) {
        server->reset();
    }
}

void WebPrefs::set_defaults() {
    for (size_t i = 0; i < fields.size(); i++) {
        set_value(fields[i].value, i);
    }
}

void WebPrefs::enableAuthentication(const char *_user, const char *_password) {
    user = _user;
    password = _password;
    isAuth = true;
}

void WebPrefs::disableAuthentication() {
    isAuth = false;
}

// callback function that handles a GET request to the "/getJson" route
// and sends a JSON response containing the values of all the fields in the fields vector.
void WebPrefs::onDataReuqest(AsyncWebServerRequest *request) {
    LOG_SDEBUG("onDataReuqest");
    String json_data;
    String jvalues;

    if (request->hasParam("fnc")) {
        if (request->getParam("fnc")->value().equalsIgnoreCase("done")) {
            if (done != nullptr) {
                request->send(200);
                return done();
            }
        } else {
            json_data = "{$$$}";
            for (size_t t = 0; t < fields.size(); t++) {
                jvalues += get_value(t) + ",";
            }
            jvalues.remove(jvalues.length() - 1);
            json_data.replace("$$$", jvalues);
        }
    }
    request->send(200, "application/json", json_data);
}

// callback function that handles a POST request to the "/postForm" route
// and sets the values of the fields in the fields vector from the request data.
void WebPrefs::onDataReceive(AsyncWebServerRequest *request) {
    LOG_SDEBUG("onDataReceive");
    if (!check_credentials(request))
        return;

    int params = request->params();
    for (int i = 0; i < params; i++) {
        AsyncWebParameter *p = request->getParam(i);
        // LOG_SDEBUG("POST[%s]: %s\n", p->name(), p->value());
        set_value(p->value(), i);
    }
    request->send(200);

    if (save != nullptr)
        save();
}

// callback function that handles a GET request to the "/"
// route and sends an HTML response containing the settings page.
void WebPrefs::onIndex(AsyncWebServerRequest *request) {
    LOG_SDEBUG("onIndex");
    if (!check_credentials(request))
        return;
    AsyncResponseStream *response = request->beginResponseStream("text/html");
    response->printf("%s %s %s %s", html_head, script_head, css_head, settings_body);
    request->send(response);
}

void WebPrefs::onDone(AsyncWebServerRequest *request) {
    LOG_SDEBUG("onDone");
    if (!check_credentials(request))
        return;
    AsyncResponseStream *response = request->beginResponseStream("text/html");
    response->printf("%s %s %s %s", html_head, script_head, css_head, done_body);
    request->send(response);
}

// callback function that handles a request when the route
// is not found and sends an HTML response containing the 404 not found HTML.
void WebPrefs::notFound(AsyncWebServerRequest *request) {
    if (!check_credentials(request))
        return;
    request->send(404, "text/plain", "Not found");
}

// checks if the request has proper credentials (username and password) if authentication is enabled
bool WebPrefs::check_credentials(AsyncWebServerRequest *request) {
    if (isAuth) {
        if (!request->authenticate(user, password)) {
            request->requestAuthentication();
            return false;
        }
    }
    return true;
}

bool WebPrefs::set_value(const String &value, const String &name) const {
    for (size_t i = 0; i < fields.size(); i++) {
        if (name.equalsIgnoreCase(fields[i].name)) {
            set_value(value, i);
            return true;
        }
    }
    return false;
}

void WebPrefs::set_value(const String &value, int index) const {
    char *_conf = ((char *)prefs) + fields[index].offset;

    if (fields[index].type == WebPrefs::PASSWORD && strncmp(value.c_str(), "***", 3) == 0)
        return;
    if (fields[index].type == WebPrefs::STRING || fields[index].type == WebPrefs::PASSWORD) {
        LOG_SDEBUG("String: %s Value: %s\n", fields[index].name, value.c_str());
        if (strncmp(_conf, value.c_str(), fields[index].size) != 0) {
            snprintf(_conf, fields[index].size, "%s", value.c_str());
        }
    } else if (fields[index].type == WebPrefs::INTEGER) {
        LOG_SDEBUG("Integer: %s Value: %s\n", fields[index].name, value.c_str());
        *(uint16_t *)_conf = (uint16_t)value.toInt();
    } else if (fields[index].type == WebPrefs::CHECKBOX) {
        LOG_SDEBUG("Checkbox: %s Value: %s\n", fields[index].name, value.c_str());
        *(bool *)_conf = (bool)value.equalsIgnoreCase("on");
    }
}

String WebPrefs::get_value(int index) const {
    String jvalue;
    String value;

    if (index >= 0 && index < (int)fields.size()) {
        const char *_conf = (char *)prefs;

        if (fields[index].type == WebPrefs::STRING) {
            value = url_encode(_conf + fields[index].offset);
        } else if (fields[index].type == WebPrefs::PASSWORD) {
            value = "***";
        } else if (fields[index].type == WebPrefs::INTEGER) {
            value = url_encode(String(*(uint16_t *)(_conf + fields[index].offset)));
        } else if (fields[index].type == WebPrefs::CHECKBOX) {
            (*(bool *)(_conf + fields[index].offset)) ? value = "on" : value = "off";
        }
        jvalue = "\"";
        jvalue += fields[index].name;
        jvalue += "\":\"";
        jvalue += value + "\"";
    }
    return jvalue;
}

String WebPrefs::url_encode(const String &str) const {
    String estr;
    char c;
    const char *chars = str.c_str();
    char hexval[4];
    int len = strlen(chars);

    for (int i = 0; i < len; i++) {
        c = chars[i];
        if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~')
            estr += c;
        else {
            snprintf(hexval, 4, "%%%02X", c);
            estr += hexval;
        }
    }
    return estr;
}

String WebPrefs::url_decode(const String &str, bool decodePlus) const {
    String dstr;
    int i, ii, len = str.length();

    for (i = 0; i < len; i++) {
        if (str[i] != '%') {
            if (str[i] == '+' && decodePlus)
                dstr += ' ';
            else
                dstr += str[i];
        } else {
            sscanf(str.substring(i + 1, i + 3).c_str(), "%x", &ii);
            dstr += static_cast<char>(ii);
            i = i + 2;
        }
    }
    return dstr;
}