//THIS SKETCH RECEIVES UDP DATA FOR LED STRIPS AND CONTROL IT


//========WIFI_CONFIG===========//

// INCLUDE ESP8266WiFi:
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

//CHANGE WIFI INFO HERE//
//HOME WIFI
const char * ssid = "TP-LINK_62DC";
const char * password = "67506101";


//MICROCONTROLLER WIFI SETTINGS
IPAddress ip(192, 168, 0, 125); //MICROCONTROLLER IP ADRESS
IPAddress gateway(192, 168, 0, 1);
IPAddress subnet(255, 255, 255, 0);

//UDP
WiFiUDP udp;

//IP address to send UDP data to:
// either use the ip address of the server or
// a network broadcast address
const char * udpAddress = "192.168.0.103"; //USEFULL IF NEED TO WRITE BACK TO COMPUTER
const int udpPort = 3333; //LISTENING PORT


//===========LED_CONFIG========//

//INCLUDE FASTLED
#include <FastLED.h>


//-- CHANGE THESE -- //
#define DATA_PIN_ONE 14 //D5 on the wemos
//#define DATA_PIN_TWO 13 //D7 on the wemos

#define LED_TYPE WS2811
#define COLOR_ORDER GRB
#define NUM_LEDS 100 //1 strip

//LED ARRAY
CRGB leds[NUM_LEDS];
const int dataChunkSize = 3; // PACKETS OF RGB DATA
const int numOfBytes = NUM_LEDS * dataChunkSize;
int byteReturnLen = 0;
int byteReturnCounter = 0;
char inputBuffer[numOfBytes];

int maxBright = 200;

int led = 0;
int led_r = 0;
int led_g = 0;
int led_b = 0;


//Are we currently connected?
boolean connected = false;

// ------------------- Gama Lookup table -------------------- //
const byte dim_curve[] = {
  0,   0,   0,   0,   0,   0,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
  1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
  1,   2,   2,   2,   2,   2,   2,   2,   2,   3,   3,   3,   3,   3,   4,   4,
  4,   4,   4,   5,   5,   5,   5,   6,   6,   6,   6,   7,   7,   7,   7,   8,
  8,   8,   9,   9,   9,  10,  10,  10,  11,  11,  12,  12,  12,  13,  13,  14,
  14,  15,  15,  15,  16,  16,  17,  17,  18,  18,  19,  19,  20,  20,  21,  22,
  22,  23,  23,  24,  25,  25,  26,  26,  27,  28,  28,  29,  30,  30,  31,  32,
  33,  33,  34,  35,  36,  36,  37,  38,  39,  40,  40,  41,  42,  43,  44,  45,
  46,  46,  47,  48,  49,  50,  51,  52,  53,  54,  55,  56,  57,  58,  59,  60,
  61,  62,  63,  64,  65,  67,  68,  69,  70,  71,  72,  73,  75,  76,  77,  78,
  80,  81,  82,  83,  85,  86,  87,  89,  90,  91,  93,  94,  95,  97,  98,  99,
  101, 102, 104, 105, 107, 108, 110, 111, 113, 114, 116, 117, 119, 121, 122, 124,
  125, 127, 129, 130, 132, 134, 135, 137, 139, 141, 142, 144, 146, 148, 150, 151,
  153, 155, 157, 159, 161, 163, 165, 166, 168, 170, 172, 174, 176, 178, 180, 182,
  184, 186, 189, 191, 193, 195, 197, 199, 201, 204, 206, 208, 210, 212, 215, 217,
  219, 221, 224, 226, 228, 231, 233, 235, 238, 240, 243, 245, 248, 250, 253, 255
};


//======SETUP=====//

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  connectWifi(ip, gateway, subnet);
  
  udp.begin(udpPort); // BEGIN LISTENING ON UDP PORT udpReceivePort
  // // PRINT CONNECTION SETTINGS
  Serial.println();
  Serial.printf("Now listening at IP %s, UDP port %d\n", WiFi.localIP().toString().c_str(), udpPort);

  connected = true;

  //SETUP FASTLED
  //ADD STRIP ONE
  FastLED.addLeds<LED_TYPE, DATA_PIN_ONE, COLOR_ORDER>(leds, NUM_LEDS);
  //ADD STRIP TWO
  FastLED.addLeds<LED_TYPE, DATA_PIN_TWO, COLOR_ORDER>(leds, NUM_LEDS);

}


//======MAIN_LOOP=====//

void loop() {

//THIS PART OF CODE IS TAKEN FROM 
  if (connected) {
    
    //processing incoming packet, must be called before reading the buffer
    udp.parsePacket();

    if (udp.read(inputBuffer, numOfBytes) > 0) {
      
      for (int j = 0; j < NUM_LEDS; j++) { // for loop through our total led's set from above.
        led_r = constrain(dim_curve[inputBuffer[(j * dataChunkSize)]], 0, maxBright); // set each r,g, and b value through the gamma table and using offsets and multiple of three to traverse the array.
        led_g = constrain(dim_curve[inputBuffer[(j * dataChunkSize) + 1]], 0, maxBright);
        led_b = constrain(dim_curve[inputBuffer[(j * dataChunkSize) + 2]], 0, maxBright);

        inputBuffer[(j * dataChunkSize)] = 0;
        inputBuffer[(j * dataChunkSize) + 1] = 0;
        inputBuffer[(j * dataChunkSize) + 2] = 0;

        leds[j].setRGB( led_r, led_g, led_b);
      }
    }
    FastLED.show();
    udp.flush(); //EMPTY BUFFER
  }
}

//======HELPERS=====//

void connectWifi(IPAddress _ip , IPAddress _gateway, IPAddress _subnet)
{
  Serial.println("***STARTING WIFI***");

  // delete old config
  WiFi.disconnect(true);
  // BEGIN WIFI
  WiFi.config(_ip , _gateway , _subnet );
  WiFi.begin(ssid, password);


  // WAIT UNTIL CONNECTED
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(10);
  }
}
