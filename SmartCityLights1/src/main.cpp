#include <Arduino.h>
#include <ittiot.h>
#include <time.h>
#define MODULE_TOPIC "ESP35/#"
#define WIFI_NAME "TalTech"
#define WIFI_PASSWORD ""
#define PIR_ID "pir"
// Pin definition for the PIR (GPIO14)
#define PIR_PIN D5
// Pin definition for the PIR LED (GPIO16)
#define PIR_LED_PIN D4
#define CLOCKS_PER_SEC 10
// PIR state for detection
bool pirState;
// State that switches PIR on and off
bool onState = true;
struct timeval ts; // time stamp to count latency between PIRs
// struct timeval timeset;

long timeset;

// If message received for PIR topic. For example:
// mosquitto_pub -u test -P test -t "ITT/IOT/3/pir" -m "1"
void iot_received(String topic, String msg)
{
  Serial.print("MSG FROM USER callback, topic: ");
  Serial.print(topic);
  Serial.print(" payload: ");
  Serial.println(msg);

  if (topic == "ESP35/pir")
  {
    // Switching the PIR shield on or off, depending what message is received
    if (msg == "1")
    {
      onState = true;
    }
    if (msg == "0")
    {
      onState = false;
    }
  }
  if (topic == "ESP35/RESET")
  {
    if (msg == "0")
    {
      // gettimeofday(&ts, NULL);
      //  ts.tv_sec = 0;
      //  ts.tv_usec = 0;
      timeset = millis();
    }
  }
}

// Function started after the connection to the server is established.
void iot_connected()
{
  Serial.println("MQTT connected callback");
  // Subscribe to the topic "pir"
  iot.subscribe(MODULE_TOPIC);
  // iot.subscribe(MODULE_TOPIC);
  iot.log(PIR_ID);
}

void setup()
{
  Serial.begin(115200); // setting up serial connection parameter
  Serial.println("Booting");

  iot.setConfig("wname", WIFI_NAME);
  iot.setConfig("wpass", WIFI_PASSWORD);
  iot.setConfig("msrv", "193.40.245.72");
  iot.setConfig("mport", "1883");
  iot.setConfig("muser", "test");
  iot.setConfig("mpass", "test");

  iot.printConfig(); // Print json config to serial
  iot.setup();       // Initialize IoT library

  // Initialize PIR pin
  pinMode(PIR_PIN, INPUT);
  pinMode(PIR_LED_PIN, OUTPUT);
}

void loop()
{
  iot.handle(); // IoT behind the plan work, it should be periodically called
  delay(50);    // Wait 0.2 seconds

  if (onState == true)
  {
    // This part of the code is executed, when PIR shield is active
    if (digitalRead(PIR_PIN))
    {
      if (pirState == false)
      {
        // gettimeofday(&timeset, NULL);
        //  When PIR has detected motion, then the LED is switched on and milliseconds stamp is published to the MQTT broker
        digitalWrite(PIR_LED_PIN, HIGH);

        String msg = String(millis() - timeset);
        iot.publishMsg(PIR_ID, msg.c_str());
        // Serial.println(msg);
        pirState = true;
      }
    }
    else
    {
      if (pirState == true)
      {
        // PIR shields LED is switched off, when it is not detecting any motion
        digitalWrite(PIR_LED_PIN, LOW);
        pirState = false;
      }
    }
  }
  else
  {
    // When the PIR shield has been switched off, then its offline state is sent to the MQTT broker
    iot.log("PIR offline");
    delay(100); // Waiting 0.1 seconds
  }
}