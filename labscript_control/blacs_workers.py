#####################################################################
#                                                                   #
# /labscript_devices/Eurocard_DDS/blacs_worker.py                       #
#                                                                   #
# This file is part of labscript_devices                            #
#                                                                   #
#####################################################################
# from asyncio.log import loggerf
import logging
from formatter import NullFormatter
from os import times
import re
import select, serial
import labscript_utils.h5_lock
import h5py
import numpy as np
from blacs.tab_base_classes import Worker
from labscript_utils.connections import _ensure_str
import labscript_utils.properties as properties
from struct import pack
from time import time
import sys
from time import sleep


class Eurocard_DDSWorker(Worker):
    def init(self):
        # logging.basicConfig(level=logging.DEBUG) # Use this line to debugging 
        global serial; import serial
        global h5py; import labscript_utils.h5_lock, h5py 
        self.smart_cache = np.array([-1])
        self.connection_check()
   
    def connection_check(self):
        self.connection = serial.Serial(self.com_port, baudrate=115200, timeout=0.1)
        try:
            self.connection.write(b'^')
            checker = self.connection.read()
        except ConnectionRefusedError as msg:
            self.logger.debug(f"ConnectionRefusedError: Communication failed. Check physical Serial connection")
            raise
        if checker != b'O':
            raise ConnectionError(f"Error in connection attempt: Expect 'O' but received {str(checker.decode())}.")
        self.connection.close()

    def program_manual(self,front_panel_values):
        self.connection = serial.Serial(self.com_port, baudrate=115200, timeout=0.1)
        data_string = '@ %i\r\n'%self.channel
        f =front_panel_values['DDS_channel']['freq']
        if f>3e8:
            f=3e8
        elif f<2e8:
            f=2e8
        command = 'w %i 0 %i\r\n'%(f,front_panel_values['DDS_channel']['amp'])
        data_string += command
        data_string += '$\r\n'
        self.connection.write(data_string.encode())
        self.connection.close()
        print(data_string)
        return front_panel_values

    def transition_to_buffered(self,device_name,h5file,initial_values,fresh):
        final_values = initial_values
        if self.rack_index == None:
            table_data = None
            with h5py.File(h5file, 'r') as hdf5_file:
                group = hdf5_file['/devices/'+device_name]
                if 'TABLE_DATA%i'%self.channel in group:
                    table_data = group['TABLE_DATA%i'%self.channel][:]
            if table_data is not None:
                data = table_data
                amp_list = data['amp'][:]
                freq_list = data['freq'][:]
                amp_list = np.concatenate(([amp_list[0],amp_list[0]],amp_list)) # PB trigger synch
                freq_list = np.concatenate(([freq_list[0],freq_list[0]],freq_list))
                data_string = '@ %i\r\n'%self.channel
                for i in range(len(amp_list)):
                    data_string += 'w %i 0 %i\r\n'%(freq_list[i],amp_list[i])
                data_string += '$\r\n'
                self.connection = serial.Serial(self.com_port, baudrate=115200, timeout=0.1)
                self.connection.write(data_string.encode())
                final_values = {'Eurocard_DDS':{}}
                final_values['Eurocard_DDS']['freq'] = freq_list[-1]
                final_values['Eurocard_DDS']['amp'] = amp_list[-1]
                self.connection.close()
        else:
            data_string = ''
            with h5py.File(h5file, 'r') as hdf5_file:
                group = hdf5_file['/devices/Eurocard_DDS%i'%self.rack_index]
                for j in range(11):
                    if 'TABLE_DATA%i'%j in group and group['TABLE_DATA%i'%j][:] is not None:
                        if self.channel == j:
                            for k in range(j,11):
                                if 'TABLE_DATA%i'%k in group and group['TABLE_DATA%i'%k][:] is not None:
                                    data = group['TABLE_DATA%i'%k][:]
                                    amp_list = data['amp'][:]
                                    freq_list = data['freq'][:]
                                    data_string += '@ %i\r\n'%k
                                    for i in range(len(amp_list)):
                                        data_string += 'w %i 0 %i\r\n'%(freq_list[i],amp_list[i])
                                    if k==j:
                                        final_values = {'Eurocard_DDS':{}}
                                        final_values['Eurocard_DDS']['freq'] = freq_list[-1]
                                        final_values['Eurocard_DDS']['amp'] = amp_list[-1]
                    break            
            if data_string != '':
                self.connection = serial.Serial(self.com_port, baudrate=115200, timeout=0.1)
                data_string += '$\r\n'
                self.connection.write(data_string.encode())
                print('Data sent\n')
                self.connection.close()
        return final_values    

    
    def abort_transition_to_buffered(self):
        return self.transition_to_manual(True)
        
    def abort_buffered(self):
        return self.transition_to_manual(True)
    
    def transition_to_manual(self,abort = False):
        return True

    def shutdown(self):
        pass

     
