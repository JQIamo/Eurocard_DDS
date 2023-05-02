from labscript import *
from labscript_utils import import_or_reload
import_or_reload('labscriptlib.RbYbTweezer.connection_table')



start()

t = 0
E_DDS1.set_param(t,f_start,a_start)


for i in range(0,n_reps):

    E_DDS1.ramp(t, fc_ramp_time, f_start,f_stop,a_start,a_stop, n_steps/fc_ramp_time)

    t += fc_ramp_time

    E_DDS1.ramp(t, fc_ramp_time, f_stop, f_start,a_stop,a_start, n_steps/fc_ramp_time)

    t += fc_ramp_time

E_DDS1.set_param(t,f_start,a_start)

stop(t)