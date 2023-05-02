#####################################################################
#                                                                   #
# /Eurocard_Synth.py                                                   #
#                                                                   #
#                                                                   #
#                                                                   #
# This file is part of the module labscript_devices, in the         #
# labscript suite (see http://labscriptsuite.org), and is           #
# licensed under the Simplified BSD License. See the license.txt    #
# file in the root of the project for the full license.             #
#                                                                   #
#####################################################################


from labscript_devices import runviewer_parser

from labscript import IntermediateDevice, DDSQuantity, Device, config, LabscriptError, set_passed_properties
from labscript_utils.unitconversions import Eurocard_SynthFreqConversion, Eurocard_SynthAmpConversion
import labscript_utils.properties
import numpy as np
import time

__author__ = ['Oliver Tu']

class Eurocard_DDS(DDSQuantity):


    def set_param(self, t, freq, amp, units=None):
        print('Eurocard_DDS.setfreq(): compiling data table... ')
        self.setfreq(t,freq, units=units)
        self.setamp(t,amp, units=units)

    def ramp(self, t, duration, initial_freq, final_freq, initial_amp, final_amp, samplerate, units=None, truncation=1.):
        print('Eurocard_DDS.ramp(): compiling data table... ')
        self.frequency.ramp(t, duration, initial_freq, final_freq, samplerate, units=units, truncation=truncation)
        self.amplitude.ramp(t, duration, initial_amp, final_amp, samplerate, units=units, truncation=truncation)

class Eurocard_Synth(IntermediateDevice):
    """
    This class is initilzed with the key word argument
    'TCPIP_address',  -- 
    'TCPIP_port' -- 
    """
    description = 'Eurocard_DDS'
    allowed_children = [Eurocard_DDS]

    @set_passed_properties(
        property_names={
            'connection_table_properties': [
                'com_port',
                'rack_index',
                'channel'
            ]
        }
    )
    def __init__(
        self,
        name,
        parent_device,
        channel,
        com_port="",
        rack_index=None,
        **kwargs
    ):
        IntermediateDevice.__init__(self, name, parent_device, **kwargs)
        self.BLACS_connection = 'serial:%s'%(com_port)
        self.rack_index = rack_index
        self.com_port = com_port
        self.channel = channel
        if self.rack_index == None and self.com_port=="":
            raise TypeError("Rack index and Com port cannot be both Null: For single module, only assign the com_port of the connection; for multiple-module-on-rack, assign the rack_index and do/don't assign a com_port if the module is a Master/Slave.")

    def add_device(self, device):
        Device.add_device(self, device)
        device.frequency.default_value = 1e8
        device.amplitude.default_value = 100
        if self.rack_index != None:
            if self.com_port != "":
                global Eurocard_COMlist
                try:
                    Eurocard_COMlist[str(self.rack_index)]=self.com_port
                except:
                    Eurocard_COMlist={}
                    Eurocard_COMlist[str(self.rack_index)]=self.com_port

            
    def get_default_unit_conversion_classes(self, device):
        """Child devices call this during their __init__ (with themselves
        as the argument) to check if there are certain unit calibration
        classes that they should apply to their outputs, if the user has
        not otherwise specified a calibration class"""
        return Eurocard_SynthFreqConversion, Eurocard_SynthAmpConversion, None
    
    # def quantise_freq(self, data, device):
    #     """Provides bounds error checking and scales input values to instrument
    #     units (0.1 Hz) before ensuring uint32 integer type."""
    #     if not isinstance(data, np.ndarray):
    #         data = np.array(data)
    #     # Ensure that frequencies are within bounds:
    #     if np.any(data > 5e8 )  or np.any(data < 1 ):  #base unit Hz
    #         raise LabscriptError('%s %s '%(device.description, device.name) +
    #                           'can only have frequencies between 500MHz and 1Hz, ' + 
    #                           'the limit imposed by %s.'%self.name)
    #     scale_factor = (2 ** 32 - 1 )/ 1e9
    #     data = np.array(data * scale_factor,dtype=np.uint32)
    #     return data
        
    # def quantise_amp(self,data,device):
    #     if not isinstance(data, np.ndarray):
    #         data = np.array(data)
    #     # ensure that amplitudes are within bounds:
    #     if np.any(data > 100 )  or np.any(data < 0):
    #         raise LabscriptError('%s %s '%(device.description, device.name) +
    #                           'can only have amplitudes between 0 and 100%, ' + 
    #                           'the limit imposed by %s.'%self.name)
    #     scale_factor = 2 ** 20 - 1
    #     data = np.array(scale_factor*data,dtype=np.uint32)
    #     return data
        
    def generate_code(self, hdf5_file):
        # DDSs = {}
        # error check and make a list of the two devices:
        # for output in self.child_devices:
            # Check that the instructions will fit into RAM:
            # if isinstance(output, Eurocard_DDS) and len(output.frequency.raw_output) > 5000 - 2: # -2 to include space for dummy instructions TODO: check the RAM
            #     raise LabscriptError('%s can only support XXXX instructions. '%self.name +
            #                          'Please decrease the sample rates of devices on the same clock, ' + 
            #                          'or connect %s to a different pseudoclock.'%self.name)
            # try:
            #     int(output.connection)
            #     DDSs[output.connection] = output
            # except:
            #     raise TypeError("%s must have an integer string connection index" % output.name)
        dds = self.child_devices[0]

        dtypes = {'names':['freq','amp'],'formats':[np.uint32,np.uint8]}

        clockline = self.parent_clock_line
        pseudoclock = clockline.parent_device
        times = pseudoclock.times[clockline]
        # print("time when started preparing h5file:"+str(time.time())+"\n")
            # print('time when ramp finished:'+str(time.time()))
        out_table = np.zeros(len(times),dtype=dtypes)# create the table data for the hdf5 file.
        out_table['freq'].fill(1) # set them to 1
        out_table['freq'][:] = dds.frequency.raw_output
        out_table['amp'].fill(1) # set them to 1
        out_table['amp'][:] = dds.amplitude.raw_output
        # print("time when started creating h5file:"+str(time.time())+"\n")
        if self.rack_index == None:
            grp = self.init_device_group(hdf5_file)
        else:
            grp = hdf5_file['/devices/Eurocard_DDS%i'%self.rack_index]
            if not grp:
                grp = hdf5_file['/devices'].create_group('Eurocard_DDS%i'%self.rack_index)
        grp.create_dataset('TABLE_DATA%i'%self.channel,compression=config.compression,data=out_table) 
        print("time when ended creating h5file:"+str(time.time())+"\n")
