#include <Wire.h>
#include <PCF8575.h>

PCF8575 pcf(0x20);

int PushCounter = 0;  // nilai awal counter
int State = 0;        // status sekarang
int lastState = 0;    // status sebelumnya

void setup() {
  Serial.begin(9600);
  Wire.begin();
  pcf.begin();
  pcf.pinMode(P9, INPUT_PULLUP);  // Ganti dari P11 ke P9
  Serial.println("Test P11 mulai...");
}

void loop() {
  State = pcf.digitalRead(P9);  // Ganti dari P11 ke P9
  Serial.println(State);

  if (State != lastState) {     // jika ada perubahan
    if (State == 0) {           // sensor tertutup
      PushCounter++;
      Serial.println(PushCounter);
    }
  }
  lastState = State;
  delay(300);
}
