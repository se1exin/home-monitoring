import os
import paho.mqtt.client as mqtt
import serial
import time

MQTT_ADDRESS = os.environ.get('MQTT_ADDRESS')
MQTT_USER = os.environ.get('MQTT_USER')
MQTT_PASSWORD = os.environ.get('MQTT_PASSWORD')
MQTT_CLIENT_ID = os.environ.get('MQTT_CLIENT_ID')
SERIAL_BAUD_RATE = os.environ.get('SERIAL_BAUD_RATE', 9600)

ser = serial.Serial("/dev/ttyS0")
ser.baudrate = SERIAL_BAUD_RATE

def on_connect(client, userdata, flags, rc):
    print('Connected with result code ' + str(rc))

def on_disconnect(client, userdata, flags, rc):
    print('Disconnected with result code ' + str(rc))

def main():

    mqtt_client = mqtt.Client(MQTT_CLIENT_ID)
    mqtt_client.username_pw_set(MQTT_USER, MQTT_PASSWORD)
    mqtt_client.on_connect = on_connect
    mqtt_client.on_disconnect = on_disconnect

    mqtt_client.connect(MQTT_ADDRESS, 1883)
    mqtt_client.loop_start()

    while True:
    	data = ser.readline()
        print("Recieved: " + data)
        parts = str(data).split(",")
        sensor_topic = parts[0]
        sensor_reading = parts[1]
    	mqtt_client.publish(sensor_topic, sensor_reading)

if __name__ == '__main__':
    print('Serial to MQTT bridge')
    main()