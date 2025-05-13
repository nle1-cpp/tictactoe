#!/bin/bash

hostname="localhost"
player="X"
player_low="x"
opp="O"
i=0

sleep 1
mosquitto_pub -h localhost -t "ttt/moves" -m "READY_$player"

# play until game end loop
game_over=0
while [[ $game_over -eq 0 ]]; do
	echo "$player: loop start"

	# listen for "TURN_$player" from "ttt/game/status"
	# on recieved response ...
	# publish move information to "ttt/moves"
	# listen for response from "ttt/auth/o"
	# if "OK"
	# next loop
	# if "ILLEGAL_MOVE"
	# retry publish to "ttt/moves" with new info and listen to auth response

	# reads each publish from "ttt/game/status"
	# if publish is TURN_$player, proceed
	declare status	
	while 

		status=$(mosquitto_sub -h $hostname -t "ttt/board/status" -C 1)

		if [[ $status != "TURN_$player" && $status != "TURN_$opp" ]];  then
			game_over=1
			echo "$player: game over: $status"
			break
		fi
		echo "$status"
		
	[[ $status != "TURN_$player" ]]
	do : ; done

	if [[ $game_over -eq 1 ]]; then
		break
	fi

	echo "Player $player is taking their turn"

	# publishes game position
	# waits for validation response
	# repeat until valid
	declare auth
	while 
		rand_x=$((RANDOM % 3))
		rand_y=$((RANDOM % 3))
		move="$player$rand_x$rand_y"
	
		echo "Player $player attempting move at $rand_x,$rand_y"

		mosquitto_pub -h $hostname -t "ttt/moves" -m "$move"
		auth=$(mosquitto_sub -h $hostname -t "ttt/auth/$player_low" -C 1)
		
		echo "$auth"
	[[ $auth = "ILLEGAL_MOVE" ]]
	do : ; done

	echo "$player: loop end"
done

echo "$player: end of game"
