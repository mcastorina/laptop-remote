#!/bin/bash
DEBUG=1

BLE_MAC=FC:28:32:08:E0:B3
GATT_OPTS="--device=$BLE_MAC --addr-type=random"
GATT_READ_OPTS="--char-write-req --handle=0x0e --value=0100 --listen"

MOVE_AMT=10
#
# button map
#   00  01  02  03
#   04  05  06  07
#   08  09  0a  0b
#   0c  0d  0e  0f
# map of button to command
declare -A receiver_map=(
    ["05"]="xdotool mousemove_relative -- -$MOVE_AMT -$MOVE_AMT"
    ["06"]="xdotool mousemove_relative --  0         -$MOVE_AMT"
    ["07"]="xdotool mousemove_relative --  $MOVE_AMT -$MOVE_AMT"
    ["09"]="xdotool mousemove_relative -- -$MOVE_AMT  0"
    ["0a"]="xdotool click 1"
    ["0b"]="xdotool mousemove_relative --  $MOVE_AMT  0"
    ["0d"]="xdotool mousemove_relative -- -$MOVE_AMT  $MOVE_AMT"
    ["0e"]="xdotool mousemove_relative --  0          $MOVE_AMT"
    ["0f"]="xdotool mousemove_relative --  $MOVE_AMT  $MOVE_AMT"
)

function receiver_callback {
    if [[ $DEBUG ]]; then
        echo "Received: $1"
    fi
    if [[ ! -z "${receiver_map[$1]}" ]]; then
        # run command
        ${receiver_map[$1]}
    fi
}
# gatttool will loop forever
# this loop will call receiver_callback when it receives a value
gatttool $GATT_OPTS $GATT_READ_OPTS | while read -r line; do
    if [[ $line =~ "Characteristic value was written successfully" ]]; then
        echo "Connected successfully"
    elif [[ $line =~ "Notification handle = 0x000d value:" ]]; then
        if [[ $line =~ ([0-9a-f][0-9a-f])$ ]]; then
            receiver_callback ${BASH_REMATCH[1]}
        fi
    else
        echo $line
    fi
done
