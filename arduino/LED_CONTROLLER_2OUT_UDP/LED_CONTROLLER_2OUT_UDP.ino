//THIS SKETCH RECEIVES UDP DATA FOR LED STRIPS AND CONTROL IT

//========WIFI_CONFIG===========//

// INCLUDE ESP8266WiFi:
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

//CHANGE WIFI INFO HERE//
//HOME WIFI
const char *ssid = "TP-LINK_62DC";
const char *password = "67506101";

//MICROCONTROLLER WIFI SETTINGS
IPAddress ip(192, 168, 0, 135); //MICROCONTROLLER IP ADRESS
IPAddress gateway(192, 168, 0, 1);
IPAddress subnet(255, 255, 255, 0);

//UDP
WiFiUDP udp;

//IP address to send UDP data to:
// either use the ip address of the server or
// a network broadcast address
const char *udpAddress = "192.168.0.103"; //USEFULL IF NEED TO WRITE BACK TO COMPUTER
const int udpPort = 3433;                 //LISTENING PORT

//Are we currently connected?
boolean connected = false;

//===========LED_CONFIG========//

//INCLUDE FASTLED
#include <FastLED.h>

//-- CHANGE THESE -- //
#define DATA_PIN_ONE 14 //D5 on the wemos
#define DATA_PIN_TWO 13 //D7 on the wemos

#define LED_TYPE WS2811
#define COLOR_ORDER BRG
#define NUM_STRIPS 2
#define NUM_LEDS 50 //1 strip

// ============== MAX COMMUNICATIONS ===========//
#define BUFF_SIZE 64
char inputBuffer[BUFF_SIZE];

// ------------------- Gama Lookup table -------------------- //
const byte dim_curve[] = {
    0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 2, 2, 2, 2, 2, 2, 2, 2, 3, 3, 3, 3, 3, 4, 4,
    4, 4, 4, 5, 5, 5, 5, 6, 6, 6, 6, 7, 7, 7, 7, 8,
    8, 8, 9, 9, 9, 10, 10, 10, 11, 11, 12, 12, 12, 13, 13, 14,
    14, 15, 15, 15, 16, 16, 17, 17, 18, 18, 19, 19, 20, 20, 21, 22,
    22, 23, 23, 24, 25, 25, 26, 26, 27, 28, 28, 29, 30, 30, 31, 32,
    33, 33, 34, 35, 36, 36, 37, 38, 39, 40, 40, 41, 42, 43, 44, 45,
    46, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60,
    61, 62, 63, 64, 65, 67, 68, 69, 70, 71, 72, 73, 75, 76, 77, 78,
    80, 81, 82, 83, 85, 86, 87, 89, 90, 91, 93, 94, 95, 97, 98, 99,
    101, 102, 104, 105, 107, 108, 110, 111, 113, 114, 116, 117, 119, 121, 122, 124,
    125, 127, 129, 130, 132, 134, 135, 137, 139, 141, 142, 144, 146, 148, 150, 151,
    153, 155, 157, 159, 161, 163, 165, 166, 168, 170, 172, 174, 176, 178, 180, 182,
    184, 186, 189, 191, 193, 195, 197, 199, 201, 204, 206, 208, 210, 212, 215, 217,
    219, 221, 224, 226, 228, 231, 233, 235, 238, 240, 243, 245, 248, 250, 253, 255};

// -------------------------- ANIMATION ---------------------------------- //
enum State
{
    REVIVING,
    LIVING,
    DYING
};

struct Strip{
    CRGB leds[NUM_LEDS] = {};

    State currentState = LIVING;

    int livingIndex = 0;
    int revivingIndex = 0;
    int dyingIndex = 0;
};

Strip strips[2] = {};
Strip* currentStrip;

void animateLiving();
void animateReviving();
void animateDying();

void fadeToColor();

//void config();

