#include <SoftwareSerial.h>
#include <TFT_eSPI.h>       // Library TFT_eSPI
#include <SPI.h>

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

// TFT object
TFT_eSPI tft = TFT_eSPI(); 

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

  // Inisialisasi TFT
  tft.init();
  tft.setRotation(2);          // Atur orientasi layar
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_GREEN, TFT_BLACK);
  tft.setTextSize(2);
  tft.setCursor(10, 10);
  tft.println("SOIL SENSOR READY");
  delay(2000);
  tft.fillScreen(TFT_BLACK);
  analogWrite(13,0);
  analogWrite(13,255);
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

  // Ambil data dari array balasan dan konversi menjadi nilai aktual
  soilMoisture = sensorValue((int)receivedData[3], (int)receivedData[4]) * 0.1;
  soilTemp     = sensorValue((int)receivedData[5], (int)receivedData[6]) * 0.1;
  soilPH       = sensorValue((int)receivedData[9], (int)receivedData[10]) * 0.1;
  nitrogen     = sensorValue((int)receivedData[11], (int)receivedData[12]);
  phosphorus   = sensorValue((int)receivedData[13], (int)receivedData[14]);
  potassium    = sensorValue((int)receivedData[15], (int)receivedData[16]);
  
  // ===================== SERIAL MONITOR =====================
  Serial.println();
  Serial.println("===== SOIL PARAMETERS =====");
  Serial.println("Moisture: " + String(soilMoisture) + " %");
  Serial.println("Temperature: " + String(soilTemp) + " °C");
  Serial.println("pH: " + String(soilPH));
  Serial.println("Nitrogen (N): " + String(nitrogen) + " mg/kg");
  Serial.println("Phosporus (P): " + String(phosphorus) + " mg/kg");
  Serial.println("Potassium (K): " + String(potassium) + " mg/kg");

  // ===================== TFT DISPLAY =====================
  tft.fillScreen(TFT_BLACK);
  tft.setCursor(0, 50);
  tft.printf("Moisture : %.1f %%\n", soilMoisture);
  tft.printf("Temp     : %.1f C\n", soilTemp);
  tft.printf("pH       : %.1f\n", soilPH);
  tft.printf("Nitrogen : %.0f mg/kg\n", nitrogen);
  tft.printf("Phosphor : %.0f mg/kg\n", phosphorus);
  tft.printf("Potassium: %.0f mg/kg\n", potassium);

  delay(2000);
}

// Fungsi untuk menggabungkan dua byte menjadi integer
int sensorValue(int highByte, int lowByte) {
  int value = 0;
  value = highByte * 256;
  value = value + lowByte;
  return value;
}
