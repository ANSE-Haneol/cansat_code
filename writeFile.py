import serial

ser = serial.Serial('ENTER_YOUR_PORT_NUMBER', 9600)
f = open("C:/cansat_code/cansatData.txt", 'w')
f.close()

while True:
    if ser.readable():
        f = open("C:/cansat_code/cansatData.txt", 'a')
        data = ser.readline()
        decVal = data.decode('UTF-8')
        f.write(str(decVal))
        f.close()
