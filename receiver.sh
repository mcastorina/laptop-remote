#!/bin/sh
gatttool -b FC:28:32:08:E0:B3 -t random --char-write-req --handle=0x0e --value=0100 --listen
