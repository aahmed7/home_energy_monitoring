#include <Arduino.h>
#include <ESP8266HTTPUpdateServer.h>
#include <ESP8266WebServer.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <WiFiClient.h>

#define MUX_PIN 2

// Wifi credentials.
const char* ssid = "Xiaomi_7E1B";
const char* password = "bonefire5628";

/* MQTT Credentials. */
const char* mqttBroker = "broker.hivemq.com";
const int   mqttPort = 1883;

/* MQTT Connection Parameters. */
const char* current_topic = "S2166341-esp8266/current_topic";
const char* voltage_topic = "S2166341-esp8266/voltage_topic";

/* Client Objects. */
WiFiClient   espClient;
PubSubClient client(espClient);

ESP8266WebServer        httpServer(80);
ESP8266HTTPUpdateServer httpUpdater;

int       sensorValue = 0;
const int analogInPin = A0;

void setup() {
    Serial.begin(74880);

    pinMode(MUX_PIN, OUTPUT);
    digitalWrite(MUX_PIN, LOW);

    /* Connect to WiFi. */
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(2000);
        Serial.println("Connecting to WiFi..");
    }
    Serial.println("Connected to the WiFi network");

    httpUpdater.setup(&httpServer);
    httpServer.begin();

    /* Register at the MQTT Broker. */
    client.setServer(mqttBroker, mqttPort);
    while (!client.connected()) {
        Serial.println("Connecting to MQTT...");
        if (client.connect(WiFi.macAddress().c_str())) {
            Serial.print("connected: ");
            Serial.println(WiFi.macAddress().c_str());
        } else {
            Serial.print("failed with state ");
            Serial.print(client.state());
            Serial.println();
            delay(2000);
        }
    }
}

void loop() {
    httpServer.handleClient();
    char buf[6] = "1234";

    // Send the current.
    digitalWrite(MUX_PIN, HIGH);
    sensorValue = analogRead(analogInPin);
    client.publish(current_topic, itoa(sensorValue, buf, 10));
    delay(50);

    // Send the voltage.
    digitalWrite(MUX_PIN, LOW);
    sensorValue = analogRead(analogInPin);
    client.publish(voltage_topic, itoa(sensorValue, buf, 10));
    delay(50);
}