//COLOR PALETTE;
uint8_t hue = 85;
CHSV normal    = CHSV(hue, 255, 120);
CHSV brighter  = CHSV(hue, 255, 150);
CHSV brightest = CHSV(hue, 255, 200);
CHSV dimmer1   = CHSV(hue, 255, 75);
CHSV dimmer2   = CHSV(hue, 255, 40);
//CHSV dimmer3   = CHSV(hue, 255, 40);

//======SETUP=====//

void setup()
{
    // put your setup code here, to run once:
    Serial.begin(115200);
    connectWifi(ip, gateway, subnet);

    udp.begin(udpPort); // BEGIN LISTENING ON UDP PORT udpReceivePort
    // // PRINT CONNECTION SETTINGS
    Serial.println();
    Serial.printf("Now listening at IP %s, UDP port %d\n", WiFi.localIP().toString().c_str(), udpPort);

    connected = true;

    delay(2000);
    //SETUP FASTLED
    //ADD STRIP ONE
    FastLED.addLeds<LED_TYPE, DATA_PIN_ONE, COLOR_ORDER>(strips[0].leds, NUM_LEDS);
    //ADD STRIP TWO
    FastLED.addLeds<LED_TYPE, DATA_PIN_TWO, COLOR_ORDER>(strips[1].leds, NUM_LEDS);

    //UNCOMMENT TO ENTER CONFIG MODE
    config();

    //TODO: !-- TEMPORARILY SET CURRENTSTRIP TO 0
    currentStrip = &strips[0];
}

//======MAIN_LOOP=====//

void loop()
{
    if (connected)
    {

        //If there is something in the buffer
        if (udp.parsePacket())
        {
            //Set the state on the specified strip
            //int index;
            //char command[16];
            int len = udp.read(inputBuffer, BUFF_SIZE);

            Serial.println(inputBuffer);
            Serial.println(len);
            //sscanf(inputBuffer, "%d %s", index, &command);
            //Serial.println(index);
            //Serial.println(command);


            if (strcmp(inputBuffer, "kill0") == 0){
                Serial.println(inputBuffer); //Debug
                strips[0].currentState = DYING;
            }
            else if (strcmp(inputBuffer, "revive0") == 0){
                Serial.println(inputBuffer);
                strips[0].currentState = REVIVING;
            }
            else if (strcmp(inputBuffer, "kill1") == 0){
                strips[1].currentState = DYING;
            }
            else if (strcmp(inputBuffer, "revive1") == 0){
                strips[1].currentState = REVIVING;
            }
            else {
                Serial.println("Unrecognized case from buffer");
                Serial.printf("InputBuffer: %s\n", inputBuffer);
                //Serial.printf("Command: %s\n", command);
                //Serial.printf("Index: %d\n", index);
            }

            udp.flush();
        }

        //Do Animation
        for(int s = 0; s < NUM_STRIPS; ++s){
            //Switch strip
            currentStrip = &strips[s];
            //Animate one frame on the strip
            switch (currentStrip->currentState)
            {
                case REVIVING:
                    animateReviving();
                    break;
                case DYING:
                    animateDying();
                    break;
                default:
                    animateLiving();
                    break;
            }

            // Serial.printf("Current Strip: %d\n", s);
            // Serial.printf("currentState: %d\n", currentStrip->currentState);
            // Serial.printf("livingIndex: %d\n", currentStrip->livingIndex);
            //Serial.println("dyingIndex: " + currentStrip->dyingIndex)
        }

        FastLED.show();
        //TODO: put this back to faster speed.
        delay(50);
    }
}
//======HELPERS=====//

void connectWifi(IPAddress _ip, IPAddress _gateway, IPAddress _subnet)
{
    Serial.println("***STARTING WIFI***");

    // delete old config
    WiFi.disconnect(true);
    // BEGIN WIFI
    WiFi.config(_ip, _gateway, _subnet);
    WiFi.begin(ssid, password);

    // WAIT UNTIL CONNECTED
    while (WiFi.status() != WL_CONNECTED) {
        Serial.print(".");
        delay(10);
    }
}

