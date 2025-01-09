import serial
import time
ser = serial.Serial('COM3')
# '''
ser.write(b'@ 5\r\n')
ser.write(b'w 83578000 0 94\r\n')
# ser.write(b'f 137000000\r\n')
# ser.write(b'fl 137000000 138000000\r\n')
# ser.write(b'w 300000 0 25\r\n')
# ser.write(b'w 200000 0 10\r\n')
# ser.write(b'w 10000 0 50\r\n')
# '''
# ser.write(b'a 0\r\n')
ser.write(b'$\r\n')
ser.close()