
#include <Arduino.h>
#include <html.html>

#ifdef ESP32
  #include <WiFi.h>             // from "framework-arduinoespressif32" ;version": "3.20009.0"
  #include <AsyncTCP.h>         // from "framework-arduinoespressif32" ;version": "3.20009.0"
#else
  #include <ESP8266WiFi.h>      // from "framework-arduinoespressif8266" ;version": "3.20704.0"
  #include <ESPAsyncTCP.h>      // version=1.2.2   url=https://github.com/me-no-dev/ESPAsyncTCP
#endif
#include <ESPAsyncWebServer.h>  // version=1.2.4  url=https://github.com/me-no-dev/ESPAsyncWebServer
#include <DNSServer.h>          // from "framework-arduinoespressif32" ;version": "3.20009.0"
#include "ADS1X15.h"            // version=0.5.1  url=https://github.com/RobTillaart/ADS1X15

const char* PARAM_INPUT_1 = "state";
const char* PARAM_INPUT_2 = "relais";

const int Relais1 = 4;   // Licht Links 7 (5W)
const int Relais2 = 16;   // Licht rechts 5  (5W)
const int Relais3 = 17;		// Bremslicht 6   (2 x 21W)
const int Relais4 = 18;		// Blinker Links 1 (21W)
const int Relais5 = 19;	  // Blinker Rechts 4 (21W)
const int Relais6 = 21;   // Rückfahrleuchte 8 (21W)
const int Relais7 = 22;	  // Nebel Schlussleuchte 2 (21W)
const int Relais8 = 23;   // Dauer Plus 9

const int Input1 = 35;
const int Input2 = 32;
const int Input3 = 33;
const int Input4 = 25;
const int Input5 = 26;
const int Input6 = 27;
const int Input7 = 14;
const int Input8 = 13;


bool tmpBlinkerL , tmpBlinkerR , tmpBremslicht, blinkTakt, blinkTaktold; 
bool tmpRueckfahr , tmpNebel, tmpLichtR , tmpLichtL , tmpPlus;
unsigned long blinkInterval = 500, blinkms, bremsms, rueckms, nebelms;
float battlevelInt, battlevelExt;
int16_t val_0, val_1, battlevelSymbol;

void relaisOutput();
void digitalSetup();
void readADC();

class CaptivePortalHandler : public AsyncWebHandler {
public:
  CaptivePortalHandler() {}
  virtual ~CaptivePortalHandler() {}

  bool canHandle(AsyncWebServerRequest *request){
    return request->url() == "/";
  }

  void handleRequest(AsyncWebServerRequest *request) {
    if (request->method() == HTTP_GET && request->url() == "/") {
      request->send(200, "text/html", index_html);
    } else {
      request->send(200, "text/html", index_html);
    }
  }
};