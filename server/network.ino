#include <WiFi.h>
#include <PubSubClient.h>

#include "globals.h"
#include "credentials.h"

WiFiClient wifi_client;
PubSubClient client;

const char *ssid = SECRET_SSID;
const char *pass = SECRET_PASS;
const char *mqtt_broker = GCP_ADDRESS;
const int mqtt_port = 1883; 

void setup_wifi_client()
{
	// Connect to WiFi
	WiFi.mode(WIFI_STA);
	WiFi.disconnect();
	delay(500);
	WiFi.begin(ssid, pass);
	Serial.println("Connecting to WiFi..");
	while (WiFi.status() != WL_CONNECTED) {
		delay(20);
		Serial.print(".");
	}

	// Print connection info
	Serial.println("\nWiFi connected");
	Serial.print("SSID: ");
	Serial.println(WiFi.SSID());
	Serial.print("IP address: ");
	Serial.println(WiFi.localIP());
}

void setup_mqtt_client()
{
  client.setClient(wifi_client);
	client.setServer(mqtt_broker, mqtt_port);
	client.setCallback(callback);

	String client_id = "esp32-client-" + WiFi.macAddress();

	Serial.println("Connecting to MQTT broker..");
	while(!client.connected()) {
		delay(500);
		Serial.print(".");
		if (client.connect(client_id.c_str()))
		{
			Serial.println("\nMQTT client \""+ String(client_id) +"\"");
			Serial.println("Connected to "+ String(mqtt_broker) +" on port "+ String(mqtt_port));
		} 
		else 
		{
      Serial.print("\nAttempt failed with state ");
      Serial.println(client.state());
      delay(2000);
		}
	}

	if (client.subscribe(move_channel))
		Serial.println("Subscribed to topic \""+ String(move_channel) +"\"");
	else
		Serial.println("Failed to subscribe to topic");
}
