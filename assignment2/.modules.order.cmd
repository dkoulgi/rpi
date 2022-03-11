cmd_/home/pi/assignment2/modules.order := {   echo /home/pi/assignment2/cycle_count_mod.ko; :; } | awk '!x[$$0]++' - > /home/pi/assignment2/modules.order
