// EEPROM prefs data
// --------------------------------------------------------------
struct _device_config {
    char header[4];
    uint16_t check_sum;
    char wlan_ssid[33];
    char wlan_passwd[64];
    char web_user[17];
    char web_passwd[64];
    char alert_link[193];
    char alert_text[129];
    char token[101];
    uint16_t ddelay;     // detection delay (seconds)
    uint16_t dthreshold; // detection threshold (counts)
    uint16_t stimer;     // sleep timer (seconds)
    uint16_t atimer;     // attention timer (seconds)
    bool led;            // led
    bool power;          // power off/on
} *device_config;
// --------------------------------------------------------------

// default prefs
const std::vector<WebPrefs::input_field> input_fields = {
    {"wlanssid", "WLAN_SSID", WebPrefs::STRING, sizeof(_device_config::wlan_ssid), offsetof(_device_config, wlan_ssid)},
    {"wlanpasswd", "WLAN_PASSWORD", WebPrefs::PASSWORD, sizeof(_device_config::wlan_passwd), offsetof(_device_config, wlan_passwd)},
    {"webuser", "admin", WebPrefs::STRING, sizeof(_device_config::web_user), offsetof(_device_config, web_user)},
    {"webpasswd", "password", WebPrefs::PASSWORD, sizeof(_device_config::web_passwd), offsetof(_device_config, web_passwd)},
    {"alertlink", "https://api.telegram.org/bot$token$/sendMessage?chat_id=YOUR_CHAT_ID&text=$text$", WebPrefs::STRING, sizeof(_device_config::alert_link), offsetof(_device_config, alert_link)},
    {"alerttext", "the pet want's in ($bat$V)", WebPrefs::STRING, sizeof(_device_config::alert_text), offsetof(_device_config, alert_text)},
    {"token", "YOUR_TELEGRAM_TOKEN", WebPrefs::PASSWORD, sizeof(_device_config::token), offsetof(_device_config, token)},
    {"ddelay", "2", WebPrefs::INTEGER, sizeof(_device_config::ddelay), offsetof(_device_config, ddelay)},
    {"dthreshold", "4", WebPrefs::INTEGER, sizeof(_device_config::dthreshold), offsetof(_device_config, dthreshold)},
    {"stimer", "300", WebPrefs::INTEGER, sizeof(_device_config::stimer), offsetof(_device_config, stimer)},
    {"atimer", "30", WebPrefs::INTEGER, sizeof(_device_config::atimer), offsetof(_device_config, atimer)},
    {"led", "on", WebPrefs::CHECKBOX, sizeof(_device_config::led), offsetof(_device_config, led)},
    {"power", "on", WebPrefs::CHECKBOX, sizeof(_device_config::power), offsetof(_device_config, power)}};
