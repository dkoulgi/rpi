cmd_/home/pi/assignment2/Module.symvers := sed 's/\.ko$$/\.o/' /home/pi/assignment2/modules.order | scripts/mod/modpost -m -a  -o /home/pi/assignment2/Module.symvers -e -i Module.symvers   -T -
