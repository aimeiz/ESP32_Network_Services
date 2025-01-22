// ESP32 Network Services Library
//Library is modtly based on standard network librarys included in ESP32 Arduino SDK
//Used also:
// FTPClientServer-master version 0.9.9 by dplasa@gmail.c needed if #define ENABLE_FTP is used. https://github.com/dplasa/FTPClientServer
// Telnetstream2 https://github.com/ameer1234567890/TelnetStream if #define ENABLE_TELNETSTREAM is used
#ifndef ESP32_NETWORK_SERVICES_H
#define ESP32_NETWORK_SERVICES_H

#include <Arduino.h>
#include <WiFi.h>

#ifdef ENABLE_OTA
#include <ArduinoOTA.h>
#endif

#ifdef ENABLE_WEBSOCKETS
#define ENABLE_WEBSERVER
#include <WebSocketsServer.h>
#ifndef WEBSOCKETS_PORT
#define WEBSOCKETS_PORT 81
#endif
WebSocketsServer webSocket = WebSocketsServer(WEBSOCKETS_PORT);

String lastWebSocketMessage = ""; // Global variable to hold the last message
bool newWebSocketMessage = false; // Flag to indicate a new message

void webSocketEvent(uint8_t num, WStype_t type, uint8_t* payload, size_t length) {
	if (type == WStype_CONNECTED) {
		Serial.printf("WebSocket client #%u connected\n", num);
	}
	else if (type == WStype_DISCONNECTED) {
		Serial.printf("WebSocket client #%u disconnected\n", num);
	}
	else if (type == WStype_TEXT) {
		Serial.printf("Client #%u sent: %s\n", num, payload);
       lastWebSocketMessage = String((char*)payload).substring(0, length);
       newWebSocketMessage = true; // Mark new message as received
	 Serial.printf(F("newWebSocketMessage = %d, lastWebSocketMessage = %s \r\n"), newWebSocketMessage, lastWebSocketMessage);
	}
}
#endif

#ifdef ENABLE_FTP
#define ENABLE_SPIFFS
#define ENABLE_NTP //Ntp is enabled if ftp enabled to assure proper date / time for stored files
#include <SPIFFS.h>
#include<FS.h>
#include <FTPServer.h> //FTPClientServer-master version 0.9.9 by dplasa@gmail.c
FTPServer ftp(SPIFFS);
#endif
#if defined ENABLE_FTP || defined ENABLE_WEBSOCKETS
// FreeRTOS core selection
#ifndef FASTNET_SERVICE_CORE
#define FASTNET_SERVICE_CORE 0
#endif
#if defined ENABLE_SPIFFS && !defined ENABLE_FTP
#include <SPIFFS.h>
#include<FS.h>
#endif
TaskHandle_t fastNetTaskHandle = NULL;
void fastNetTask(void* parameter) {
	for (;;) {
		//esp_task_wdt_reset();
#ifdef ENABLE_FTP
		ftp.handleFTP();
#endif
#ifdef ENABLE_WEBSOCKETS
		webSocket.loop();
#endif
		vTaskDelay(pdMS_TO_TICKS(10)); // Adjust delay as needed
	}
}
#endif

#ifdef ENABLE_NTP
#include <time.h>
#ifndef	 NTP_SERVER 
#define NTP_SERVER "pool.ntp.org"
// get week days
#define ANG 0
#define POL 1
#ifndef LANGUAGE
#define LANGUAGE ANG
#endif
const char* getTimeString() {
	static unsigned long lastUpdate = 0;
	static unsigned long hundredthsCounter = 0; // Tracks 1/100s of a second
	static int lastSecond = -1;                 // Tracks the last second to reset counter
	static char timeStr[64];                   // Persistent buffer for the formatted string
#if (LANGUAGE == POL)
//	const char* weekDays[] PROGMEM = { "Niedziela", "Poniedzia³ek", "Wtorek", "Œroda", "Czwartek", "Pi¹tek", "Sobota" };
	const char* weekDays[] = { "Niedziela", "Poniedzia³ek", "Wtorek", "Œroda", "Czwartek", "Pi¹tek", "Sobota" };
#elif (LANGUAGE == ANG)
	const char* weekDays[] PROGMEM = { "Sunday", "Monday","Tuesday","Wednesday", "Thursday", "Friday", "Saturday" };
#endif
	unsigned long currentMillis = millis();

	// Update every 10 milliseconds
	if (currentMillis - lastUpdate >= 10) {
		lastUpdate = currentMillis;

		struct tm timeinfo;
		if (getLocalTime(&timeinfo)) {
			// Reset 1/100s counter if the second has changed
			if (lastSecond != timeinfo.tm_sec) {
				lastSecond = timeinfo.tm_sec;
				hundredthsCounter = 0;
			}
			else {
				hundredthsCounter = (currentMillis % 1000) / 10; // Update 1/100s counter
			}

			// Format the time string
			snprintf(timeStr, sizeof(timeStr), "%s %04d-%02d-%02d %02d:%02d:%02d.%02d",
				weekDays[timeinfo.tm_wday],   // Week days names
				timeinfo.tm_year + 1900,        // Year (adjusted from tm_year)
				timeinfo.tm_mon + 1,            // Month (tm_mon is 0-11)
				timeinfo.tm_mday,               // Day of the month
				timeinfo.tm_hour,               // Hours
				timeinfo.tm_min,                // Minutes
				timeinfo.tm_sec,                // Seconds
				hundredthsCounter);             // 1/100s of a second
		}
	}

	return timeStr;
}
#endif
#ifndef GMTOFFSET_SEC 
#define GMTOFFSET_SEC 0 				//Default UTC time
#endif
#ifndef DAYLIGHTOFFSET_SEC 
#define DAYLIGHTOFFSET_SEC 3600 	//Default summer time offset 1 hour
#endif
struct tm timeinfo;
bool timeOK;
void ntp_handle() {
	if ((WiFi.status() == WL_CONNECTED) && ((timeinfo.tm_min % 10) == 0) && (timeinfo.tm_sec == 0)) //Get time from the network each 10 min
	{
		configTime(GMTOFFSET_SEC, DAYLIGHTOFFSET_SEC, NTP_SERVER);
		if (!getLocalTime(&timeinfo)) {
			Serial.println(F("Failed to obtain time"));
		}
		bool timeOK = getLocalTime(&timeinfo);
	}
}
#endif

