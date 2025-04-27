#include <main.h>
// 8 Relais und 8 Input
// Schalten geht
// Rückmeldung aller Slider und Inputs geht
// Anordnung in Tabelle
// Captive Portal
// Alles von false auf true ändern (html und cpp)
// In / Out Pins neu geordnet
// Anzeige der Batteriespannung
// ADC ADS1115 eingefügt und zweite Batteriespannung
// Batteriesymbol
// Eingänge neu geordnet (Nach Hardware)

// ToDo: 

DNSServer dnsServer;
AsyncWebServer server(80); // AsyncWebServer objekt instanz port 80
ADS1115 ADS(0x48);  // Objekt Instanz

void setup(){
  Serial.begin(115200);
  digitalSetup();
  Wire.setPins(5,15);
  Wire.begin();
  ADS.begin();
  ADS.setGain(0);      //  6.144 volt
  ADS.readADC(0);      //  first read to trigger
  ADS.readADC(1);      //  

  boolean result = WiFi.softAP("Trailertest");      // Start AP

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){    // Route für root / web page
    request->send_P(200, "text/html", index_html);
  });

  // GET request vom client (browser) bei Bedienung. Format: /update? relais=element.id &state= 0/1
  server.on("/update", HTTP_GET, [] (AsyncWebServerRequest *request) {
    if (request->hasParam(PARAM_INPUT_1) && request->hasParam(PARAM_INPUT_2)) {   //Komplette Anfrage?
      String state = request->getParam(PARAM_INPUT_1)->value();
      String relais = request->getParam(PARAM_INPUT_2)->value();
      if(relais == "relais1"){
       tmpLichtL = state.toInt();
      }else if (relais == "relais2"){
        tmpLichtR = state.toInt();
      }else if (relais == "relais3"){
        tmpBremslicht = state.toInt();
      }else if (relais == "relais4"){
        tmpBlinkerL = state.toInt();
      }else if (relais == "relais5"){
        tmpBlinkerR = state.toInt();
      }else if (relais == "relais6"){
        tmpRueckfahr = state.toInt(); 
      }else if (relais == "relais7"){
        tmpNebel = state.toInt(); 
      }else if (relais == "relais8"){
        tmpPlus = state.toInt(); 
      }
      //Serial.println(color+"="+state);
    }
    request->send(200, "text/plain", "OK");
  });

  // Zyklischer GET request vom client für status. Komma getrennt
  server.on("/state", HTTP_GET, [] (AsyncWebServerRequest *request) {
    String respond;
    respond = (tmpLichtL)? "true":"false"; 
    respond += ","; respond += (tmpLichtR)? "true":"false";             // Rückmeldung für Slider
    respond += ","; respond += (tmpBremslicht)? "true":"false";         // 1:0
    respond += ","; respond += (tmpBlinkerL)? "true":"false";
    respond += ","; respond += (tmpBlinkerR)? "true":"false"; 
    respond += ","; respond += (tmpRueckfahr)? "true":"false";
    respond += ","; respond += (tmpNebel)? "true":"false";
    respond += ","; respond += (tmpPlus)? "true":"false";
    respond += ","; respond += (digitalRead(Input1))? "true":"false";   // Rückmeldung für Checkbox Ausgänge
    respond += ","; respond += (digitalRead(Input2))? "true":"false";   // 1:0  0 = Eingang aktiv
    respond += ","; respond += (digitalRead(Input3))? "true":"false";
    respond += ","; respond += (digitalRead(Input4))? "true":"false";
    respond += ","; respond += (digitalRead(Input5))? "true":"false";
    respond += ","; respond += (digitalRead(Input6))? "true":"false";
    respond += ","; respond += (digitalRead(Input7))? "true":"false";
    respond += ","; respond += (digitalRead(Input8))? "true":"false";
    respond += ","; respond += battlevelInt;                             // Wert für Batteriespannung Intern
    respond += ","; respond += battlevelExt;                             // Wert für Batteriespannung Auto
    respond += ","; respond += battlevelSymbol;
    //Serial.println(respond);
    request->send(200, "text/plain", respond);
  });
    server.addHandler(new CaptivePortalHandler()).setFilter(ON_AP_FILTER);
  server.onNotFound([&](AsyncWebServerRequest *request){
    request->send(200, "text/html", index_html); 
  });

  dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
  dnsServer.setTTL(300);
  dnsServer.start(53, "*", WiFi.softAPIP());
  server.begin();
}

