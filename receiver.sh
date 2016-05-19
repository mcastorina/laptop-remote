#!/bin/bash
DEBUG=1

BLE_MAC=FC:28:32:08:E0:B3
GATT_OPTS="--device=$BLE_MAC --addr-type=random"
GATT_READ_OPTS="--char-write-req --handle=0x0e --value=0100 --listen"

function receiver_map {
    if [[ $DEBUG ]]; then
        echo "Received: $1"
    fi
}


# gatttool will loop forever
# this loop will call receiver_map when it receives a value
gatttool $GATT_OPTS $GATT_READ_OPTS | while read -r line; do
    if [[ $line =~ "Characteristic value was written successfully" ]]; then
        echo "Connected successfully"
    elif [[ $line =~ "Notification handle = 0x000d value:" ]]; then
        if [[ $line =~ ([0-9a-f][0-9a-f])$ ]]; then
            receiver_map ${BASH_REMATCH[1]}
        fi
    else
        echo $line
    fi
done
