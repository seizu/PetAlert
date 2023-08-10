#include <Arduino.h>
#include <EEPROM.h>
#include <ESPAsyncWebServer.h>

#if defined(ESP8266)
  #include <ESPAsyncTCP.h>
  #include <ESP8266HTTPClient.h>
  #include <WiFiClientSecure.h>
  #define ESP_SLEEP_WAKEUP_EXT0 REASON_EXT_SYS_RST
  #define ESP_SLEEP_WAKEUP_EXT1 REASON_EXT_SYS_RST
  #define ESP_SLEEP_WAKEUP_TIMER REASON_DEEP_SLEEP_AWAKE
#elif defined(ESP32)
  #include <AsyncTCP.h>
  #include <HTTPClient.h>
  #include <esp_wifi.h>
#endif

#include <string>
#include "serlog.h"
#include "SimpleWifi/SimpleWifi.h"
#include "WebPrefs/WebPrefs.h"
#include "config.h"

#define WIFI_MAX_RETRIES 30
// power off 24 hours
#define POWER_OFF_TIME 24 * 3600 * 1000000ull
// idle timeout 5 minutes
#define WEBSERVER_MAX_IDLE 1000 * 60 * 5

// default access point prefix & password 
#define AP_PREFIX "PET"
#define AP_PASSWD "petalert"

#define VERSION "1.2.0"

// GPIO PINS ----------------------------
#if defined(ESP8266)
  #define INT_PIN D2
  #define LED_PIN D5
  #define APMODE_PIN D6
  #define BAT_PIN A0
#elif defined(ESP32)
  #define INT_PIN 4
  #define LED_PIN 25
  //#define LED_PIN 22
  #define APMODE_PIN 33
  #define BAT_PIN 34
  #define BUTTON_PIN_BITMASK 0x1 << INT_PIN
#endif

uint8_t blink = LOW;
bool valid_config = false;
bool ap_mode = false;
bool client_mode = false;
ulong next_sleep = 0;
uint16_t detection_count = 0;

SimpleWiFi *sw;
WebPrefs *wp;

bool read_config();
bool write_config();

void save();
void done();

int get_wakeup_reason();
void deep_sleep(uint64_t timer = 0);

bool http_get(String url);
String get_bat_status();
String prepare_alert_link();
uint16_t compute_checksum(uint8_t *data, size_t size);

void setup() {
    Serial.begin(115200);
    delay(1000);
    Serial.printf("PetAlert - Firmware V%s\n", VERSION);

    pinMode(INT_PIN, INPUT);
    pinMode(LED_PIN, OUTPUT);
    pinMode(BAT_PIN, INPUT);
    digitalWrite(LED_PIN, LOW);
#if defined(ESP32)
    pinMode(APMODE_PIN, INPUT_PULLDOWN); // PULLDOWN not supported by ESP8266 (use 10K resistor GND)
#endif

    device_config = new _device_config();

    // read & verify EEPROM config
    if ((valid_config = read_config())) {
        LOG_SINFO("Config read OK! ChkSum: 0x%X\n", device_config->check_sum);
    } else {
        LOG_SERROR("Config read NOK! ChkSum: 0x%X\n", device_config->check_sum);
    }

    // init WIFI
    sw = new SimpleWiFi(AP_PREFIX);

    // init WebPrefs Server on port 80
    wp = new WebPrefs(80, (void *)device_config, input_fields);
    // set callback functions for save and done button
    wp->save = save;
    wp->done = done;

    if (valid_config == false) {
        // use defaults config
        LOG_SINFO("Set default config");
        wp->set_defaults();
    }

    int wakeup_reason = get_wakeup_reason();
    LOG_SINFO("WAKEUP CODE %d\n", wakeup_reason);
    if (wakeup_reason == 0) {
        // activate web interface on PowerOn or Reset
        next_sleep = WEBSERVER_MAX_IDLE + millis();
        // enable AP mode only when APMODE_PIN is HIGH (connect APMODE_PIN pin and 3V3 pin with 10K Ohm resistor)
        if (digitalRead(APMODE_PIN) == HIGH) {
            if (sw->init_ap_mode(nullptr, AP_PASSWD)) {
                // no web authentication in AP mode
                // wp->enableAuthentication(device_config->web_user, device_config->web_passwd);
                wp->start();
                ap_mode = true;
                LOG_SINFO("AP MODE");
                delay(5000);
                return;
            }
        } else {
            // try to establish WLAN (client mode)
            if (sw->init_client_mode(device_config->wlan_ssid, device_config->wlan_passwd, WIFI_MAX_RETRIES)) {
                wp->enableAuthentication(device_config->web_user, device_config->web_passwd);
                wp->start();
                client_mode = true;
                digitalWrite(LED_PIN, HIGH);
                LOG_SINFO("CLIENT MODE");
                delay(5000);
                return;
            }
        }
    } else if (wakeup_reason == ESP_SLEEP_WAKEUP_EXT1) {
        // set attention timer
        next_sleep = device_config->atimer * 1000 + millis();
        return;
    }

    if (!device_config->power)
        // power off sleep 24 hours
        deep_sleep(POWER_OFF_TIME);
    // RTC trigger sleep
    deep_sleep();
}

