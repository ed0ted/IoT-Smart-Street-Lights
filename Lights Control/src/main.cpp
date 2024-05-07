// Includes global variables and librarys that the RGB LED uses
#include <Arduino.h>
#include <ittiot.h>
#include <Adafruit_NeoPixel.h>

#define WIFI_NAME "TalTech"
#define WIFI_PASSWORD ""

// Change it according to the real name of the ESP microcontroler IoT module where the encoder is connected
#define ENC_TOPIC "ESP35/"

// change accordingly to the uploading light module
#define MODULE_ID "ESP19"
// RGB LED pin conficuration
#define PIN D2
#define DEFAULT_TRANSITION_MS 500
// Create an object for RGB LED
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(1, PIN, NEO_GRB + NEO_KHZ800);

// Variable to store led brightness
int mode;
int current_intensity = 10;
int ledMill = 0;

void change_light(int final_intensity, double milliseconds)
{

  if (current_intensity < final_intensity)
  {
    double delay_ms = milliseconds / (final_intensity - current_intensity);
    String msg = String((int)delay_ms);
    iot.publishMsg("delay_ms", msg.c_str());
    for (;
         current_intensity < final_intensity;
         current_intensity++)
    {

      // Set all led at same brightness
      pixels.setPixelColor(0, current_intensity, current_intensity, current_intensity);
      // This sends the updated pixel color to the hardware.
      pixels.show();
      delay(delay_ms);
    }
  }

  else if (final_intensity < current_intensity)
  {
    double delay_ms = milliseconds / (current_intensity - final_intensity);
    String msg = String((int)delay_ms);
    iot.publishMsg("delay_ms", msg.c_str());
    for (;
         current_intensity > final_intensity;
         current_intensity--)
    {
      // Set all led at same brightness
      pixels.setPixelColor(0, current_intensity, current_intensity, current_intensity);
      // This sends the updated pixel color to the hardware.
      pixels.show();
      delay(delay_ms);
    }
  }
  current_intensity = final_intensity;
}

// Message received from encoder
void iot_received(String topic, String msg)
{
  if (topic == ENC_TOPIC MODULE_ID)
  {
    Serial.println("Aim intensity: " + msg);
    change_light(msg.toInt(), DEFAULT_TRANSITION_MS);
    // delay(DEFAULT_TRANSITION_MS);
    // change_light(10, DEFAULT_TRANSITION_MS);
  }
  if (topic == ENC_TOPIC MODULE_ID "/PIR_mode")
  {
    Serial.println("Aim intensity: 90");
    change_light(90, msg.toInt()/2);
    delay(msg.toInt());
    //delay(1000);
    change_light(10, msg.toInt()/2);
  }
}

// Function started after the connection to the server is established.
void iot_connected()
{
  // Send message to serial port to show that connection is established
  Serial.println("MQTT connected callback");
  // Subscribe to get enc message
  iot.subscribe(ENC_TOPIC MODULE_ID);
  iot.subscribe(ENC_TOPIC MODULE_ID "/PIR_mode");
  // Send message to MQTT server to show that connection is established
  iot.log("IoT Light");
}

void setup()
{
  // Initialize serial port and send message
  Serial.begin(115200); // setting up serial connection parameter
  Serial.println("Booting");

  iot.setConfig("wname", WIFI_NAME);
  iot.setConfig("wpass", WIFI_PASSWORD);
  iot.setConfig("msrv", "193.40.245.72");
  iot.setConfig("mport", "1883");
  iot.setConfig("muser", "test");
  iot.setConfig("mpass", "test");
  iot.printConfig(); // print IoT json config to serial
  iot.setup();       // Initialize IoT library

  pixels.begin(); // Initialize RGB LED

  // Turn all LED-s off
  pixels.setPixelColor(0, 0, 0, 0);
  pixels.show();
}

void loop()
{
  // if(current_intensity == 90 && millis()>=ledMill+10000)
  // {
  //  change_light(10, DEFAULT_TRANSITION_MS);
  // }
  iot.handle(); // IoT behind the plan work, it should be periodically called
  delay(10);    // Wait for 0.01 second
}