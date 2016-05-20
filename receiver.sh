#!/bin/bash

## User Configuration #########################################################
DEBUG=1
BLE_MAC=FC:28:32:08:E0:B3

#
# button map
#   ------------------  ------------------
#   | 00  01  02  03 |  | qq  ww  ii  pp |
#   | 04  05  06  07 |  | aa  ss  dd  ++ |
#   | 08  09  0a  0b |  | ff  w-  w+  -- |
#   | 0c  0d  0e  0f |  | su  <<  >>  pp |
#   ------------------  ------------------

function receiver_browse {
    # if we're in mpv, press o (to display time)
    # otherwise, spawn a browser
    if [[ $(xdotool getwindowfocus getwindowname) =~ "mpv" ]]; then
        xdotool key o
    else
        urxvt -e ranger ~ &
    fi
}
function receiver_cycle_win {
    let "num_desktops = $(xdotool get_num_desktops)"
    let "cur_win = $(xdotool getwindowfocus)"
    let "cur_desktop = $(xdotool get_desktop)"
    # build wins array
    wins=()
    OLDIFS=$IFS
    IFS=$'\n'   # used for string split
    for ((i = 0; i < $num_desktops; i++)) {
        # use onlyvisible flag for current desktop
        if [[ $i == $cur_desktop ]]; then
            opt="--onlyvisible"
        else
            opt=""
        fi

        while read -r line; do wins+=("$line");
        done <<<"$(xdotool search $opt --desktop $i . 2> /dev/null)"
    }
    IFS=$OLDIFS

    # find cur_win in array and set next to cur + $1
    let "next = 0"
    for ((i = 0; i < ${#wins[@]}; i++)) {
        if [[ ${wins[$i]} == $cur_win ]]; then
            let "next = ($i + $1 + ${#wins[@]}) % ${#wins[@]}";
            break
        fi
    }

    # set focus to next
    xdotool set_desktop $(xdotool get_desktop_for_window ${wins[$next]}) && \
        xdotool windowfocus ${wins[$next]}
}

# map of button to command
declare -A receiver_map=(
    ["00"]="xdotool key q"
    ["01"]="xdotool key k"
    ["02"]="receiver_browse"
    ["03"]="xdotool key p"
    ["04"]="xdotool key h"
    ["05"]="xdotool key j"
    ["06"]="xdotool key l"
    ["07"]="xdotool key 0"
    ["08"]="xdotool key f"
    ["09"]="receiver_cycle_win -1"
    ["0a"]="receiver_cycle_win 1"
    ["0b"]="xdotool key 9"
    ["0c"]="xdotool key v"
    ["0d"]="xdotool key Left"
    ["0e"]="xdotool key Right"
    ["0f"]="xdotool key p"
)
################################################################################

GATT_OPTS="--device=$BLE_MAC --addr-type=random"
GATT_READ_OPTS="--char-write-req --handle=0x0e --value=0100 --listen"

function receiver_callback {
    if [[ $DEBUG == 1 ]]; then
        echo "Received: $1"
    fi
    # run command if it's in the map
    if [[ ! -z "${receiver_map[$1]}" ]]; then
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
        if [[ $DEBUG == 1 ]]; then
            echo $line
        fi
    fi
done