void loop() {
    ulong now = millis();
    bool connected;
    if (now < next_sleep) {
        // webserver config mode ?
        if (ap_mode || client_mode) {
            if (ap_mode) {
                delay(1000);
                blink ^= HIGH;
                digitalWrite(LED_PIN, blink);
                LOG_SDEBUG("Blink %d\n", blink);
            }
        } else {
            if (digitalRead(INT_PIN)) {
                // movement detection ---
                LOG_SINFO("Detection count: %d\n", detection_count);
                if (device_config->led) {
                    digitalWrite(LED_PIN, HIGH);
                    delay(100);
                    digitalWrite(LED_PIN, LOW);
                }

                if (++detection_count >= device_config->dthreshold) {
                    // activate WiFi and establish a WLAN connection
                    if ((connected = sw->init_client_mode(device_config->wlan_ssid, device_config->wlan_passwd, WIFI_MAX_RETRIES))) {
                        // send push notification
                        if (http_get(prepare_alert_link()) && connected) {
                            // TIMER sleep on message sent (in microseconds)
                            deep_sleep(device_config->stimer * 1000000ull);
                        } else {
                            // RTC sleep on error
                            LOG_SERROR("Push notification faild");
                            deep_sleep();
                        }
                    }
                }
                // reset attention timer at each detection.
                next_sleep = now + device_config->atimer * 1000;
                delay(device_config->ddelay * 1000);
            }
        }
    } else {
        if (!device_config->power)
            deep_sleep(POWER_OFF_TIME); // power off
        deep_sleep();
    }
}

// store config in flash memory
void save() {
    if (write_config()) {
        LOG_SINFO("Config written! ChkSum: 0x%X\n", device_config->check_sum);
    } else {
        LOG_SERROR("Save config failed!");
    }
    // reset webserver idle timer
    next_sleep = millis() + WEBSERVER_MAX_IDLE;
}

// exit main loop, go to sleep
void done() {
    next_sleep = 0;
    LOG_SINFO("done");
}

// prepare text & url, replace placeholder $token$, $bat$, $text$
String prepare_alert_link() {
    String url = device_config->alert_link;
    String alert_text = device_config->alert_text;
    url.replace("$token$", device_config->token);
    alert_text.replace("$bat$", get_bat_status());
    url.replace("$text$", wp->url_encode(alert_text));
    return url;
}

// read battery voltage
String get_bat_status() {
    double bat = 0;
    // voltage measurements
    int mea = 10;
    for (int t = 0; t < mea; t++) {
        uint16_t dvalue = analogRead(BAT_PIN);
        bat += dvalue;
        delay(100);
    }
    // calculate avarage
    bat = bat / mea;
    LOG_SDEBUG("bat value: %f\n", bat);

    bat = bat - 30.0f; // <-- adjust this offset for voltage calibration
                       // volt=f(bat) for range 1.395V-1.714V
    double volt = (bat * bat) * -0.0000180767730347157f + (bat * 0.89958301442874f) + 37.1819880715802f;

    LOG_SDEBUG("bat value/Volt: %f/%f\n", bat, volt * 2 / 1000.0f);
    // LOG_SINFO("Voltage: %f", bat * 0.0018); // linear calculation, adjust value (0.0018) for your needs
    return String(volt * 2 / 1000.0f, 3);
}

bool http_get(String url) {
    bool result = false;
#if defined(ESP8266)
    WiFiClientSecure client;
    client.setInsecure();
    HTTPClient http;
    http.begin(client, url.c_str());
#elif defined(ESP32)
    HTTPClient http;
    http.begin(url.c_str());
#endif
    int httpResponseCode = http.GET();

    if (httpResponseCode > 0) {
        if (httpResponseCode >= 400) {
            LOG_SERROR("HTTP Response code: %d\n", httpResponseCode);
        } else {
            result = true;
        }
        String payload = http.getString();
        LOG_SDEBUG("Payload: %s\n", payload.c_str());
    } else {
        LOG_SERROR("HTTP GET - Can't connect");
        result = false;
    }
    // Free resources
    http.end();
    return result;
}

