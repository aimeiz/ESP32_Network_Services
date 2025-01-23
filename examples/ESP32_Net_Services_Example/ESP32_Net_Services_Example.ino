//#define ESP32DIAG
#define VERSION "ESP32_Net_Services_Basic_Example_2024122"
//Definitions below enables network services. Put // to which You do not use.
#define ENABLE_OTA
#define ENABLE_FTP
#define ENABLE_TELNETSTREAM
#define ENABLE_WEBSERVER //if websockets are enable, webserver is also enabled
#define ENABLE_WEBSOCKETS
#define WEBSOCKETS_PORT 83 //If not defined, default 81 is set
#define LANGUAGE POL //If not defined default language is ANG
#define ENABLE_NTP //If FTP is enabled also NTP is enabled so no need to enable
//End of enabled services
#if defined(ENABLE_FTP) || defined(ENABLE_NTP)
//#define NTP_SERVER "ntp1.tp.pl" //If not defined default is pool.ntp.org
#define GMTOFFSET_SEC 3600			//Timeshift from UTC to Your timezone.
#define DAYLIGHTOFFSET_SEC 3600		//Additional timeshift for summer time
#endif
//Optionally define ESP32 core for services. default is core 0 - different from arduino program.
//#define FASTNET_SERVICE_CORE 1 //Default is core 0
//#define NETWORK_SERVICES_CORE 1  //Default is core 0
#include <credentials.h>
/* #define MYSSID "Wifi_ssid"
#ifndef MYPASSWORD
#define MYPASSWORD "wifi_password"
#define OTAPASSWORD "Your_Ota_Password"
#define FTPUSER "Your_Ftp_user"
#define FTPPASSWORD "Your_ftp_password"
 #define OTA_HOSTNAME VERSION // Set Your ota hostname or leave one related to program name and version
 */
#include <ESP32_Network_Services.h>

#if defined(ENABLE_WEBSOCKETS) && (defined(ENABLE_FTP) || defined(ENABLE_NTP))
#include "WebPage.html"
#endif

void setup() {
	Serial.begin(115200);
	startNetworkServices();
#ifdef ENABLE_WEBSERVER
#ifndef ENABLE_NTP 
	server.on("/", []() {
		server.send(200, "text/plain", "Hello, world!");
		});
#else
	// Serve the webpage
	server.on("/getWebSocketPort", []() {
		server.send(200, "text/plain", String(WEBSOCKETS_PORT)); // Send the port as plain text
		});

	server.on("/", []() {
		server.send(200, "text/html", PAGE_HTML); // Serve the HTML directly from PROGMEM
		});
#endif
#endif
#ifdef ESP32DIAG
	// Enable runtime stats collection
	vTaskStartScheduler();
#endif
}

void loop() {
#if defined(ENABLE_WEBSOCKETS) && defined(ENABLE_NTP)
	webSocket.broadcastTXT(getTimeString()); // Send time to all WebSocket clients
#endif

#if defined ENABLE_TELNETSTREAM
	if (TelnetStream2.available()) {
		//String input = TelnetStream2.readStringUntil('\n');
#if defined(ENABLE_FTP) || defined(ENABLE_NTP)
		TelnetStream2.println(getTimeString());
		//TelnetStream2.print(getTimeString());
		//TelnetStream2.print(F(" Echo: "));
		//TelnetStream2.println(input);
#else
		TelnetStream2.println("Echo: " + input);
		TelnetStream2.flush();
#endif
	}
#endif
#ifdef ESP32DIAG
	if (!(millis() % 100)){
		printHeapStats();
		//printRunTimeStats();
	}
#endif
}