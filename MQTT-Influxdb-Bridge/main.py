#!/usr/bin/env python3

"""A MQTT to InfluxDB Bridge
This script receives MQTT data and saves those to InfluxDB.

Thanks: https://github.com/Nilhcem/home-monitoring-grafana/blob/master/02-bridge/main.py
"""

import os
import re

from typing import NamedTuple

import paho.mqtt.client as mqtt

from influxdb import InfluxDBClient

INFLUXDB_ADDRESS = os.environ.get('INFLUXDB_ADDRESS')
INFLUXDB_USER = os.environ.get('INFLUXDB_USER')
INFLUXDB_PASSWORD = os.environ.get('INFLUXDB_PASSWORD')
INFLUXDB_DATABASE = os.environ.get('INFLUXDB_DATABASE')
INFLUXDB_PORT = os.environ.get('INFLUXDB_PORT', 8086)

MQTT_ADDRESS = os.environ.get('MQTT_ADDRESS')
MQTT_USER = os.environ.get('MQTT_USER')
MQTT_PASSWORD = os.environ.get('MQTT_PASSWORD')
MQTT_TOPIC = os.environ.get('MQTT_TOPIC')
MQTT_REGEX = os.environ.get('MQTT_REGEX')
MQTT_CLIENT_ID = os.environ.get('MQTT_CLIENT_ID')
MQTT_IGNORE_DEVICES = os.environ.get('MQTT_IGNORE_DEVICES', "")
MQTT_IGNORE_MEASUREMENTS = os.environ.get('MQTT_IGNORE_MEASUREMENTS', "")

influxdb_client = InfluxDBClient(INFLUXDB_ADDRESS, INFLUXDB_PORT, INFLUXDB_USER, INFLUXDB_PASSWORD, None)

IGNORED_DEVICES = MQTT_IGNORE_DEVICES.split(",")
IGNORED_MEASUREMENTS  = MQTT_IGNORE_MEASUREMENTS.split(",")

class SensorData(NamedTuple):
    device: str
    measurement: str
    value: float


def on_connect(client, userdata, flags, rc):
    """ The callback for when the client receives a CONNACK response from the server."""
    print('Connected with result code ' + str(rc))
    client.subscribe(MQTT_TOPIC)


def on_message(client, userdata, msg):
    """The callback for when a PUBLISH message is received from the server."""
    print(msg.topic + ' ' + str(msg.payload))
    sensor_data = _parse_mqtt_message(msg.topic, msg.payload.decode('utf-8'))
    if sensor_data is not None:
        _send_sensor_data_to_influxdb(sensor_data)


def _parse_mqtt_message(topic, payload):
    match = re.match(MQTT_REGEX, topic)
    if match:
        device = match.group(1)
        measurement = match.group(2)
        if device in IGNORED_DEVICES:
            return None
        if measurement in IGNORED_MEASUREMENTS:
            return None
        return SensorData(device, measurement, float(payload))
    else:
        return None


def _send_sensor_data_to_influxdb(sensor_data):
    json_body = [
        {
            'measurement': sensor_data.measurement,
            'tags': {
                'device': sensor_data.device
            },
            'fields': {
                'value': sensor_data.value
            }
        }
    ]
    influxdb_client.write_points(json_body)


def _init_influxdb_database():
    databases = influxdb_client.get_list_database()
    if len(list(filter(lambda x: x['name'] == INFLUXDB_DATABASE, databases))) == 0:
        influxdb_client.create_database(INFLUXDB_DATABASE)
    influxdb_client.switch_database(INFLUXDB_DATABASE)


def main():
    _init_influxdb_database()

    mqtt_client = mqtt.Client(MQTT_CLIENT_ID)
    mqtt_client.username_pw_set(MQTT_USER, MQTT_PASSWORD)
    mqtt_client.on_connect = on_connect
    mqtt_client.on_message = on_message

    mqtt_client.connect(MQTT_ADDRESS, 1883)
    mqtt_client.loop_forever()


if __name__ == '__main__':
    print('MQTT to InfluxDB bridge')
    main()