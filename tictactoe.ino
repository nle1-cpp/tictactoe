#include <WiFi.h>
#include <PubSubClient.h>
#include <string.h>

#include "types.h"
#include "globals.h"

Game* game_ptr = (Game *) malloc(sizeof(Game));

// ready up
int client_ready_o = 0;
int client_ready_x = 0;

// game counters
int games_played = 0;
int games_o_won = 0;
int games_x_won = 0;
int games_drawn = 0;

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

      delay(1000);
      publish_board();
      delay(500);

      if (game_ptr->current_player == 'O')
        client.publish(status_channel, "TURN_O");
      else client.publish(status_channel, "TURN_X");
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
  
    delay(1000);
    publish_board();
    delay(250);
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
        delay(250);
        publish_board();

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
        delay(250);
        publish_board();

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
        delay(250);
        publish_board();

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

void publish_board() {
  char board_state[32] = "";
  for (int i = 0; i < 3; i++)
  {
    for (int j = 0; j < 3; j++)
    {
      if (game_ptr->cells[i][j] == 'O')
       strcat(board_state, "O");
      else if (game_ptr->cells[i][j] == 'X')
       strcat(board_state, "X");
      else
       strcat(board_state, "_");

    }
    strcat(board_state, "\n");
  }
  client.publish(display_channel, board_state);
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
