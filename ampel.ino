#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
//#include <SPIFFS.h>
#include "psk.h"
#include "ArduinoJson-v6.13.0.h"

#ifndef STASSID
#define STASSID "freieslabor"
#define STAPSK "stehtaufdemzettel"
#endif

//bool astop = true;
//bool astart = false;
const char* ssid = STASSID;
const char* passwort = STAPSK;

//pindefinitionen
const int ROTE_LAMPE = 12;
const int GELBE_LAMPE = 13;
const int GRUENE_LAMPE = 14;

//funktionsdeklaration
void nacht_modus();
void normal_modus();

//credit where credit is due
//github.com/migu
//github.com/orithena
typedef struct Zustand Zustand;

struct Zustand {
  bool led_rot;
  bool led_gelb;
  bool led_gruen;

  Zustand* next;
  //millisekunden
  unsigned long dauer;
};

ESP8266WebServer server(80);

//Zustände die die ampel annehmnen kann, 3bit (rot,gelb,gruen) = 8 zustände
Zustand ampel_aus = {false, false, false, NULL, 1000};
Zustand ampel_rot = {true, false, false, NULL, 1000};
Zustand ampel_gelb = {false, true, false, NULL, 1000};
Zustand ampel_gruen = {false, false, true, NULL, 1000};
Zustand ampel_gelbrot = {true, true, false, NULL, 1000};
Zustand ampel_rotgruen = {true, false, true, NULL, 1000};
Zustand ampel_gelbgruen = {false, true, true, NULL, 1000};
Zustand ampel_rotgelbgruen = {true, true, true, NULL, 1000};

Zustand *jetzt = NULL;

void handleRoot(){
  server.send(200, "text/plain", "wenn sie dies lesen hat noch niemand ein interface geschrieben");
}

void handleNotFound(){
  server.send(404, "text/plain", "not found");
}

void handleNacht(){
  nacht_modus();
  server.send(200, "text/plain", "nachtmodus aktiv");
}

void handleNormal(){
  normal_modus();
  server.send(200, "text/plain", "normalmodus aktiv");
}

void handleStop(){
  es_werde_licht(&ampel_aus);
  jetzt = NULL;
  server.send(200, "text/plain", "stopped");
  
}
void handleCustomPOST(){
  //not yet tested, ampel ausschalten, gucken ob genug parameter da sind, zum debuggen parameter ausgeben, pins dahin ziehen wo sie hinsollen
  
  
  if(server.hasArg("red") && server.hasArg("yellow") && server.hasArg("green")){
    
    //debug
    Serial.print("anfrage eingegangen,");
    Serial.print(" rot: "+server.arg("red"));
    Serial.print(" gelb: "+server.arg("yellow"));
    Serial.print(" gruen: "+server.arg("green"));

    jetzt = NULL;
    es_werde_licht(&ampel_aus);
    
    digitalWrite(ROTE_LAMPE, !server.arg("red").toInt());
    digitalWrite(GELBE_LAMPE, !server.arg("yellow").toInt());
    digitalWrite(GRUENE_LAMPE, !server.arg("green").toInt());
    server.send(200, "text/plain", "ok");
  }else{
    server.send(400, "text/plain", "no or not enough parameters");
  }
}
void handleCustomGET(){
  //hier könnte ihre implementierung eines javascript ajax interface für requests an "/custom" stehen
  server.send(200, "text/html", "<html><body><form method=\"POST\"><p>rot:an<input type=\"radio\" name=\"red\" value=\"1\"> aus<input type=\"radio\" name=\"red\" value=\"0\"></p><p>gelb:an<input type=\"radio\" name=\"yellow\" value=\"1\"> aus<input type=\"radio\" name=\"yellow\" value=\"0\"></p><p>gruen:an<input type=\"radio\" name=\"green\" value=\"1\"> aus<input type=\"radio\" name=\"green\" value=\"0\"></p><input type=\"submit\" value=\"abschicken\"></form></body></html>");
  //text/plain response: you have reached the custom mode, your request is importend to us, please hold the socket...
}
/*
Zustand * kreire_abfolge(String abfolge_str){/*char, char[], String?*/
  /*WIP: 
  nimm einen json string ( [{"RYG":(int)dauer},... ) entgegen,
    wenn nicht valide gib fehler zurueck,
  kreiere ein array von zuständen len=länge der liste in json,
  itteriere ueber die elemente,
  setzte pins abhängig von string ("Zustand->led_rot = 'R' in elem[i].name") setzte dauer auf wert,
  wenn element nicht letztes in der liste,
    Zustand.next auf naechsten eintrag im array setzten,
  sonst auf erstes zeigen lassen, array zurückgeben//
  StaticJsonDocument<512> abfolge_json;
  deserializeJson(abfolge_json, abfolge_str);
  int count = (int)abfolge_json.size();
   Zustand abfolge[count];
  for(int i = 0; i < count-1;i++){
   abfolge[i].led_rot = abfolge_json[i]["rot"];// = "R" in abfolge_json[i].key()
   abfolge[i].led_gelb = abfolge_json[i]["gelb"];// = "Y" in abfolge_json[i].key()
   abfolge[i].led_gruen = abfolge_json[i]["gruen"];// = "G" in abfolge_json[i].key()
   abfolge[i].dauer;// = abfolge_json[i].value()
   abfolge[i].next = &abfolge[i+1];
    
  }
  //abfolge[]
  abfolge[count-1].led_rot;// = "R" in abfolge_json[i].key()
  abfolge[count-1].led_gelb;// = "Y" in abfolge_json[i].key()
  abfolge[count-1].led_gruen;// = "G" in abfolge_json[i].key()
  abfolge[count-1].dauer;// = abfolge_json[i].value()
  abfolge[count-1].next;// = &abfolge[i+1];
  return &abfolge[0];
}
//*/

