cmd_/home/sy/work/minipci/linux/driver/Module.symvers := sed 's/\.ko$$/\.o/' /home/sy/work/minipci/linux/driver/modules.order | scripts/mod/modpost -m -a  -o /home/sy/work/minipci/linux/driver/Module.symvers -e -i Module.symvers   -T -
