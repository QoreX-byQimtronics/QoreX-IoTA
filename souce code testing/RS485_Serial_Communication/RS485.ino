#define RS485_TX 15
#define RS485_RX 26

HardwareSerial RS485Serial(1);

unsigned long lastSendTime = 0;
const unsigned long sendInterval = 1000; // 1 detik

void setup() {
  RS485Serial.begin(9600, SERIAL_8N1, RS485_RX, RS485_TX);
  Serial.begin(115200);
  Serial.println("MASTER aktif: kirim 'testing rs485' tiap 1 detik");
}

void loop() {
  if (millis() - lastSendTime >= sendInterval) {
    RS485Serial.println("testing rs485");
    lastSendTime = millis();
    Serial.println("📤 Kirim: testing rs485");
  }

  // Jika ingin membaca balasan dari slave
  while (RS485Serial.available()) {
    Serial.write(RS485Serial.read());
  }
}
