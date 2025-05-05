#include <WiFi.h>
#include <PubSubClient.h>
#include <stdlib.h>
#include <time.h>

#include "types.h"

WiFiClient wifi_client;
PubSubClient client(wifi_client);

const char* player_channel = "player-actions";  // imports player actions
const char* logic_channel = "game-status";      // export game status
const char* display_channel = "board-state";

int games_played = 0;
int games_o_won = 0;
int games_x_won = 0;
int games_drawn = 0;

int is_playing = 0;

Game* game_ptr = (Game *) malloc(sizeof(Game));

// executes when a player acts
void callback(char* topic, byte* payload, unsigned int length)
{
	char parsed[64] = "";

	for (int i = 0; i < length; i++)
	{ // translate payload to a string
		parsed[i] = (char) payload[i];
	}

  // Expects payload of format "player,x,y"
  char* turn_data = parsed;
  GameStatus current_state = add_to_board((int) turn_data[0], (int) turn_data[1], (int) turn_data[2], game_ptr);

  switch (current_state)
  {
    case GAME_ILLEGAL_TURN:
      client.publish(logic_channel, "GAME_ILLEGAL_TURN");
      break;
    case GAME_CONTINUE:
      client.publish(logic_channel, "GAME_CONTINUE");
      print_board(game_ptr);
      break;
    case GAME_VICTORY_O:
      client.publish(logic_channel, "GAME_VICTORY_O");
      new_game(game_ptr);
      break;
    case GAME_VICTORY_X:
      client.publish(logic_channel, "GAME_VICTORY_X");
      new_game(game_ptr);
      break;
  }
}

void setup()
{
	Serial.begin(115200);
	delay(2000);
	Serial.println("\nDevice is on.");

  // setup_wifi_client();
	// setup_mqtt_client(client, player_channel, callback);
  setup_display();

  randomSeed(analogRead(0));

  new_game(game_ptr);
}

void loop()
{
  // if(is_playing)
  // {
  //   // keeps connection alive
  //   client.loop();
  //
  //   if(WiFi.status() != WL_CONNECTED)
  //     Serial.println("WiFi is not connected.");
  //   //else
  //     //Serial.println("WiFi is connected."); 
  // }
  // else
  // {
  //   Serial.println("MQTT client disconnected.");
  //   client.disconnect();
  // }
  if (game_ptr->is_running)
  {
    int status;
    int x;
    int y;
    do
    {
      x = random(0,3);
      y = random(0,3);
      status = add_to_board(1, x, y, game_ptr);
    }
    while (status == GAME_ILLEGAL_TURN);
    
    if (status == GAME_VICTORY_O)
      games_o_won++;
    else if (status == GAME_DRAW)
      games_drawn++;

    print_board(game_ptr);
  }

  if (game_ptr->is_running == 1)
  {
    int status;
    int x;
    int y;
    do
    {
      x = random(0,3);
      y = random(0,3);
      status = add_to_board(2, x, y, game_ptr);
    }
    while (status == GAME_ILLEGAL_TURN);

    if (status == GAME_VICTORY_X)
      games_x_won++;
    else if (status == GAME_DRAW)
      games_drawn++;

    print_board(game_ptr);
  }

  if(!game_ptr->is_running && games_played != 99)
  {
    new_game(game_ptr);
    games_played++;
  }

  lcd_print_ln1((" O - "+ String(games_o_won) +"; X - "+ String(games_x_won)).c_str());
  lcd_print_ln2((" Draw - "+ String(games_drawn)).c_str());

  delay(20);
}