void deep_sleep(uint64_t timer) {
    delete wp;
    WiFi.disconnect();

    if (timer) {
        LOG_SINFO("Going to deep sleep with timer");
#if defined(ESP32)
        esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH, ESP_PD_OPTION_OFF);
        esp_sleep_pd_config(ESP_PD_DOMAIN_XTAL, ESP_PD_OPTION_OFF);
        esp_sleep_enable_timer_wakeup(timer);
#endif
    } else {
        LOG_SINFO("Going to deep sleep with trigger pin");
#if defined(ESP32)
        esp_sleep_enable_ext1_wakeup(BUTTON_PIN_BITMASK, ESP_EXT1_WAKEUP_ANY_HIGH);
#endif
    }

    LOG_SINFO("WiFi Stop");
#if defined(ESP8266)
    if (WiFi.mode(WIFI_OFF)) {
#elif defined(ESP32)
    // optional
    // LOG_SINFO("esp_bt_controller_disable");
    // esp_bt_controller_disable();
    if (ESP_OK == esp_wifi_stop()) {
#endif
        LOG_SINFO("WiFi stopped, deep sleep in 3sec...");
        delay(3000);
    } else {
        LOG_SINFO("WiFi can not be stopped or nothing to stop");
    }
    LOG_SINFO("Deep sleep start");
#if defined(ESP8266)
    ESP.deepSleep(timer);
#elif defined(ESP32)
    esp_deep_sleep_start();
#endif
}

int get_wakeup_reason() {
#if defined(ESP8266)
    int wakeup_reason = ESP.getResetInfoPtr()->reason;
    LOG_SINFO("Wake UP Reason: %s\n", ESP.getResetReason().c_str());
    switch (wakeup_reason) {
    case REASON_DEFAULT_RST:
        LOG_SINFO("Power On");
        break;
    case REASON_EXT_SYS_RST:
        LOG_SINFO("External System");
        break;
    case REASON_DEEP_SLEEP_AWAKE:
        LOG_SINFO("Deep Seep Aawake");
        break;
#elif defined(ESP32)
    esp_sleep_wakeup_cause_t wakeup_reason;
    wakeup_reason = esp_sleep_get_wakeup_cause();

    switch (wakeup_reason) {
    case ESP_SLEEP_WAKEUP_EXT0:
        LOG_SINFO("Wakeup caused by RTC_IO");
        break;
    case ESP_SLEEP_WAKEUP_EXT1:
        LOG_SINFO("Wakeup caused by RTC_CNTL");
        break;
    case ESP_SLEEP_WAKEUP_TIMER:
        LOG_SINFO("Wakeup caused by timer");
        break;
    case ESP_SLEEP_WAKEUP_TOUCHPAD:
        LOG_SINFO("Wakeup caused by touchpad");
        break;
    case ESP_SLEEP_WAKEUP_ULP:
        LOG_SINFO("Wakeup caused by ULP program");
        break;
#endif
    default:
        LOG_SINFO("Wakeup was not caused by deep sleep !!!");
        break;
    }
    return wakeup_reason;
}

bool write_config() {
    uint8_t *buffer = (uint8_t *)device_config;
    size_t size = sizeof(_device_config);
    device_config->check_sum = compute_checksum((buffer + 6), size - 6);
    snprintf(device_config->header, 4, "%s", "PET");
    EEPROM.begin(size);
    for (size_t i = 0; i < size; i++) {
        EEPROM.write(i, buffer[i]);
    }
    return EEPROM.commit();
}

bool read_config() {
    uint8_t *buffer = (uint8_t *)device_config;
    size_t size = sizeof(_device_config);
    EEPROM.begin(size);
    for (size_t i = 0; i < size; i++) {
        buffer[i] = EEPROM.read(i);
    }
    uint16_t chksum = compute_checksum((buffer + 6), size - 6);
    if (chksum == device_config->check_sum && strncmp(device_config->header, "PET\0", 4) == 0) {
        return true;
    }
    return false;
}

uint16_t compute_checksum(uint8_t *data, size_t size) {
    uint16_t checksum = size;
    while (size) {
        checksum += *data++ * size--;
    }
    return checksum;
}
