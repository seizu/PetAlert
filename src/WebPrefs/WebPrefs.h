#include <ESPAsyncWebServer.h>
#if defined(ESP8266)
	#include <ESPAsyncTCP.h>
#elif defined(ESP32)
	#include <AsyncTCP.h>
#endif
#include <vector>
#include "css.h"
#include "js.h"
#include "html.h"
#include "../serlog.h"

class WebPrefs {
  public:
    enum {
        STRING = 1,
        FLOAT,
        INTEGER,
        PASSWORD,
        CHECKBOX
    };

    struct input_field {
        const char *name;
        const char *value;
        uint8_t type;
        uint8_t size;
        uint16_t offset;
    };

    WebPrefs(int port, void *prefs, const std::vector<input_field> &_fields);
    ~WebPrefs();
    void start();
    void stop();
    void set_defaults();
    void onDataReuqest(AsyncWebServerRequest *request);
    void onDataReceive(AsyncWebServerRequest *request);
    void onIndex(AsyncWebServerRequest *request);
    void onDone(AsyncWebServerRequest *request);
    void notFound(AsyncWebServerRequest *request);
    void enableAuthentication(const char *_user, const char *_password = "");
    void disableAuthentication();
    String url_encode(const String &str) const;
    String url_decode(const String &str, bool decodePlus = false) const;
    std::function<void()> done;
    std::function<void()> save;

  private:
    AsyncWebServer *server;
    bool check_credentials(AsyncWebServerRequest *request);
    bool set_value(const String &value, const String &name) const;
    void set_value(const String &value, int index) const;
    String get_value(int index) const;

    uint32_t start_time;
    std::vector<input_field> fields;
    const char *user;
    const char *password;
    void *prefs;
    bool ap_mode;
    bool isAuth;
};