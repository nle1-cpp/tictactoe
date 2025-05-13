// tictactoe logic
// following every action...
// check 3 rows, 3 columns, and 2 diagonals
// terminate if all 3 cells are of a kind
#include <stdio.h>
#include "types.h"

void setup_game(Game* game_ptr)
{
  memset(game_ptr->cells, 0, sizeof(game_ptr->cells));
  game_ptr->current_player = 'O';
  game_ptr->current_turn = 1; // or 0 idk
  game_ptr->is_running = 0;
}

void new_game(Game* game_ptr, char first_player)
{
  memset(game_ptr->cells, 0, sizeof(game_ptr->cells));
  game_ptr->current_player = first_player;
  game_ptr->current_turn = 1;
  game_ptr->is_running = 1;
}

void alt_player(Game* game_ptr)
{
  if (game_ptr->current_player == 'O')
    game_ptr->current_player = 'X';
  else
    game_ptr->current_player = 'O';
}

void print_board(Game* game_ptr)
{
  Serial.println("Current board state:");
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

GameStatus add_to_board(char player, int x, int y, Game* game_ptr)
{
  GameStatus status = GAME_CONTINUE; 

  if (game_ptr->cells[y][x] == 'O' || game_ptr->cells[y][x] == 'X')	
  {
    status = GAME_ILLEGAL_TURN;
  }
  else
  {
    status = GAME_CONTINUE;

    game_ptr->cells[y][x] = player; // designate selected cell to player 

    if (status == GAME_CONTINUE && is_game_won(game_ptr))
    {
      if (player == 'O')
        status = GAME_VICTORY_O;
      else
        status = GAME_VICTORY_X;

      game_ptr->is_running = 0;
    }
    else if (status == GAME_CONTINUE && game_ptr->current_turn == 9)
    {
      status = GAME_DRAW;
      game_ptr->is_running = 0;
    }

    game_ptr->current_turn++;
    alt_player(game_ptr);
  }
  return status;
}

int is_game_won(Game* game_ptr)
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
