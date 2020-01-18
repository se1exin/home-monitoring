docker run \
	-e MQTT_ADDRESS="10.1.1.100" \
	-e MQTT_CLIENT_ID="SerialMQTTBridge" \
	-e SERIAL_BAUD_RATE="9600" \
	--device /dev/ttyS0:/dev/ttyS0 \
	--privileged \
	-it selexin/serial-mqtt-bridge:latest
