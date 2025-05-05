// tictactoe logic
// following every action...
// check 3 rows, 3 columns, and 2 diagonals
// terminate if all 3 cells are of a kind
#include <stdio.h>
#include "types.h"

void new_game(Game* game_ptr)
{
  memset(game_ptr->cells, 0, sizeof(game_ptr->cells));
  game_ptr->current_player = 'O';
  game_ptr->current_turn = 1; // or 0 idk
  game_ptr->is_running = 1;
}

void print_board(Game* game_ptr)
{
  for (int i = 0; i < 3; i++)
  {
    for (int j = 0; j < 3; j++)
    {
      Serial.print(String(game_ptr->cells[i][j]));
      if (game_ptr->cells[i][j] == 0)
      {
        Serial.print(" ");
      }
    }
    Serial.println();
  }
}

// 1 is O, 2 is X
GameStatus add_to_board(int player, int x, int y, Game* game_ptr)
{
  char (*cells)[3] = game_ptr->cells;
  GameStatus status = GAME_CONTINUE; 

  if (game_ptr->current_turn > 9)
  {
    status = GAME_DRAW;
    game_ptr->is_running = 0;
  }
  else if (cells[y][x] == 'O' || cells[y][x] == 'X')	
  {
    status = GAME_ILLEGAL_TURN;
  }
  else
  {
    status = GAME_CONTINUE;

    if (player == 1)
      cells[y][x] = 'O';
    else if (player == 2) 
      cells[y][x] = 'X';

    if (status == GAME_CONTINUE && is_game_over(game_ptr))
    {
      if (player == 1)
        status = GAME_VICTORY_O;
      else
        status = GAME_VICTORY_X;

      game_ptr->is_running = 0;
    }
    game_ptr->current_turn++;
  }
  return status;
}

int is_game_over(Game* game_ptr)
{
  char (*cells)[3] = game_ptr->cells;
  // check rows
  for (int i = 0; i < 3; i++)
  {
    if (cells[i][0] != 0 && cells[i][0] == cells[i][1] && cells[i][0] == cells[i][2])
      return 1;
  }
  // Check columns
  for (int i = 0; i < 3; i++)
  {
    if (cells[0][i] != 0 && cells[0][i] == cells[1][i] && cells[0][i] == cells[2][i])
      return 1;
  }
  // Check diagonals
  if (cells[1][1] != 0 && (
      cells[0][0] == cells[1][1] && cells[0][0] == cells[2][2] ||
      cells[0][2] == cells[1][1] && cells[0][2] == cells[2][0] ))
      return 1;

  return 0;
}
