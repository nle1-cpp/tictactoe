#include <WiFi.h>

#include "credentials.h"

const char *ssid = SECRET_SSID;
const char *pass = SECRET_PASS;

void setup_wifi_client()
{
	// Connect to WiFi
	WiFi.mode(WIFI_STA);
	WiFi.disconnect();
	delay(100);
	WiFi.begin(ssid, pass);
	Serial.println("Connecting to WiFi..");
	while (WiFi.status() != WL_CONNECTED) {
		delay(500);
		Serial.print(".");
	}

	// Print connection info
	Serial.println("\nWiFi connected");
	Serial.print("SSID: ");
	Serial.println(WiFi.SSID());
	Serial.print("IP address: ");
	Serial.println(WiFi.localIP());
}
