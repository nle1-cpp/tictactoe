#include <PubSubClient.h>

#include "credentials.h"

const char *mqtt_broker = GCP_ADDRESS;
const int mqtt_port = 1883; 

void setup_mqtt_client(PubSubClient client, const char* topic, void (*callback)(char* topic, byte* payload, unsigned int length))
{
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

	if (client.subscribe(topic))
		Serial.println("Subscribed to topic \""+ String(topic) +"\"");
	else
		Serial.println("Failed to subscribe to topic");
}
