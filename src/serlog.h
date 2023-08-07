#if defined(LOG_SERIAL_LEVEL) && (LOG_SERIAL_LEVEL&1)
#define LOG_SERROR(format, ...) if (Serial) { Serial.print("ERROR: "); if (strstr(format, "%") != NULL) { Serial.printf(format, ##__VA_ARGS__); } else { Serial.println(format); }}
#else
#define LOG_SERROR(format, ...) ((void) 0)
#endif

#if defined(LOG_SERIAL_LEVEL) && (LOG_SERIAL_LEVEL&2)
#define LOG_SINFO(format, ...) if (Serial) { Serial.print(" INFO: "); if (strstr(format, "%") != NULL) { Serial.printf(format, ##__VA_ARGS__); } else { Serial.println(format); }}
#else
#define LOG_SINFO(format, ...) ((void) 0)
#endif

#if defined(LOG_SERIAL_LEVEL) && (LOG_SERIAL_LEVEL&4)
#define LOG_SDEBUG(format, ...) if (Serial) { Serial.print("DEBUG: "); if (strstr(format, "%") != NULL) { Serial.printf(format, ##__VA_ARGS__); } else { Serial.println(format); }}
#else
#define LOG_SDEBUG(format, ...) ((void) 0)
#endif