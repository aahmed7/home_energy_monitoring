#include <Arduino.h>
#include <ESP8266HTTPUpdateServer.h>
#include <ESP8266WebServer.h>
#include <ESP8266WiFi.h>
#include <InfluxDbClient.h>
#include <InfluxDbCloud.h>
#include <WiFiClient.h>
#include <credentials.h>
#include <math.h>

#define MUX_PIN               2
#define RMS_SAMPLING          100
#define RMS_SAMPLING_DELAY_MS 1
#define DEFAULT_FREQ          50.0

// Analog pin.
const int analogInPin = A0;

// Timekeeping.
unsigned long prev_time;

// InfluxDB point.
Point energy("energy-test2");

// HTTP server global variables.
ESP8266WebServer        httpServer(80);
ESP8266HTTPUpdateServer httpUpdater;

// InfluxDB client instance with preconfigured InfluxCloud certificate
InfluxDBClient influxClient(INFLUXDB_URL, INFLUXDB_ORG, INFLUXDB_BUCKET, INFLUXDB_TOKEN,
                            InfluxDbCloud2CACert);

void handleID() {
    httpServer.send(200, "text/plain",
                    WiFi.macAddress().c_str());  // Send HTTP status 200 (Ok) and send
                                                 // some text to the browser/client
}

void handleNotFound() {
    httpServer.send(404, "text/plain",
                    "404: Not found");  // Send HTTP status 404 (Not Found) when there's
                                        // no handler for the URI in the request
}

// convert RAW value to current
double readCurrent() {
    // Calibration values, need to be calculated using manual testing.
    double R1 = 21.8;
    double R2 = 9.8;
    double R3 = 9.8;
    double R4 = 10.0;
    double regV = 3.25;
    double offset = (R2 / (R1 + R2) * regV); // Offset should be about 1V
    double multiplier = (R3 + R4) / R4;
    double calibration = 0.75;

    // Using SCT-010-30. Outputs 1V per 30A
    double VperA = 1/20.0;

    // Change mux to current.
    digitalWrite(MUX_PIN, HIGH);
    return (((analogRead(analogInPin)/1024.0 * multiplier) - offset) / VperA) * calibration;
}

// convert RAW value to voltage
// Using ZMPT101b
double readVoltage() {
    // Calibration values, need to be calculated using manual testing.
    int    offset = 302;
    double multiplier = 5.4;
    // Change mux to voltage.
    digitalWrite(MUX_PIN, LOW);
    return (analogRead(analogInPin) - offset) * multiplier;
}

// Get the RMS current.
double calcRMSCurrent() {
    double rmsBuff[RMS_SAMPLING];
    double sum = 0.0;

    // Get RMS_SAMPLING samples with 1ms delay.
    for (int i = 0; i < RMS_SAMPLING; i++) {
        rmsBuff[i] = readCurrent();  // convert RAW value to voltage
        delay(RMS_SAMPLING_DELAY_MS);
    }

    for (int i = 0; i < RMS_SAMPLING; i++) {
        sum += (rmsBuff[i] * rmsBuff[i]);
    }
    sum = sum / RMS_SAMPLING;
    return sqrt(sum);
}

// Get the RMS voltage.
double calcRMSVoltage() {
    double rmsBuff[RMS_SAMPLING];
    double sum = 0.0;

    // Get 100 samples with 1ms delay.
    for (int i = 0; i < RMS_SAMPLING; i++) {
        rmsBuff[i] = readVoltage();
        delay(RMS_SAMPLING_DELAY_MS);
    }

    for (int i = 0; i < RMS_SAMPLING; i++) {
        sum += (rmsBuff[i] * rmsBuff[i]);
    }
    sum = sum / RMS_SAMPLING;
    return sqrt(sum);
}

double calcFreq(bool def) {
    if (def) return (DEFAULT_FREQ);
    double        freq = 0.0;
    unsigned long interval = 200;  // interval in ms
    float multiplier = 1000.0 / interval;  // interval in ms

    // Count the zero crossings.
    unsigned long startTime = millis();
    while ((millis() - startTime) < interval) {
        if (abs(readVoltage()) < 10.0) {
            delay(5);
            freq += 1;
        } else {
            delayMicroseconds(100);
        }
    }

    // One sine wave has two zero crossings.
    return (freq / 2.0 * multiplier);
}

// Assuming lagging phase.
double calcPF() {
    unsigned long vtime = 0;
    unsigned long ctime = 0;
    unsigned long phaseDiff = 0;

    // Change mux to voltage.
    digitalWrite(MUX_PIN, LOW);
    while (abs(analogRead(analogInPin)) > 0.5) {
        delayMicroseconds(100);
    }
    vtime = micros();

    // Change mux to current.
    digitalWrite(MUX_PIN, HIGH);
    while (abs(analogRead(analogInPin)) > 5) {
        delayMicroseconds(100);
    }
    ctime = micros();

    phaseDiff = ctime - vtime;

    // phaseAngle = (phaseDiff/calcFreq()) * 360;
    return cos((phaseDiff / calcFreq(false)) * 360.0);
}

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
    httpServer.on("/id", handleID);
    httpServer.onNotFound(handleNotFound);

    httpServer.begin();

    timeSync(TZ_INFO, NTP_SERVER1, NTP_SERVER2);

    // Add constant tags - only once
    energy.addTag("device", WiFi.macAddress());
    // current.addTag("device", "ESP8266");

    // Check server connection
    if (influxClient.validateConnection()) {
        Serial.print("Connected to InfluxDB: ");
        Serial.println(influxClient.getServerUrl());
    } else {
        Serial.print("InfluxDB connection failed: ");
        Serial.println(influxClient.getLastErrorMessage());
    }
}

void loop() {
    httpServer.handleClient();
    energy.clearFields();

    // Sync time after 1000 secs
    if ((millis() - prev_time) > 1000000) {
        prev_time = millis();
        timeSync(TZ_INFO, NTP_SERVER1, NTP_SERVER2);
    }

    energy.addField("current", calcRMSCurrent());
    energy.addField("voltage", calcRMSVoltage());
    energy.addField("freq", calcFreq(false));

    // Print what are we exactly writing
    Serial.print("Writing: ");
    Serial.println(influxClient.pointToLineProtocol(energy));
    // voltage.setTime(tnow);  //set the time
    if (!influxClient.writePoint(energy)) {
        Serial.print("InfluxDB write failed: ");
        Serial.println(influxClient.getLastErrorMessage());
    }
    delay(1000);
}
