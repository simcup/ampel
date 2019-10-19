

const int ROTE_LAMPE = 12;
const int GELBE_LAMPE = 13;
const int GRUENE_LAMPE = 14;

typedef struct Zustand Zustand;

struct Zustand {
  bool led_rot;
  bool led_gelb;
  bool led_gruen;

  Zustand* next;
  //milisekunden
  unsigned long dauer;
};

Zustand ampel_rot = {true, false, false, NULL, 4000};
Zustand ampel_gelb = {false, true, false, NULL, 1500};
Zustand ampel_gruen = {false, false, true, NULL, 4000};
Zustand ampel_gelbrot = {true, true, false, NULL, 1500};
Zustand *ampel = &ampel_gruen;

void setup() {
  // Serial output mit baud rate 115200 initialisieren
  Serial.begin(115200);
  
  //pins initialisieren
  pinMode(ROTE_LAMPE, OUTPUT);
  pinMode(GELBE_LAMPE, OUTPUT);
  pinMode(GRUENE_LAMPE, OUTPUT);
  
  //zustände zuweisen 
  ampel_rot.next = &ampel_gelbrot;
  ampel_gelb.next = &ampel_rot;
  ampel_gruen.next = &ampel_gelb;
  ampel_gelbrot.next = &ampel_gruen;
}

void es_werde_licht(Zustand *zustand){
  Serial.printf("rot: %d, gelb: %d, gruen: %d\n", zustand->led_rot, zustand->led_gelb, zustand->led_gruen); 
  digitalWrite(ROTE_LAMPE, zustand->led_rot);
  digitalWrite(GELBE_LAMPE, zustand->led_gelb);
  digitalWrite(GRUENE_LAMPE, zustand->led_gruen);
  delay(zustand->dauer);
}

void loop() {
  es_werde_licht(ampel);
  ampel = ampel->next;
  
}