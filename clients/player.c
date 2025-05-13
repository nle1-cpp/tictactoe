#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <unistd.h>

#include <mosquitto.h>

#include "player.h"

struct mosquitto *mosq;

char player_assignment;
char *hostname = "34.169.220.30";

char *display_channel = "ttt/board/display"; 
char *mode_channel = "ttt/";
char *auth_channel = "ttt/auth/";
char *bot_start_channel;

char cmd[128];
char turn_data[4] = "";

volatile int is_msg_received = 0;
char *received_msg;

void on_connect(struct mosquitto *mosq, void *obj, int rc)
{
	if (rc != 0)
	{
		printf("Error of code %d\n", rc);
		exit(-1);
	}
}

void on_message(struct mosquitto *mosq, void *obj, const struct mosquitto_message *msg)
{
	// handle messages
	received_msg = strdup((char *)msg->payload);
	is_msg_received = 1;
}

char *get_next_message(struct mosquitto *mosq, const char *topic) {
	is_msg_received = 0;
	received_msg = NULL;

	// Subscribe to new topic
	if (mosquitto_subscribe(mosq, NULL, topic, 0) != MOSQ_ERR_SUCCESS) {
		fprintf(stderr, "Failed to subscribe to topic: %s\n", topic);
		return NULL;
	}

	// Wait for one message
	while (!is_msg_received) {
		mosquitto_loop(mosq, 1000, 1);
	}

	printf("From topic: %s\n", topic);
	printf("Received payload: %s\n", received_msg);

	mosquitto_unsubscribe(mosq, NULL, topic);

	return received_msg;
}

int main()
{	
	printf("\033[2J\033[H");	// clears terminal screen

	setvbuf(stdout, NULL, _IONBF, 0);
	srand(time(NULL));

	// init mqtt
	mosquitto_lib_init();
	mosquitto_loop_start(mosq);
	mosq = mosquitto_new(NULL, true, NULL);

	mosquitto_connect_callback_set(mosq, on_connect);
	mosquitto_message_callback_set(mosq, on_message);

	int rc;
	do
	{
		rc = mosquitto_connect(mosq, hostname, 1883, 10);
		if(rc) {
			printf("Could not connect to Broker with return code %d\n", rc);
			sleep(2);
		}
	} while (rc != 0);
	// end init


	// Print menu:
	printf("1. Player vs. Player\n");
	printf("2. Player vs. Computer\n");
	printf("3. Computer vs. Computer\n");
	printf("Select gamemode (1-3):\n");

	char mode_selection;
	int valid_mode = 0;
	do
	{
		mode_selection = getchar();
		getchar(); // consume newline

		char *message;
		switch (mode_selection)
		{
			case '1': 
			{
				printf("\033[2J\033[H");	// clears terminal screen
					
				// sends a unique string into assignment channel
				// only used for player vs player
				message = get_client_id(32);
				snprintf(cmd, 128, "mosquitto_pub -h %s -t %s -m %s", hostname, "ttt/assignment", message);
				system(cmd);

				// recieves player assignment through private channel named after unique string
				snprintf(cmd, 128, "mosquitto_sub -h %s -t %s", hostname, message);
				FILE *cmd_exec = popen(cmd, "r");

				char cmd_output[128];
				if (fgets(cmd_output, sizeof(cmd_output), cmd_exec) != NULL)
					if (cmd_output[0] == 'O')
						player_assignment = 'O';
					else
						player_assignment = 'X';
				pclose(cmd_exec);

				pvp_play();

				valid_mode = 1;
				break;
			}
			case '2':
			{
				// always o
				valid_mode = 1;

				pvai_play();

				break;
			}
			case '3':
			{
				// printf("\033[2J\033[H");	// clears terminal screen

				autoplay();

				valid_mode = 1;
				break;
			}
			default:
			{
				printf("Invalid selection. Try again.");
				break;
			}
		}	
	} while (!valid_mode);


	// terminate mqtt 
	mosquitto_disconnect(mosq);
	mosquitto_destroy(mosq);
	mosquitto_lib_cleanup();


	return 0;
}

void pvp_play()
{
	system("clear");
	// Get player assignment from server

	// Prompt: Press any key to READY 
	printf("Press ENTER to READY.");	
	getchar();
	system("clear");

	snprintf(cmd, 128, "mosquitto_pub -h %s -t %s -C 1", 
			hostname, auth_channel, tolower(player_assignment));
	system(cmd);

	printf("Waiting for another player...");
	snprintf(cmd, 128, "mosquitto_sub -h %s -t %s%c -C 1", hostname, "");
	FILE *cmd_exec = popen(cmd, "r");

	printf("You are player %c.\n", player_assignment);
	printf("Enter X coordinate [0-2]: ");
	char x_coord = getchar();
	printf("Enter Y coordinate [0-2]: ");
	char y_coord = getchar();

	snprintf(cmd, 128, "mosquitto_pub -h %s -t %s%c -C 1",
			hostname, auth_channel, tolower(player_assignment));
}

