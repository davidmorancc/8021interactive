#include <Adafruit_NeoPixel.h>
#include "ESP8266WiFi.h"

const char* ssid     = "commdat";
const char* password = "0p3nm35h";
int rssi_max  = 0;
int rssi_min  = 100;

int auto_rssi_size = 50;
int auto_rssi[50];

int auto_rssi_max_cur = 0;
int auto_rssi_min_cur = 100;
int rssi_previous = 0;
int auto_rssi_pointer = 1;
int auto_rssi_count = 0;

#define PIN 0

// Parameter 1 = number of pixels in strip
// Parameter 2 = pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
Adafruit_NeoPixel strip = Adafruit_NeoPixel(2, PIN, NEO_GRB + NEO_KHZ800);

void setup() {
  Serial.begin(115200);
  strip.begin();
  strip.setBrightness(200);
  strip.show(); // Initialize all pixels to 'off'

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
}

void breath() {
  //Written by: Jason Yandell
  
  int TOTAL_LEDS = 2;
  float MinimumBrightness = 100;
  float MaximumBrightness = 200;
  float SpeedFactor = 0.007; // I don't actually know what would look good
  float StepDelay = 2; // ms for a step delay on the lights

  
  // Make the lights breathe
  for (int i = 0; i < 65535; i++) {
   
    // Intensity will go from 10 - MaximumBrightness in a "breathing" manner
    float intensity = MaximumBrightness /2.0 * (1.0 + sin(SpeedFactor * i));
    strip.setBrightness(intensity);

    // Now set every LED to that color
    for (int ledNumber=0; ledNumber<TOTAL_LEDS; ledNumber++) {
      strip.setPixelColor(ledNumber, 180, 0, 255);
    }
  
    strip.show();
    //Wait a bit before continuing to breathe
    delay(StepDelay);
  
  }
}

void loop() {

  int rssi = 0;

  //get the strenght of the current network
  rssi = convert_rssi(WiFi.RSSI());

  //only update the strip if there's a change
  if (rssi != rssi_previous) {
    //strip.setBrightness(rssi);
    strip.setPixelColor(0, 155, 0, rssi);
    strip.setPixelColor(1, 155, 0, rssi);
    strip.show(); 

    rssi_previous = rssi;
    /* Testing */
    Serial.println("-");
    Serial.println(rssi_max);
    Serial.println(rssi_min);
    Serial.println(rssi);
    Serial.println("-");
    
  }

  delay(250);

}

int convert_rssi(int rssi_in) {
  int rssi_out = 0;

  rssi_in = abs(rssi_in);
  auto_rssi_minmax(rssi_in);
  
  if (rssi_min != rssi_max) {
    //map function to scale the input to 0-255
    rssi_out = map(rssi_in, rssi_min, rssi_max, 0, 255);
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
  if ((rssi_max-rssi_min)<10) {
    rssi_diff = (10-(rssi_max-rssi_min))/2;
    rssi_min = rssi_min - rssi_diff;
    rssi_max = rssi_max + rssi_diff;
  }

  //Increment to pointer and reset once we get to top of the array
  if (auto_rssi_pointer == auto_rssi_size) { 
    auto_rssi_pointer = 1; 
  } else {
    auto_rssi_pointer++;
  }
  
  /* Testing */
  /* Serial.println("-");
  Serial.println(rssi_max);
  Serial.println(rssi_min);
  Serial.println("-");*/
}


