#####################################################################
#                                                                   #
# /labscript_devices/Eurocard_Synth/blacs_tab.py                        #
#                                                                   #
# Copyright 2021, Philip Starkey; edited by Oliver Tu               #
#                                                                   #
# This file is part of labscript_devices, in the labscript suite    #
# (see http://labscriptsuite.org), and is licensed under the        #
# Simplified BSD License. See the license.txt file in the root of   #
# the project for the full license.                                 #
#                                                                   #
#####################################################################
from blacs.device_base_class import (
    DeviceTab,
    define_state,
    MODE_BUFFERED,
    MODE_MANUAL,
    MODE_TRANSITION_TO_BUFFERED,
    MODE_TRANSITION_TO_MANUAL,
)
import labscript_utils.properties

from qtutils.qt import QtWidgets

class Eurocard_SynthTab(DeviceTab):
    def initialise_GUI(self):        
        self.units =    {'freq':'Hz',      'amp':'%'}
        self.min =      {'freq':2e8,      'amp':0}
        self.max =      {'freq':3e8,      'amp':100} 
        self.step =     {'freq':10000,      'amp':1}
        self.decimals = {'freq':0,         'amp':0} 
        
        # Create DDS Output objects: we only need frequency for the carrier and frequency and phase for the mod
        dds_prop = {}
        subchnl = 'freq'
        dds_prop['DDS_channel']={}
        dds_prop['DDS_channel'][subchnl]={'base_unit':self.units[subchnl],
                                                     'min':self.min[subchnl],
                                                     'max':self.max[subchnl],
                                                     'step':self.step[subchnl],
                                                     'decimals':self.decimals[subchnl]
                                                    }
        subchnl = 'amp'
        dds_prop['DDS_channel'][subchnl]={'base_unit':self.units[subchnl],
                                                     'min':self.min[subchnl],
                                                     'max':self.max[subchnl],
                                                     'step':self.step[subchnl],
                                                     'decimals':self.decimals[subchnl]
                                                    }
    #    Create the output objects    
        self.create_dds_outputs(dds_prop)        
        # Create widgets for output objects
        dds_widgets,ao_widgets,do_widgets = self.auto_create_widgets()
        # and auto place the widgets in the UI
        self.auto_place_widgets(("DDS settings",dds_widgets))
        
        connection_object = self.settings['connection_table'].find_by_name(self.device_name)
        connection_table_properties = connection_object.properties
        
        self.com_port = connection_table_properties.get('com_port', None)
        self.rack_index = connection_table_properties.get('rack_index', None)
        self.channel = connection_table_properties.get('channel', None)
        
        # Create and set the primary worker
        self.create_worker("main_worker",
                            'labscript_devices.Eurocard_Synth.blacs_workers.Eurocard_DDSWorker',
                            {'com_port':self.com_port,
                            'rack_index': self.rack_index,
                            'channel':self.channel})

        
        self.primary_worker = "main_worker"

        # Set the capabilities of this device
        self.supports_remote_value_check(False)
        self.supports_smart_programming(True) 
