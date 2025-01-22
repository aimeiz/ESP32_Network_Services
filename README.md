//This is library for convinient usage of main network services used in ESP32 projects:
OTA, NTP, WEBSERVER, WEBSOCKETS, FTP, SPIFFS, TELNETSTREAM.
Services can be used selectively using #define ENABLE_<SERVICE> i.e: #define ENABLE_FTP
Then proper libraries and code are incorporated into your code during compilation.
Also possible to define wifi ssid, ftp user ota password, define ESP32 core assigned to network tasks. See example.