//======= ANIMATIONS FUNCTIONS =========
// *! Run on every update call.

// TODO: fix animations
void animateLiving(){
    //Reset index of other animations to 0
    currentStrip->revivingIndex = 0;
    currentStrip->dyingIndex = 0;

    int* livingIndex = &(currentStrip->livingIndex);
    //Do one frame
    if(*livingIndex < NUM_LEDS + 4)
    {

        if(*livingIndex < NUM_LEDS){
            currentStrip->leds[*livingIndex] = normal;
        }

        //Set leds behind it if you can 
        if(*livingIndex >= 1 && (*livingIndex - 1 < NUM_LEDS)){
            currentStrip->leds[*livingIndex - 1] = brighter;
        }
        if (*livingIndex >= 2 && (*livingIndex - 2 < NUM_LEDS)){
            currentStrip->leds[*livingIndex - 2] = brightest;
        }
        if (*livingIndex >= 3 && (*livingIndex - 3 < NUM_LEDS)){
            currentStrip->leds[*livingIndex - 3] = brighter;
        }
        if (*livingIndex >= 4 && (*livingIndex - 4 < NUM_LEDS)){
            currentStrip->leds[*livingIndex - 4] = normal;
        }

        (*livingIndex)++;
    }
    else
    {
        //After the wave, reset the the livingIndex to 0
        *livingIndex = 0;
    }
}

void animateDying(){
    currentStrip->livingIndex = 0;
    currentStrip->revivingIndex = 0;

    int* dyingIndex = &(currentStrip->dyingIndex);

    //TODO: Test the function fadelightby
    if(*dyingIndex < NUM_LEDS + 3)
    {
        //Set current to bright green
        if(*dyingIndex < NUM_LEDS){
            currentStrip->leds[*dyingIndex] = brighter;
        }

        //Set led behind it
        if(*dyingIndex >= 1 && (*dyingIndex - 1 < NUM_LEDS)){
            currentStrip->leds[*dyingIndex - 1] = dimmer1;
        }
        if(*dyingIndex >= 2 && (*dyingIndex - 2 < NUM_LEDS)){
            currentStrip->leds[*dyingIndex - 2] = dimmer2;
        }
        if(*dyingIndex >= 3 && (*dyingIndex - 3 < NUM_LEDS)){
            currentStrip->leds[*dyingIndex - 3] = CRGB(0, 0, 0);
        }

        (*dyingIndex)++;
    }
}

void animateReviving(){
    currentStrip->livingIndex = 0;
    currentStrip->dyingIndex = 0;
    
    int* revivingIndex = &(currentStrip->revivingIndex);
    if(++(*revivingIndex) <= 24)
    {

        //Light up all leds
        for(int i = 0; i < NUM_LEDS; ++i){
            currentStrip->leds[i] = CHSV(normal.h, normal.s, *revivingIndex * 6);
        }
    }
    //Reviving completed
    else 
    {
        currentStrip->currentState = LIVING;
    }
}

//TODO: Finish this
void fadeToColor(){
    CHSV color = CHSV(200, 255, 200);
}

void config(){

    FastLED.clear();
    
    for(int s = 0; s < NUM_STRIPS; ++s){
        strips[s].leds[0] = CRGB(255, 0, 0);
        strips[s].leds[1] = CRGB(255, 0, 0);
        strips[s].leds[2] = CRGB(0, 255, 0);
        strips[s].leds[3] = CRGB(0, 255, 0);
        strips[s].leds[4] = CRGB(0, 0, 255);
        strips[s].leds[5] = CRGB(0, 0, 255);
        strips[s].leds[7] = CRGB(0, 0, 0);
    }


    for(int s = 0; s < NUM_STRIPS; ++s){
        Serial.printf("%d %d %d\n", strips[s].livingIndex, strips[s].dyingIndex, strips[s].revivingIndex);
    }

    FastLED.show();
    
    Serial.println("Press any key to continute");
    while(!Serial.available()) {};
    Serial.println(Serial.read());

}