void normal_modus(){
  Serial.println("normal_modus aufgerufen");
  //abfolge für normal mode setzen
  static Zustand normal1 = ampel_gruen;
  static Zustand normal2 = ampel_gelb;
  static Zustand normal3 = ampel_rot;
  static Zustand normal4 = ampel_gelbrot;
  normal1.next = &normal2;
  normal1.dauer = 10000;
  normal2.next = &normal3;
  normal2.dauer = 2500;
  normal3.next = &normal4;
  normal3.dauer = 10000;
  normal4.next = &normal1;
  normal4.dauer = 1500;
  jetzt = &normal1;

}

void nacht_modus(){
  Serial.println("nacht_modus aufgerufen");
  //abfolge und dauer für nacht modus setzen
  static Zustand nacht1 = ampel_gelb;
  static Zustand nacht2 = ampel_aus;
  nacht1.next = &nacht2;
  nacht1.dauer = 1000;
  nacht2.next = &nacht1;
  nacht2.dauer = 1000;
  jetzt = &nacht1;
}



 void es_werde_licht(Zustand *zustand){
  //debug output
  Serial.printf("rot: %d, gelb: %d, gruen: %d\n",
  zustand->led_rot,
  zustand->led_gelb,
  zustand->led_gruen
  );

  //die pins dahin ziehen wo sie hinsollen
  digitalWrite(ROTE_LAMPE, !zustand->led_rot);
  digitalWrite(GELBE_LAMPE, !zustand->led_gelb);
  digitalWrite(GRUENE_LAMPE, !zustand->led_gruen);

  //warten bis zur naechsten zustandsaenderung
  pause(zustand->dauer);
}

void pause(unsigned long ms) {
  //Absolut nicht geklaut
  unsigned long wiederbringe_an = millis() + ms;
  while(millis() < wiederbringe_an) {
    yield();
  }
}

void setup() {
  // Serial output mit baud rate 115200 initialisieren
  Serial.begin(115200);
  //wlan verbindung aufbauen
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, passwort);
  Serial.println("");

  //warten bis verbindung erstellt wurde
  while(WiFi.status() != WL_CONNECTED){
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("conneted to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  //funktionen an requests binden
  server.onNotFound(handleNotFound);
  server.on("/", handleRoot);
  server.on("/normal", handleNormal);
  server.on("/night", handleNacht);
  server.on("/stop", handleStop);
  server.on("/custom", HTTP_GET, handleCustomGET);
  server.on("/custom", HTTP_POST, handleCustomPOST);

  // starte Webserver (bind TCP)
  server.begin();
  Serial.println("Web server started");

  //pins initialisieren
  pinMode(ROTE_LAMPE, OUTPUT);
  pinMode(GELBE_LAMPE, OUTPUT);
  pinMode(GRUENE_LAMPE, OUTPUT);

  //relay ist aktive low, deswegen pins high ziehen
  digitalWrite(ROTE_LAMPE, true);
  digitalWrite(GELBE_LAMPE, true);
  digitalWrite(GRUENE_LAMPE, true);

  //zustände zuweisen
  normal_modus();
}

void loop() {
  // auf neuen HTTP-Request prüfen
  server.handleClient();
  if(jetzt != NULL){

    
    es_werde_licht(jetzt);
    jetzt = jetzt->next;
  }else{
    delay(500);
   Serial.println("nichts passiert");
  }
}
