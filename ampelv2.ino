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
typedef 

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

void handlenNormal(){
  normal_modus();
  server.send(200, "text/plain", "normalmodus aktiv");
}

void normal_modus(){

  //abfolge für normal mode setzen
  ampel_rot.next = &ampel_gelbrot;
  ampel_rot.dauer = 4000;
  ampel_gelb.next = &ampel_rot;
  ampel_gelb.dauer = 1500;
  ampel_gruen.next = &ampel_gelb;
  ampel_gruen.dauer = 4000;
  ampel_gelbrot.next = &ampel_gruen;
  ampel_gelbrot.dauer = 1500;
  jetzt = &ampel_rot;
}

void nacht_modus(){
  //abfolge und dauer für nacht modus setzen
  Zustand nacht_modus_1 = ampel_gelb;
  Zustand nacht_modus_2 = ampel_aus;
  nacht_modus_1.next = &nacht_modus_2;
  nacht_modus_1.dauer = 3000;
  nacht_modus_2.next = &nacht_modus_1;
  nacht_modus_2.dauer = 3000;
  jetzt = &nacht_modus_1;
  /*
  deprecated;code und coden lassen
  ampel_gelb.next = &ampel_aus;
  ampel_gelb.dauer = 5000;
  ampel_aus.next = &ampel_gelb;
  ampel_aus.dauer = 5000;
  jetzt = &ampel_gelb;
  */
}

void strahl_modus(){
  ampel_gruen.next = &ampel_gelb;
  ampel_gruen.dauer = 500;
  ampel_gelb.next = &ampel_rot;
  ampel_gelb.dauer = 500;
  ampel_rot.next = &ampel_gruen;
  ampel_gruen.dauer = 500;
}

void custom_modus(char* ablauf){
  //ablauf auslesen, zustände in liste schreiben, liste ablaufen lassen
  //ggbf. dauer aus Zustand nehmen und in eigenen datentyp kapseln
  //problem für zukunfts simcup
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
  server.on("/night", handleNight);

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
  nacht_modus();
}

void loop() {

  // auf neuen HTTP-Request prüfen
  server.handleClient();
  if(*jetzt != NULL){
    es_werde_licht(jetzt);
    jetzt = jetzt->next;
  }
}