#ifdef ENABLE_TELNETSTREAM
#include <TelnetStream2.h>  //https://github.com/ameer1234567890/TelnetStream

#endif

#ifdef ENABLE_WEBSERVER
#include <WebServer.h>
#ifndef WEBSERVER_PORT
#define  WEBSERVER_PORT 80
#endif
WebServer server(WEBSERVER_PORT);
#endif

// FreeRTOS core selection
#ifndef NETWORK_SERVICES_CORE
#define NETWORK_SERVICES_CORE 0
#endif

// Task handles
TaskHandle_t networkTaskHandle = NULL;

//#endif

// External credentials (set in the .ino file)
#ifdef MYSSID
const char* wifiSSID = MYSSID;
#else
extern const char* wifiSSID;
#endif
#ifdef MYPASSWORD
const char* wifiPassword = MYPASSWORD;
#else
extern const char* wifiPasswor;
#endif
#ifdef FTPUSER
const char* ftpUser = FTPUSER;
#else
extern const char* ftpUser;
#endif
#ifdef FTPPASSWORD
const char* ftpPassword = FTPPASSWORD;
#else
extern const char* ftpPassword;
#endif
//extern const char* otaPassword;

void connectToWiFi() {
	if (wifiSSID == nullptr || wifiPassword == nullptr) {
		Serial.println(F("WiFi credentials are not set!"));
		return;
	}
	Serial.print(F("Trying to connect "));
	Serial.println(wifiSSID);
	WiFi.begin(wifiSSID, wifiPassword);
	for (int i = 0; (i < 20) && (WiFi.status() != WL_CONNECTED); i++) { //Try 20 times to connect
		delay(500);
		Serial.print(".");
	}
	if (WiFi.status() != WL_CONNECTED) {
		Serial.println(F("\r\nFailed to connect "));
		return;
	}
	else {
		Serial.print(F("\r\nWiFi connected to: "));
		Serial.print(WiFi.SSID());
		Serial.print(F(" IP Address "));
		Serial.println(WiFi.localIP());
	}
}

//#ifdef ENABLE_TELNETSTREAM
//void handleTelnetStream() {
//	static bool initialized = false;
//	if (!initialized) {
//		TelnetStream2.begin();
//		initialized = true;
//	}
//}
//#endif

#ifdef ENABLE_WEBSERVER
void handleWebServer() {
	server.handleClient();
}
#endif
void networkTask(void* parameter) {
	for (;;) {
#ifdef ENABLE_OTA
		ArduinoOTA.handle();
#endif
#if defined ENABLE_NTP
		ntp_handle();
#endif

//#ifdef ENABLE_TELNETSTREAM
//		handleTelnetStream();
//#endif

#ifdef ENABLE_WEBSERVER
		//handleWebServer();
		server.handleClient();
#endif
		//#ifdef ENABLE_WEBSOCKETS
		//		webSocket.loop();
		//#endif
		vTaskDelay(pdMS_TO_TICKS(100)); // Adjust delay as needed
	}
}

#ifdef ESP32DIAG
#include <esp_heap_caps.h>

