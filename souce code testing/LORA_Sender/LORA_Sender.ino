#include <Wire.h>       // Library komunikasi I2C
#include <PCF8575.h>    // Library untuk ekspander I/O PCF8575
#include <LoRa.h>       // Library untuk komunikasi LoRa

// Konfigurasi pin modul LoRa pada ESP32 (menggunakan SPI)
#define LORA_RST     16  // Pin Reset LoRa
#define LORA_SS      17  // Pin Chip Select (CS)
#define LORA_MISO    19  // Pin MISO (Master In Slave Out)
#define LORA_MOSI    23  // Pin MOSI (Master Out Slave In)
#define LORA_SCK     18  // Pin Clock SPI

// Pin DIO modul LoRa yang dihubungkan lewat ekspander PCF8575
#define DIO0_PIN     P0  // DIO0 -> menandakan TX selesai
#define DIO1_PIN     P1  // DIO1 -> RX Timeout/CAD Done (opsional)
#define DIO2_PIN     P2  // DIO2 -> RX Done (opsional untuk sender)

// Frekuensi kerja LoRa (sesuaikan dengan modul dan regulasi lokal)
#define BAND         915E6 // 915 MHz

// Inisialisasi SPI untuk LoRa menggunakan HSPI
SPIClass SPIRadio(HSPI);

// Inisialisasi objek PCF8575 dengan alamat I2C 0x20
PCF8575 pcf(0x20);

// Alamat node untuk LoRa
byte localAddress = 0xAA;  // ID pengirim
byte destination  = 0xBB;  // ID tujuan
byte msgCount     = 0;     // Nomor pesan (increment setiap pengiriman)

// Fungsi untuk mengirim satu fragmen data
void sendFragment(byte destination, byte* buffer, byte msgId, int len) {
  LoRa.beginPacket();         // Mulai paket baru
  LoRa.write(destination);    // Tulis alamat tujuan
  LoRa.write(localAddress);   // Tulis alamat pengirim
  LoRa.write(msgId);          // Tulis ID pesan
  LoRa.write(len);            // Tulis panjang data
  LoRa.write(buffer, len);    // Tulis data isi
  LoRa.endPacket();           // Kirim paket
}

void setup() {
  Serial.begin(115200);
  while (!Serial);

  Serial.println("LoRa Sender via PCF8575 DIO Pins");

  // Inisialisasi I2C untuk PCF8575
  Wire.begin();
  pcf.begin();

  // Set pin DIO di PCF sebagai input dengan pull-up internal
  // karena modul LoRa yang akan memberikan sinyal
  pcf.pinMode(DIO0_PIN, INPUT_PULLUP);
  pcf.pinMode(DIO1_PIN, INPUT_PULLUP);
  pcf.pinMode(DIO2_PIN, INPUT_PULLUP);

  // Inisialisasi SPI untuk LoRa
  SPIRadio.begin(LORA_SCK, LORA_MISO, LORA_MOSI, LORA_SS);
  LoRa.setSPI(SPIRadio);
  LoRa.setPins(LORA_SS, LORA_RST, -1); // DIO tidak dihubungkan langsung (pakai PCF8575)

  // Set frekuensi kerja LoRa
  Serial.printf("Setup LoRa freq : %.0f\n", BAND);
  if (!LoRa.begin(BAND)) { // Cek apakah LoRa berhasil diinisialisasi
    Serial.println("LoRa init FAILED");
    while (1); // Berhenti jika gagal
  }
  Serial.println("LoRa init OK");
}

void loop() {
  // Membuat data dummy berukuran 1KB
  uint8_t data[1024];
  for (int i = 0; i < 1024; i++) {
    data[i] = 'A' + (i % 26); // Isi data dengan huruf A-Z berulang
  }

  // Ukuran setiap fragmen pengiriman
  const int fragmentSize = 200;
  int totalFragments = (1024 + fragmentSize - 1) / fragmentSize; // Hitung total fragmen

  Serial.printf("Sending %d fragments, msgId: %d\n", totalFragments, msgCount);

  // Kirim data per fragmen
  for (int i = 0; i < totalFragments; i++) {
    int startIdx = i * fragmentSize;               // Posisi awal fragmen
    int len = min(fragmentSize, 1024 - startIdx);  // Panjang data fragmen

    sendFragment(destination, data + startIdx, msgCount, len); // Kirim fragmen
    Serial.printf("Sent fragment %d/%d, size: %d bytes\n", i + 1, totalFragments, len);

    // Tunggu TX selesai dengan memantau DIO0 melalui PCF8575
    while (pcf.digitalRead(DIO0_PIN) == LOW) {
      delay(1);
    }
    delay(100); // Delay antar fragmen agar penerima sempat memproses
  }

  // Increment nomor pesan
  msgCount++;

  // Delay 5 detik sebelum kirim batch berikutnya
  delay(5000);
}
