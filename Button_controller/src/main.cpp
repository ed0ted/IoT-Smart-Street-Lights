#include <Arduino.h>
#include <ittiot.h>
#include <Switch.h>
#include <time.h>
#include <list>
#define WIFI_NAME "TalTech"
#define WIFI_PASSWORD ""
const byte buttonPin = D3; // TO which pin the button has been assigned
int i;
Switch button = Switch(buttonPin);
int mode = 2; // 0 - off, 1 - on, 2 - PIR
// String pirs_topics[] = {"ESP24/pir", "ESP42/pir"}; optimization for multiple PIRs required for the whole code!
char lights_array[3][6] = {"ESP17", "ESP18", "ESP19"};
double pir1 = 0, pir2 = 0;
int intensity = 10;
void light_with_latency(int latency)
{
  // iot.publishMsg("ESP11/PIR_mode", "300");
  // Serial.println("light_with_latency: " + latency);
  String msg = String(fabs(latency));
  delay(fabs(latency));
  if (latency > 0) // if ESP24 was activated first
  {

    iot.publishMsg(lights_array[2], "10");
    for (int i = 0; i < 3; i++)
    {
      delay(latency);
      String topic = String(lights_array[i]);
      topic.concat("/PIR_mode");
      Serial.println(topic.c_str());
      iot.publishMsg(topic.c_str(), msg.c_str());
      // 3 lights, 2 PIRs - latency coefficient = 3/2
    }
  }
  if (latency < 0) // if ESP42 was activated first
  {

    iot.publishMsg(lights_array[0], "10");
    for (int i = 2; i >= 0; i--)
    {
      delay(fabs(latency));
      String topic = String(lights_array[i]);
      topic.concat("/PIR_mode");
      Serial.println(topic.c_str());
      iot.publishMsg(topic.c_str(), msg.c_str());
      // 3 lights, 2 PIRs - latency coefficient = 3/2
    }
  }
}
void mode_on()
{
  String msg = String(intensity);
  for (int i = 0; i < 3; i++)
  {
    iot.publishMsg(lights_array[i], msg.c_str());
  }
  mode = 1;
}
void mode_off()
{
  for (int i = 0; i < 3; i++)
  {
    iot.publishMsg(lights_array[i], "0");
  }
  mode = 0;
}
void mode_pir()
{
  for (int i = 0; i < 3; i++)
  {
    iot.publishMsg(lights_array[i], "10");
  }
  mode = 2;
}

void set_intensity(int intensity_)
{
  String msg = String(intensity_);
  for (int i = 0; i < 3; i++)
  {
    iot.publishMsg(lights_array[i], msg.c_str());
  }
  mode = 1;
}

// Message received
void iot_received(String topic, String msg)
{

  if (topic == "host/ESP35/intensity")
  {
    intensity=msg.toDouble();
    set_intensity(intensity);
  }

  if (topic == "host/ESP35")
  {
    if (msg.toDouble() == 0)
      mode_off();
    else if (msg.toDouble() == 1)
      mode_on();
    else if (msg.toDouble() == 2)
      mode_pir();
  }
  if (mode == 2)
  {

    if (topic == ("ESP24/pir"))
    {

      Serial.println(msg.toDouble());
      pir1 = msg.toDouble(); // Convert string to float
      Serial.println(pir1);

      if (pir2 == 0)
      {
        // light up ESP17
        iot.publishMsg(lights_array[1], "100");
      }
      else
      {
        // light up ESP11
        iot.publishMsg(lights_array[0], "100");
        iot.publishMsg(lights_array[1], "10");
      }
    }

    if (topic == ("ESP42/pir"))
    {
      Serial.println(msg.toDouble());
      pir2 = msg.toDouble(); // Convert string to double
      Serial.println(pir2);
      if (pir1 == 0)
      {
        // light up ESP17
        iot.publishMsg(lights_array[i], "100");
      }
      else
      {
        // light up ESP18
        iot.publishMsg(lights_array[3], "100");
        iot.publishMsg(lights_array[i], "10");
      }
    }

    if (pir1 != 0 && pir2 != 0)
    {
      double value;

      value = pir2 - pir1;

      if (value > 5000 || value < -5000) // timeout
      {
        pir1 = 0;
        pir2 = 0;
        mode_pir();
        return;
      }

      String msg = String((int)value);
      iot.publishMsg("latency", msg.c_str());
      Serial.println("latency" + msg);
      light_with_latency(value);
      // delay(1000);
      pir1 = 0;
      pir2 = 0;
    }
  }
}

// Function started after the connection to the server is established.
void iot_connected()
{
  // Send message to serial port to show that connection is established
  Serial.println("MQTT connected callback");
  // Subscribe to topics to get temperature and humidity messages
  iot.subscribe("ESP42/pir");
  iot.subscribe("ESP24/pir");
  iot.subscribe("host/ESP35/#");
  // Send message to MQTT server to show that connection is established
  iot.log("IoT PIR controller");
}
void setup()
{
  // Initialize serial port and send message
  Serial.begin(115200); // setting up serial connection parameter
  Serial.println("Booting");
  pinMode(buttonPin, INPUT);

  iot.setConfig("wname", WIFI_NAME);
  iot.setConfig("wpass", WIFI_PASSWORD);
  iot.setConfig("msrv", "193.40.245.72");
  iot.setConfig("mport", "1883");
  iot.setConfig("muser", "test");
  iot.setConfig("mpass", "test");

  iot.printConfig(); // print IoT json config to serial
  iot.setup();       // Initialize IoT library
}
void reset_time()
{
  iot.publishMsg("RESET", "0");
  Serial.println("Time reset!");
}
void loop()
{
  iot.handle(); // IoT behind the plan work, it should be periodically called

  button.poll();

  if (button.doubleClick())
  {
    mode_off();
    reset_time();
    pir1 = 0;
    pir2 = 0;
    // publish message to ESP35/RESET topic to reset the timers on PIR sensors
  }
  if (button.longPress())
  {
    Serial.println("Long press!");
    if (mode != 0)
    {
      set_intensity(100);
      delay(5000);
      set_intensity(intensity);
    }
  }

  delay(5);
}