#include "../serlog.h"

#define AP_DEFAULT_IP "10.100.10.1"
#if defined(ESP8266)
#include <ESP8266WiFi.h>
#elif defined(ESP32)
#include <WiFi.h>
#endif

class SimpleWiFi {
  public:
    SimpleWiFi(const char *ap_prefix = "", const char *ip = AP_DEFAULT_IP);
    bool init_client_mode(const char *ssid, const char *passwd, int retries = 30);
    bool init_ap_mode(const char *ssid, const char *passwd, int channel = 1, int hidden = 0, int max_connections = 4);

  private:
    uint32_t mac;
    char ap_ssid[33];
    IPAddress ip;
    // IPAddress gw;
    // IPAddress dns;
};