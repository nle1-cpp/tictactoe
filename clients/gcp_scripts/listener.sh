#!/bin/bash

while [[ true ]]; do
	
	response=$(mosquitto_sub -h localhost -t "ttt/bots" -C 1)

	if [[ $response == "START_AUTOPLAY" ]]; then
		~/ttt/autoplay.sh
	elif [[ $response == "START_BOT_O" ]]; then
		~/ttt/bot_o.sh
	elif [[ $response == "START_BOT_X" ]]; then
		~/ttt/bot_x.sh
	else
		echo "listener recieved an invalid command."	
	fi

done
