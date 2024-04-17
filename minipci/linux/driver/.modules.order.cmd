cmd_/home/sy/work/minipci/linux/driver/modules.order := {   echo /home/sy/work/minipci/linux/driver/minipci.ko; :; } | awk '!x[$$0]++' - > /home/sy/work/minipci/linux/driver/modules.order
