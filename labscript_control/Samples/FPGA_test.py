from labscript import *
from labscript_utils import import_or_reload
import_or_reload('labscriptlib.RbYbTweezer.connection_table')

# FPGA_DDS.setfreq(0,freq1)
# FPGA_DDS.setamp(0,amp1)

start()

t = 0

FPGA_DDS.set_param(t,freq2,amp2)

t += 1

for i in range(0,1):

    FPGA_DDS.ramp(t, ramp_time, start_freq,end_freq,start_amp,end_amp ,num_steps/ramp_time)

    t += ramp_time

    # Carrier.ramp(t, fc_ramp_time, ramp_stop, ramp_start, n_steps/fc_ramp_time)

    # t += fc_ramp_time

FPGA_DDS.set_param(t,end_freq,end_amp)

t += 10*ramp_time/num_steps

FPGA_DDS.set_param(t,end_freq,end_amp)

stop(t)