void loop() {
  dnsServer.processNextRequest();
  relaisOutput();

  if (blinkTakt != blinkTaktold){
    blinkTaktold = blinkTakt;   
    readADC();
    battlevelInt = (float)val_0/1677;
    battlevelExt = (float)val_1/1395;
    if (battlevelInt >= 12){
      battlevelSymbol = 3;
    }else if (battlevelInt < 12 && battlevelInt >= 11.2){
      battlevelSymbol = 2;
    }}else if (battlevelInt < 11.2 && battlevelInt >= 10.5){
      battlevelSymbol = 1;
    }else if (battlevelInt < 10.5 && battlevelInt >= 9.8){
      battlevelSymbol = 0;
    }else if (battlevelInt < 9.8){
      battlevelSymbol = -1;
    }
    //Serial.println(battlevelSymbol);
} 

void readADC(){
    if (ADS.isBusy() == false){
      val_0 = ADS.readADC(0);
      val_1 = ADS.readADC(1);
      //  request a new one
      ADS.requestADC(0);
      ADS.requestADC(1);
  }
}

void relaisOutput(){
  unsigned long actms = millis();
  if (actms - blinkms >= blinkInterval){
    blinkms = actms;
    blinkTakt = !blinkTakt;
  }
  if (battlevelInt < 9.8){      // Alle Ausgänge aus
    tmpLichtL = false;
    tmpLichtR = false;
    tmpBremslicht = false;
    tmpBlinkerL = false;
    tmpBlinkerR = false;
    tmpRueckfahr = false;
    tmpNebel = false;
    tmpPlus = false;
  }
  if (tmpBlinkerL){                         // Blinker aktiv 
    digitalWrite(Relais4, blinkTakt);
  }else{
    digitalWrite(Relais4, 1);               // Relais aus (sink)
  }
  if (tmpBlinkerR){
    digitalWrite(Relais5, blinkTakt);
  }else{
    digitalWrite(Relais5, 1);
  }
  if (!tmpBremslicht){                      // Nicht eingeschaltet
    bremsms = actms;
    digitalWrite(Relais3, 1);
  }else{
    if (actms - bremsms < 3000){            // Ausschalten nach 3 Sekunden
      digitalWrite(Relais3, 0);
    }else{
      digitalWrite(Relais3, 1);
      tmpBremslicht = false;
    }
  }
  if (tmpBremslicht){                       // Eingeschaltet -> keine anderen 21W erlaubt
    tmpRueckfahr = false;
    tmpNebel = false;
  }
  if (!tmpRueckfahr){                       // Nicht eingeschaltet
    rueckms = actms;
    digitalWrite(Relais6, 1);
  }else{
    if (actms - rueckms < 3000){            // Ausschalten nach 3 Sekunden
      digitalWrite(Relais6, 0);
    }else{
      digitalWrite(Relais6, 1);
      tmpRueckfahr = false;
    }
  }
  if (!tmpNebel){                           // Nicht eingeschaltet
    nebelms = actms;
    digitalWrite(Relais7, 1);
  }else{
    if (actms - nebelms < 3000){            // Ausschalten nach 3 Sekunden
      digitalWrite(Relais7, 0);
    }else{
      digitalWrite(Relais7, 1);
      tmpNebel = false;
    }
  }
  digitalWrite(Relais1, !tmpLichtL);
  digitalWrite(Relais2, !tmpLichtR);
  digitalWrite(Relais8, !tmpPlus);
}



void digitalSetup(){
  pinMode(Relais1, OUTPUT);
  pinMode(Relais2, OUTPUT);
  pinMode(Relais3, OUTPUT);
  pinMode(Relais4, OUTPUT);
  pinMode(Relais5, OUTPUT);
  pinMode(Relais6, OUTPUT);
  pinMode(Relais7, OUTPUT);
  pinMode(Relais8, OUTPUT);
  digitalWrite(Relais1, HIGH);
  digitalWrite(Relais2, HIGH);
  digitalWrite(Relais3, HIGH);
  digitalWrite(Relais4, HIGH);
  digitalWrite(Relais5, HIGH);
  digitalWrite(Relais6, HIGH);
  digitalWrite(Relais7, HIGH);
  digitalWrite(Relais8, HIGH);
  pinMode(Input1, INPUT_PULLDOWN);
  pinMode(Input2, INPUT_PULLDOWN);
  pinMode(Input3, INPUT_PULLDOWN);
  pinMode(Input4, INPUT_PULLDOWN);
  pinMode(Input5, INPUT_PULLDOWN);
  pinMode(Input6, INPUT_PULLDOWN);
  pinMode(Input7, INPUT_PULLDOWN);
  pinMode(Input8, INPUT_PULLDOWN);
}