#include <WiFi.h>
#include <PubSubClient.h>
#include "types.h"

#ifndef GLOBALS_H
#define GLOBALS_H

extern WiFiClient wifi_client;
extern PubSubClient client;

// data pipeline
extern const char* move_channel = "ttt/moves";  // import player actions
extern const char* auth_channel_o = "ttt/auth/o"; // export if o's move was valid or not
extern const char* auth_channel_x = "ttt/auth/x"; // export if x's move was valid or not
extern const char* status_channel = "ttt/board/status"; // export game status
extern const char* display_channel = "ttt/board/display";  // export board state

extern Game* game_ptr;

#endif