void pvai_play()
{
	player_assignment = 'O';
	
	mosquitto_publish(mosq, NULL, "ttt/moves", 8, "READY_O", 0, false);
	mosquitto_publish(mosq, NULL, "ttt/bots", 12, "START_BOT_X", 0, false);

	int is_game_over = 0;
	while (is_game_over == 0)
	{
		int is_turn = 0;
		char* game_status;
		do
		{
			print_board();

			game_status = get_next_message(mosq, "ttt/board/status");		

			if (strcmp(game_status, "TURN_O") == 0)
			{
				printf("It is your turn.\n\n");	
				is_turn = 1;	
			}
			else if (strcmp(game_status, "TURN_X") == 0)
				printf("It is your opponent's turn.\n\n");
			else
			{
				print_board();

				if (strcmp(game_status, "VICTORY_O") == 0)
					printf("You have won the game.");
				else if (strcmp(game_status, "VICTORY_X") == 0)
					printf("Your opponent has won the game.\n");
				else if (strcmp(game_status, "DRAW") == 0)
					printf("Game ended in a draw.\n");
				else
					printf("Received invalid game status: %s", game_status);

				is_game_over = 1;
			}

			free(game_status);		
		} while (is_turn == 0);

		if (is_game_over != 0)
			break;
	
		int is_legal = 0;
		char* auth;
		do
		{
			printf("Enter X coordinate [0-2]: ");
			char x_coord = getchar();
			getchar();

			printf("Enter Y coordinate [0-2]: ");
			char y_coord = getchar();
			getchar();

			turn_data[0] = player_assignment;		
			turn_data[1] = x_coord;		
			turn_data[2] = y_coord;		
			turn_data[3] = '\0';		

			printf("Validating move...\n");

			mosquitto_publish(mosq, NULL, "ttt/moves", 5, turn_data, 0, false);

			auth = get_next_message(mosq, "ttt/auth/o");
			if (strcmp(auth, "OK") == 0)
			{
				printf("Your move is legal.\n\n");
				is_legal = 1;
			}
			else
				printf("Illegal move, try again. \n\n");		

			free(auth);
		} while (is_legal != 1);
	}
}

void autoplay()
{
	mosquitto_publish(mosq, NULL, "ttt/bots", 15, "START_AUTOPLAY", 0, false);

	int is_game_over = 0;
	do
	{
		print_board();

		char* game_status = get_next_message(mosq, "ttt/board/status");		
		if (strcmp(game_status, "TURN_O") == 0)
			printf("It is Player O's turn.\n\n");	
      		else if (strcmp(game_status, "TURN_X") == 0)
			printf("It is Player X's turn.\n\n");
		else
		{
			print_board();

			if (strcmp(game_status, "VICTORY_O") == 0)
				printf("Player O has won the game.");
			else if (strcmp(game_status, "VICTORY_X") == 0)
				printf("Player X has won the game.\n");
			else if (strcmp(game_status, "DRAW") == 0)
				printf("Game ended in a draw.\n");
			else
				printf("Received invalid game status: %s", game_status);

			is_game_over = 1;
		}

	} while (!is_game_over);
}

void print_board()
{
	// printf("\033[2J\033[H");	// clears terminal screen
	
	char* board_display = get_next_message(mosq, "ttt/board/display");

	printf("Current Board:\n");

	printf("\n");
	printf("%s", board_display);
	printf("\n");
	// snprintf(cmd, 128, "mosquitto_sub -h %s -t %s -C 1", hostname, display_channel);
	// FILE *cmd_exec = popen(cmd, "r");
	//
	// char cmd_output[128];
	// if (fgets(cmd_output, sizeof(cmd_output), cmd_exec) != NULL)
	// 	printf("%s", cmd_output);
	//
	// pclose(cmd_exec);
	//
	free(board_display);
}

char *get_client_id(int length) 
{
    // Seed the random number generator with the current time
    srand(time(NULL));

    // Define the character set to choose from
    const char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    int charset_len = strlen(charset);

    // Allocate memory for the string, including null terminator
    char *client_id = (char*) malloc(sizeof(char)  *(length + 1));
    if (client_id == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        return NULL; // Exit if memory allocation fails
    }
    
    // Generate random characters and append to the string
    for (int n = 0; n < length; n++) {
        int key = rand() % charset_len; // Pick a random index from the character set
        client_id[n] = charset[key]; // Append the character at the index to the string
    }

    client_id[length] = '\0'; // Null-terminate the string

    return client_id;
}
