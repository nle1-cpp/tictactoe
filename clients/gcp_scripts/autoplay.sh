#!/bin/bash

(trap 'kill 0' SIGINT; ~/bot_o.sh & ~/bot_x.sh & wait)