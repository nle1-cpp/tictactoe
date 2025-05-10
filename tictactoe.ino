#include <WiFi.h>
#include <PubSubClient.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#include "credentials.h"
#include "types.h"

WiFiClient wifi_client;
PubSubClient client(wifi_client);

// networking
const char *ssid = SECRET_SSID;
const char *pass = SECRET_PASS;
const char *mqtt_broker = GCP_ADDRESS;
const int mqtt_port = 1883; 

// data pipeline
const char* move_channel = "ttt/moves";  // import player actions
const char* auth_channel_o = "ttt/auth/o"; // export if o's move was valid or not
const char* auth_channel_x = "ttt/auth/x"; // export if x's move was valid or not
const char* status_channel = "ttt/board/status"; // export game status
const char* display_channel = "ttt/board/display";  // export board state

// ready up
int client_ready_o = 0;
int client_ready_x = 0;

// game counters
int games_played = 0;
int games_o_won = 0;
int games_x_won = 0;
int games_drawn = 0;

Game* game_ptr = (Game *) malloc(sizeof(Game));

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

// executes when a player acts
void callback(char* topic, byte* payload, unsigned int length)
{
	char parsed[128] = "";

	for (int i = 0; i < length; i++)
	{ // translate payload to a string
		parsed[i] = (char) payload[i];
	}

  Serial.println(String(parsed));

  // Expects payload of format "$player$x$y"
  char* turn_data = parsed;
  if (!game_ptr->is_running)
  {
    Serial.println("Games completed: "+String(games_played));
    if (strcmp(parsed, "READY_O") == 0)
      client_ready_o++;
    else
      client_ready_x++;
    
    if (client_ready_o != 0 && client_ready_x != 0)
    {
      char first_player = get_first_player();
      new_game(game_ptr, first_player);
      if (game_ptr->current_player == 'O')
        client.publish(status_channel, "TURN_O");
      else
        client.publish(status_channel, "TURN_X");
      client_ready_o = 0;
      client_ready_x = 0;
    }
  }
  else if (game_ptr->current_player == turn_data[0])
  {
    int x = ((int) turn_data[1]) - 48; // HACK: convert character number to actual integer
    int y = ((int) turn_data[2]) - 48;
    GameStatus current_status = add_to_board(turn_data[0], x, y, game_ptr);
  
    delay(2);
    switch (current_status)
    {
      case GAME_ILLEGAL_TURN:
        if (game_ptr->current_player == 'O') // current player is still current_player as invalid move was made
          client.publish(auth_channel_o, "ILLEGAL_MOVE");
        else
          client.publish(auth_channel_x, "ILLEGAL_MOVE");
        break;

      case GAME_CONTINUE:
        delay(6);
        if (game_ptr->current_player == 'O') // current player is actually next as alt_player is called in add_to_board
        {
          client.publish(auth_channel_x, "OK");
          client.publish(status_channel, "TURN_O");
        }
        else
        {
          client.publish(auth_channel_o, "OK");
          client.publish(status_channel, "TURN_X");
        }
        break;

      case GAME_VICTORY_O:
        delay(6);
        if (game_ptr->current_player == 'O')
          client.publish(auth_channel_x, "OK");
        else
          client.publish(auth_channel_o, "OK");
        client.publish(status_channel, "VICTORY_O");
        games_o_won++;
        games_played++;
        delay(2);
        break;

      case GAME_VICTORY_X:
        delay(6);
        if (game_ptr->current_player == 'O')
          client.publish(auth_channel_x, "OK");
        else
          client.publish(auth_channel_o, "OK");    
        client.publish(status_channel, "VICTORY_X");
        games_x_won++;
        games_played++;
        delay(2);
        break;

      case GAME_DRAW:
        delay(6);
        if (game_ptr->current_player == 'O')
          client.publish(auth_channel_x, "OK");
        else
          client.publish(auth_channel_o, "OK");
        delay(6);
        client.publish(status_channel, "DRAW");
        games_drawn++;
        games_played++;
        delay(2);
        break;
    }
  }
}

char get_first_player()
{
  long rand = random(0,2);
  if (rand < 1)
    return 'O';
  else
    return 'X';
}

void setup()
{
	Serial.begin(115200);
	delay(2000);
	Serial.println("\nDevice is on.");

  setup_wifi_client();
	setup_mqtt_client();
  setup_display();
  setup_game(game_ptr);

  randomSeed(analogRead(0));
  
  Serial.println("End setup.");
}

void loop()
{
  // keeps connection alive
  client.loop();

  if(WiFi.status() != WL_CONNECTED)
    Serial.println("WiFi is not connected.");

  // Update display every game end
  lcd_print_ln1((" O - "+ String(games_o_won) +"; X - "+ String(games_x_won)).c_str());
  lcd_print_ln2((" Draw - "+ String(games_drawn)).c_str());

  delay(500);
}
