#####################################################################
#                                                                   #
# generic_frequency.py                                              #
#                                                                   #
# Copyright 2022, Monash University and contributors                #
#                                                                   #
# This file is part of the labscript suite (see                     #
# http://labscriptsuite.org) and is licensed under the Simplified   #
# BSD License. See the license.txt file in the root of the project  #
# for the full license.                                             #
#                                                                   #
#####################################################################
"""Generic frequency conversion"""

from .UnitConversionBase import *

class Eurocard_SynthFreqConversion(UnitConversion):
    base_unit = 'Hz' # must be defined here and match default hardware unit in BLACS tab

    def __init__(self, calibration_parameters = None):
        self.parameters = calibration_parameters
        if hasattr(self, 'derived_units'):
            self.derived_units += ['kHz', 'MHz']
        else:
            self.derived_units = ['kHz', 'MHz']
        UnitConversion.__init__(self,self.parameters)
    
    def kHz_to_base(self,kHz):
        Hz = kHz*1e3
        return Hz

    def kHz_from_base(self,Hz):
        kHz = Hz*1e-3
        return kHz

    def MHz_to_base(self,MHz):
        Hz = MHz*1e6
        return Hz

    def MHz_from_base(self,Hz):
        MHz = Hz*1e-6
        return MHz