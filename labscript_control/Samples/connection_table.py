from labscript import *

from labscript_devices.PulseBlasterUSB import PulseBlasterUSB
from labscript_devices.Eurocard_Synth.labscript_devices import *

#############################################################
#   PULSEBLASTER
#############################################################

PulseBlasterUSB(name='pulseblaster_0', board_number=0, time_based_stop_workaround=True, time_based_stop_workaround_extra_time=0.5)

ClockLine(name='pulseblaster_E_clockline1', pseudoclock=pulseblaster_0.pseudoclock,connection='flag 0')

Eurocard_Synth(name='E_controller1',com_port='COM5',channel=1,parent_device=pulseblaster_E_clockline1)
Eurocard_DDS(name='E_DDS1', parent_device=E_controller1, connection='DDS_channel')

if __name__ == '__main__':
    start()
    stop(1)

