from labscript import *

from labscript_devices.PulseBlasterUSB import PulseBlasterUSB
from labscript_devices.TCPDH_Synth.labscript_devices import *
from labscript_devices.FPGA_Synth.labscript_devices import *

# For dummy pseudoclock
# from labscript_devices.DummyPseudoclock.labscript_devices import DummyPseudoclock
# from labscript_devices.DummyIntermediateDevice import DummyIntermediateDevice

#############################################################
#   CONNECTION TABLE
#############################################################

# Use a virtual, or 'dummy', device for the psuedoclock
# DummyPseudoclock(name='pulseblaster_0')

#############################################################
#   PULSEBLASTER
#############################################################

PulseBlasterUSB(name='pulseblaster_0', board_number=0, time_based_stop_workaround=True, time_based_stop_workaround_extra_time=0.5)

# ClockLine(name='pulseblaster_TCPDH_clockline', pseudoclock=pulseblaster_0.pseudoclock,connection='flag 0')
ClockLine(name='pulseblaster_FPGA_clockline', pseudoclock=pulseblaster_0.pseudoclock,connection='flag 0')
# TCPDH_Synth(name='TCPDH_controller',TCPIP_address='192.168.0.101',TCPIP_port=6750,parent_device=pulseblaster_TCPDH_clockline,update_mode='synchronous')

# TCPDH_DDS(name='Carrier', parent_device=TCPDH_controller, connection='PDH_carrier')
# TCPDH_StaticDDS(name='Modulation', parent_device=TCPDH_controller, connection='PDH_mod')
FPGA_Synth(name='FPGA_controller',TCPIP_address='192.168.1.106',TCPIP_port=5000,parent_device=pulseblaster_FPGA_clockline)
FPGA_DDS(name='FPGA_DDS', parent_device=FPGA_controller, connection='DDS_channel')

# DummyIntermediateDevice(name='AOut',parent_device=pseudoclock.clockline)
# AnalogOut(name="TestOut",parent_device=AOut,connection = 'dummy_connection')


# DigitalOut(name='pb_0', parent_device = pulseblaster_0.direct_outputs, connection = 'flag 0')
DigitalOut(name='pb_1', parent_device = pulseblaster_0.direct_outputs, connection = 'flag 1')
DigitalOut(name='pb_2', parent_device = pulseblaster_0.direct_outputs, connection = 'flag 2')
DigitalOut(name='pb_3', parent_device = pulseblaster_0.direct_outputs, connection = 'flag 3')
DigitalOut(name='pb_4', parent_device = pulseblaster_0.direct_outputs, connection = 'flag 4')
DigitalOut(name='pb_5', parent_device = pulseblaster_0.direct_outputs, connection = 'flag 5')
DigitalOut(name='pb_6', parent_device = pulseblaster_0.direct_outputs, connection = 'flag 6')
DigitalOut(name='pb_7', parent_device = pulseblaster_0.direct_outputs, connection = 'flag 7')
DigitalOut(name='pb_8', parent_device = pulseblaster_0.direct_outputs, connection = 'flag 8')
DigitalOut(name='pb_9', parent_device = pulseblaster_0.direct_outputs, connection = 'flag 9')

if __name__ == '__main__':
    start()
    # FPGA_DDS.set_param(0,3e8,0.5)
    stop(1)

