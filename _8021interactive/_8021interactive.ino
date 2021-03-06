#include <Adafruit_NeoPixel.h>
#include "ESP8266WiFi.h"
#include <WiFiUdp.h>

const char* ssid        = "DavidsiPhone";
const char* password    = "0p3nm35h";
const int LEDS          = 3;
int rssi_max            = 0;
int rssi_min            = 0;

//both these need to be set to the same size
int auto_rssi_size      = 20;
int auto_rssi[20];

int auto_rssi_padding   = 1;
int rssi_previous       = 0;
int auto_rssi_pointer   = 1;

#define PIN 4

// Parameter 1 = number of pixels in strip
// Parameter 2 = pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
Adafruit_NeoPixel strip = Adafruit_NeoPixel(LEDS, PIN, NEO_GRB + NEO_KHZ800);

WiFiUDP Udp;

void setup() {
  //setup serial port
  Serial.begin(115200);
  Serial.println("Booting...");

  //test the led strip
  Serial.println("LED Test...");
  strip.begin();
  strip.setBrightness(150);
  strip_test(1);

  //Connect to the wifi
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  Serial.print("Wifi Connecting");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

//basic led strip test
void strip_test(int number_leds) {
  for (int i = 0; i < number_leds; i++) {
    strip.setPixelColor(number_leds, 255, 0, 0);
    strip.show();
    delay(200);
    strip.setPixelColor(number_leds, 0, 255, 0);
    strip.show();
    delay(200);
    strip.setPixelColor(number_leds, 0, 0, 255);
    strip.show();
    delay(200);
    strip.setPixelColor(number_leds, 0, 0, 0);
    strip.show();
  }
}

void breath() {
  //Written by: Jason Yandell
  
  int TOTAL_LEDS = 3;
  float MinimumBrightness = 50;
  float MaximumBrightness = 200;
  float SpeedFactor = 0.032; // I don't actually know what would look good
  float StepDelay = 1; // ms for a step delay on the lights
  
  // Make the lights breathe
  for (int i = 0; i < ((2*PI)/SpeedFactor); i++) {
   
    // Intensity will go from 10 - MaximumBrightness in a "breathing" manner
    float intensity = MaximumBrightness /2.0 * (1.0 + sin(SpeedFactor * i));
    strip.setBrightness(intensity);
    
    // Now set every LED to that color
    for (int ledNumber=0; ledNumber<TOTAL_LEDS; ledNumber++) {
      strip.setPixelColor(ledNumber, 70, 70, 255);
    }
  
    strip.show();
    //Wait a bit before continuing to breathe
    delay(StepDelay);
  
  }
}

//sends a udp packet and returns the rssi
int get_rssi() {
  int rssi = 0;
  
  //sends a packet before reading the rssi
  //I think the rssi function uses a recent packet to get signal, this keeps it updating faster
  Udp.beginPacket('255.255.255.255', '1234');
  Udp.write('test');
  Udp.endPacket();
  
  //get the strenght of the current network
  rssi = WiFi.RSSI();

  return rssi;
}

void loop() {

  int rssi = 0;
  int rssi_in = 0;
  
  //get the strenght of the current network
  rssi_in = get_rssi();
  
  //only update the strip if there's a change
  if (rssi_in != rssi_previous) {

    //convert and autoscale the rssi
    rssi = convert_rssi(rssi_in); 
    
    if (rssi <= 10) {
      for (int i = 0; i < 5; i++) {
        strip.setPixelColor(1, 255, 0, 0);
        strip.setPixelColor(0, 255, 0, 0);
        strip.setPixelColor(2, 255, 0, 0);     
        strip.show(); 
        delay(50);
        strip.setPixelColor(1, 100, 0, 0);
        strip.setPixelColor(0, 100, 0, 0);
        strip.setPixelColor(2, 100, 0, 0);
        strip.show(); 
        delay(50);
        strip.setPixelColor(1, 20, 0, 0);
        strip.setPixelColor(0, 20, 0, 0);
        strip.setPixelColor(2, 20, 0, 0);
        strip.show(); 
        delay(50);
      }
    } else if (rssi >= 245) {
      for (int i = 0; i < 5; i++) {
        strip.setPixelColor(0, 200, 200, 0);
        strip.setPixelColor(2, 20, 20, 0);
        strip.show(); 
        delay(50);
        strip.setPixelColor(0, 20, 20, 0);
        strip.setPixelColor(2, 200, 200, 0);
        strip.show(); 
        delay(50);
      }
    } 

    //breath(); 
    strip.setPixelColor(0, 70, 70, 255);
    strip.setPixelColor(1, 70, 70, 255);
    strip.setPixelColor(2, 70, 70, 255);
    strip.show();
    
    rssi_previous = rssi_in;

  
    /* Testing */
    Serial.println("Min/RSSI/Max//Convert " + String(rssi_min) + "/" +  String(abs(rssi_in)) + "/" + String(rssi_max) + "//" +  String(rssi));   
  }
  
}

//Converts rssi from a neg number, runs the auto_rssi_minmax, reverses the direction, scales on 0-255 
int convert_rssi(int rssi_in) {
  int rssi_out = 0;

  rssi_in = abs(rssi_in);
  auto_rssi_minmax(rssi_in);
  
  if (rssi_min != rssi_max) {
    //map function to scale the input to 0-255
    rssi_out = map(rssi_in, rssi_min, rssi_max, 0, 255);
  }

  //make sure it doesn't scale past the bounds since we can turn scaling off
  if (rssi_in > rssi_max) {
    rssi_out = 255;
  } else if (rssi_min > rssi_in) {
    rssi_out = 0;
  }

  //reverse the range
  rssi_out = 255 - rssi_out;
  return (int) rssi_out;
    
}

//Function to find highest (maximum) value in array
int get_maximum(int input[])
{
   int len = auto_rssi_size;    // sizeof(input);  // establish size of array
   int max_value = input[1];    // start with max = first element

   for(int i = 1; i<len; i++)
   {
      if(input[i] > max_value)
            max_value = input[i];       
   }
   return max_value;                // return highest value in array
}

//Function to find lowest (minimum) value in array
int get_minimum(int input[])
{
   int len = auto_rssi_size;    //sizeof(input);  // establish size of array
   int min_value = input[1];    // start with min = first element

   for(int i = 1; i<len; i++)
   {
      if(min_value > input[i] && input[i] > 0)
            min_value = input[i];     
   }
   return min_value;    // return lowest value in array
}

//return the min and max given the current rssi
void auto_rssi_minmax(int rssi_in) {
  int rssi_diff = 0;

  //adds the inputed var to the array
  auto_rssi[auto_rssi_pointer] = rssi_in;

  //set the global min and max based off of the array
  rssi_min = get_minimum(auto_rssi);
  rssi_max = get_maximum(auto_rssi);

  //pads the min and max, gives a less sudden effects
  /*if ((rssi_max-rssi_min)<auto_rssi_padding) {
    rssi_diff = (auto_rssi_padding-(rssi_max-rssi_min))/2;
    rssi_min = rssi_min - rssi_diff;
    rssi_max = rssi_max + rssi_diff;
  }*/

  //Increment to pointer and reset once we get to top of the array
  if (auto_rssi_pointer == auto_rssi_size) { 
    auto_rssi_pointer = 1; 
  } else {
    auto_rssi_pointer++;
  }
  
}


