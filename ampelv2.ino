#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>

#ifndef STASSID
#define STASSID "freieslabor"
#define STAPSK "stehtaufdemzettel"
#endif

const char* ssid = STASSID;
const char* passwort = STAPSK;

//pindefinitionen
const int ROTE_LAMPE = 12; 
const int GELBE_LAMPE = 13;
const int GRUENE_LAMPE = 14;

//credit where credit is due
//github.com/migu
//github.com/orithena
typedef struct Zustand Zustand;

struct Zustand {
//
  bool led_rot;
  bool led_gelb;
  bool led_gruen;

  Zustand* next;
  //millisekunden
  unsigned long dauer;
};

ESP8266WebServer server(80);

//Zustände die die ampel annehmnen kann
Zustand ampel_rot = {true, false, false, NULL, 4000};
Zustand ampel_gelb = {false, true, false, NULL, 1500};
Zustand ampel_gruen = {false, false, true, NULL, 4000};
Zustand ampel_gelbrot = {true, true, false, NULL, 1500};
Zustand ampel_aus = {false, false, false, NULL, 1000};
//zu sein nuetzlich
Zustand ampel_rotgruen = {true, false, true, NULL, 1000};
Zustand ampel_gelbgruen = {false, true, true, NULL, 1000};
Zustand ampel_rotgelbgruen = {true, true, true, NULL, 1000};

Zustand *jetzt = &ampel_gruen;

void handleRoot(){
	server.send(200, "text/plain", "lorem ipsum");
}

void handleNotFound(){
	server.send(404, "text/plain", "not found");
}

void normal_modus(){
	//zustände für normal mode initialisieren
	
  ampel_rot.next = &ampel_gelbrot;
  ampel_gelb.next = &ampel_rot;
  ampel_gruen.next = &ampel_gelb;
  ampel_gelbrot.next = &ampel_gruen;
}

void es_werde_licht(Zustand *zustand){
  //debug zwecke
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
  server.on("/", handleRoot);
  server.onNotFound(handleNotFound);
	
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
  es_werde_licht(jetzt);
  jetzt = jetzt->next;
}
