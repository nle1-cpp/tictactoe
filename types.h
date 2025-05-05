#ifndef TYPES_H
#define TYPES_H

typedef struct
{
  char cells[3][3];
  char current_player;
  int current_turn;
  int is_running;
} Game;

typedef enum {
	GAME_ILLEGAL_TURN = -1,
	GAME_CONTINUE = 0,
	GAME_VICTORY_O = 1,
	GAME_VICTORY_X = 2,
  GAME_DRAW = 3
} GameStatus;

#endif
