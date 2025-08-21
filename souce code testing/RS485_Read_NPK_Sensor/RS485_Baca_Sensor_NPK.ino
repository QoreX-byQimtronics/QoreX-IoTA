#include <SoftwareSerial.h>

// Perintah request data ke sensor (Modbus RTU: ID, Function, Address, Data Count, CRC)
unsigned char requestData[8] = {0X01, 0X03, 0X00, 0X00, 0X00, 0X07, 0X04, 0X08};

// Array untuk menyimpan balasan data dari sensor
unsigned char receivedData[19] = {};

// Konstanta parameter Modbus
#define FRAME_SIZE        19      // Jumlah byte balasan dari sensor
#define RESPONSE_TIMEOUT  1000    // Waktu tunggu balasan sensor (ms)
#define DEVICE_ID         0x01    // ID perangkat sensor
#define FUNC_CODE         0x03    // Kode fungsi Modbus (Read Holding Registers)
#define BYTE_COUNT        0x0E    // Jumlah byte data pada balasan

// Objek SoftwareSerial untuk komunikasi RS485
SoftwareSerial sensorSerial(26, 15); // RX = GPIO26, TX = GPIO15

// Variabel untuk menyimpan hasil pembacaan
float soilMoisture;     // Kelembaban tanah (%)
float soilTemp;         // Suhu tanah (°C)
float soilPH;           // pH tanah
float nitrogen;         // Nitrogen (mg/kg)
float phosphorus;       // Fosforus (mg/kg)
float potassium;        // Kalium (mg/kg)

void setup() {
  Serial.begin(9600);          // Serial monitor untuk debugging
  sensorSerial.begin(4800);    // Komunikasi RS485 ke sensor
}

void loop() {
  // Kirim request ke sensor
  sensorSerial.write(requestData, 8);

  // Tunggu data masuk atau timeout
  unsigned long startTime = millis();
  while ((sensorSerial.available() < FRAME_SIZE) && ((millis() - startTime) < RESPONSE_TIMEOUT)) {
    delay(1);
  }
  
  // Baca semua byte dari sensor
  while (sensorSerial.available()) {
    for (int i = 0; i < FRAME_SIZE; i++) {
      receivedData[i] = sensorSerial.read();
    }

    // Cek apakah data valid berdasarkan ID, fungsi, dan jumlah byte
    if (receivedData[0] != DEVICE_ID && receivedData[1] != FUNC_CODE && receivedData[2] != BYTE_COUNT) {
      return;
    }
  }

  Serial.println();
  Serial.println("===== SOIL PARAMETERS =====");
  Serial.print("Byte Response: ");

  // Tampilkan data balasan dalam format HEX
  String hexString;
  for (int j = 0; j < 19; j++) {
    hexString += receivedData[j] < 0x10 ? " 0" : " ";
    hexString += String(receivedData[j], HEX);
    hexString.toUpperCase();
  }
  Serial.println(hexString);

  // Ambil data dari array balasan dan konversi menjadi nilai aktual
  soilMoisture = sensorValue((int)receivedData[3], (int)receivedData[4]) * 0.1;
  soilTemp     = sensorValue((int)receivedData[5], (int)receivedData[6]) * 0.1;
  soilPH       = sensorValue((int)receivedData[9], (int)receivedData[10]) * 0.1;
  nitrogen     = sensorValue((int)receivedData[11], (int)receivedData[12]);
  phosphorus   = sensorValue((int)receivedData[13], (int)receivedData[14]);
  potassium    = sensorValue((int)receivedData[15], (int)receivedData[16]);
  
  // Tampilkan hasil
  Serial.println("Moisture: " + String(soilMoisture) + " %");
  Serial.println("Temperature: " + String(soilTemp) + " °C");
  Serial.println("pH: " + String(soilPH));
  Serial.println("Nitrogen (N): " + String(nitrogen) + " mg/kg");
  Serial.println("Phosporus (P): " + String(phosphorus) + " mg/kg");
  Serial.println("Potassium (K): " + String(potassium) + " mg/kg");

  delay(1000);
}

// Fungsi untuk menggabungkan dua byte menjadi integer
int sensorValue(int highByte, int lowByte) {
  int value = 0;
  value = highByte * 256;
  value = value + lowByte;
  return value;
}
