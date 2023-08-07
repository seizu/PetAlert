
#include "SimpleWifi.h"

// Init SSID name and AP IP
// ap_prefix + mac address = AP SSID NAME
SimpleWiFi::SimpleWiFi(const char * ap_prefix, const char * ip) {
    #if defined(ESP8266)
      mac = ESP.getChipId();
    #elif defined(ESP32)
      uint64_t _mac = ESP.getEfuseMac();
      mac = uint32_t(_mac >> 24);
    #endif
		LOG_SINFO("MAC: %06X\n", mac);
    if (!this->ip.fromString(ip)) {
      this->ip.fromString(AP_DEFAULT_IP);
      LOG_SINFO("Using default IP");    
    }
		snprintf(ap_ssid, 33, "%s%06X", ap_prefix, mac);
}

// Init WiFI as access point
// if ssid == nullptr default AP SSID NAME is used.
bool SimpleWiFi::init_ap_mode(const char *ssid, const char *passwd, int channel, int hidden, int max_connections) {
    bool err = false;
    if (ssid != nullptr) {
			snprintf(ap_ssid, 33, "%s", ssid);
      LOG_SINFO("Unique SSID");
    }
    LOG_SINFO("AP SSID: %s\n",ap_ssid);
    if (WiFi.mode(WIFI_AP)) {
		//if (WiFi.mode(WIFI_AP_STA)) {
      IPAddress subnet(255, 255, 255, 0);
			WiFi.setHostname(ap_ssid);
      if (WiFi.softAPConfig(ip, ip, subnet)) {
        if (WiFi.softAP(ap_ssid, passwd, channel, hidden, max_connections)) {
          //IPAddress IP = WiFi.softAPIP();
          LOG_SINFO("AP IP address: %s\n",ip.toString().c_str());
          return true;
        }
      }
    }
		LOG_SERROR("Error init AP MODE");
    return err;
}

// Init WiFI as station, client IP is assigned by DHCP. Hostname is set to AP SSID NAME
bool SimpleWiFi::init_client_mode(const char *ssid, const char *passwd, int retries) {
  LOG_SINFO("Connecting to SSID: %s\n",ssid);  
	LOG_SINFO(ap_ssid);
  WiFi.setHostname(ap_ssid);
  WiFi.begin(ssid, passwd);
  Serial.print("Connecting ...");
  for (int t = 0; t < retries; t++) {
    if (WiFi.status() == WL_CONNECTED) {      
			ip = WiFi.localIP();
      Serial.print("\n");
      LOG_SINFO("WiFi connected - assigned IP address: %s\n",ip.toString().c_str());
      return true;
    } else {
      delay(1000);
      Serial.print(".");
    }
  }
  Serial.print("\n");
	LOG_SERROR("Error init CLIENT MODE");
  return false;
}