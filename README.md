# Home energy monitoring using ESP8266

This project uses an ESP8266 to measure the electric power consumption. The following sensors
were used:

- ZMPT101b (For voltage measurement)
- SCT-013-030 (For current measurement)

ESP8266 has only one ADC, so a 4051 multiplexer was used to read multiple analog signals.

The 1st and 2nd commits use MQTT. The newer commits use InfluxDB 2.

TODO:

- Add circuit diagram
- Hardware testing is not complete