void printHeapStats() {
	// Internal DRAM
	size_t totalInternalHeap = heap_caps_get_free_size(MALLOC_CAP_INTERNAL);
	size_t largestInternalBlock = heap_caps_get_largest_free_block(MALLOC_CAP_INTERNAL);
	float internalFragmentation = 0.0;
	if (totalInternalHeap > 0) {
		internalFragmentation = 100.0 * (1.0 - (float)largestInternalBlock / (float)totalInternalHeap);
	}

	// External PSRAM
	size_t totalPsramHeap = heap_caps_get_free_size(MALLOC_CAP_SPIRAM);
	size_t largestPsramBlock = heap_caps_get_largest_free_block(MALLOC_CAP_SPIRAM);
	float psramFragmentation = 0.0;
	if (totalPsramHeap > 0) {
		psramFragmentation = 100.0 * (1.0 - (float)largestPsramBlock / (float)totalPsramHeap);
	}

	Serial.println(F("Heap Statistics:"));

	// Internal DRAM
	Serial.println(F("Internal DRAM Heap:"));
	Serial.printf(F("  Total Free Heap: %u bytes\n"), totalInternalHeap);
	Serial.printf(F("  Largest Free Block: %u bytes\n"), largestInternalBlock);
	Serial.printf(F("  Fragmentation: %.2f%%\n"), internalFragmentation);

	// External PSRAM
	if (totalPsramHeap > 0) {
		Serial.println(F("External PSRAM Heap:"));
		Serial.printf(F("  Total Free Heap: %u bytes\n"), totalPsramHeap);
		Serial.printf(F("  Largest Free Block: %u bytes\n"), largestPsramBlock);
		Serial.printf(F("  Fragmentation: %.2f%%\n"), psramFragmentation);
	}
	else {
		Serial.println(F("No external PSRAM available."));
	}

	Serial.println(); // Blank line for readability
}

//#include <freertos/FreeRTOS.h>
//#include <freertos/task.h>
//
//void printRunTimeStats() {
//	// Allocate a buffer to hold runtime stats information
//	char runTimeStatsBuffer[512];
//
//	// Header for the output
//	Serial.println(F("Task Name       Time    %"));
//
//	// Get runtime stats information
//	vTaskGetRunTimeStats(runTimeStatsBuffer);
//
//	// Print the runtime stats information
//	Serial.println(runTimeStatsBuffer);
//}

#endif

void startNetworkServices() {
	connectToWiFi();
#ifdef ENABLE_NTP
	configTime(GMTOFFSET_SEC, DAYLIGHTOFFSET_SEC, NTP_SERVER);
	if (!getLocalTime(&timeinfo)) {
		Serial.println(F("Failed to obtain time"));
	}
	else
	{
		bool timeOK = getLocalTime(&timeinfo);
		Serial.print(F("Date / Time set to: "));
		Serial.println(&timeinfo, " %A, %B %d %Y %H:%M:%S");
	}
#endif
#ifdef ENABLE_OTA
#ifndef OTA_HOSTNAME
#define OTA_HOSTNAME "ESP32_OTA"
#endif
	ArduinoOTA.setHostname(OTA_HOSTNAME);
#ifdef OTA_PASSWORD
	const char* otaPassword PROGMEM = OTA_PASSWORD;
	ArduinoOTA.setPassword(otaPassword);//If defined ota password initiate ota with password
#endif
	ArduinoOTA.begin();
	Serial.print(F("OTA service started. Hostname: "));
	Serial.println(OTA_HOSTNAME);
#endif
#ifdef ENABLE_SPIFFS
if (!SPIFFS.begin(true)) {
	Serial.println(F("SPIFFS Mount Failed"));
}
else {
	Serial.println(F("SPIFFS Mounted"));
}
#endif
#if defined ENABLE_FTP
	//if (!SPIFFS.begin(true)) {
	//	Serial.println(F("SPIFFS Mount Failed"));
	//}
	ftp.begin(ftpUser, ftpPassword);
	Serial.println(F("FTP server started on port 21"));
#endif
#if defined ENABLE_FTP || defined ENABLE_WEBSOCKETS
	xTaskCreatePinnedToCore(
		fastNetTask,
		"FastNet Task",
		//4096, // Adjust stack size if needed
		8192, // Adjust stack size if needed
		//16384, // Adjust stack size if needed
		NULL,
		1,
		&fastNetTaskHandle,
		FASTNET_SERVICE_CORE
	);
#endif

#ifdef ENABLE_WEBSERVER
	server.begin();
	Serial.print(F("Web server started. http://"));
	Serial.print(WiFi.localIP());
	Serial.print(F(":"));
	Serial.println(WEBSERVER_PORT);
	//server.on("/favicon.ico", HTTP_GET, []() {
	//	server.send(200, "text/plain", ""); // Send an empty string
	//	});
#endif
#ifdef ENABLE_WEBSOCKETS
	// Start WebSocket Server
	webSocket.begin();
	webSocket.onEvent(webSocketEvent);
	Serial.print(F("WebSocket serwer starter on port "));
	Serial.println(WEBSOCKETS_PORT);
#endif
#ifdef ENABLE_TELNETSTREAM
	TelnetStream2.begin();
#endif

	xTaskCreatePinnedToCore(
		networkTask,
		"Network Task",
		8192, // Adjust stack size if needed
		//16384, // Adjust stack size if needed
		NULL,
		1,
		&networkTaskHandle,
		NETWORK_SERVICES_CORE
	);
}

#endif // ESP32_NETWORK_SERVICES_H
