#include <Wire.h>       // Library komunikasi I2C
#include <PCF8575.h>    // Library ekspander I/O PCF8575
#include <LoRa.h>       // Library komunikasi LoRa

// Konfigurasi pin modul LoRa pada ESP32 (SPI)
#define LORA_RST     16  // Pin Reset LoRa
#define LORA_SS      17  // Pin Chip Select LoRa
#define LORA_MISO    19  // Pin MISO (Master In Slave Out)
#define LORA_MOSI    23  // Pin MOSI (Master Out Slave In)
#define LORA_SCK     18  // Pin Clock SPI

// Mapping pin DIO LoRa ke PCF8575 (pakai enum dari library Mischianti)
#define DIO0_PIN     P0  // DIO0 -> interrupt RX selesai
#define DIO1_PIN     P1  // DIO1 -> TX Done / CAD Done (opsional)
#define DIO2_PIN     P2  // DIO2 -> RX Done (opsional)

// Frekuensi kerja LoRa (ubah sesuai regulasi lokal)
#define BAND         915E6 // 915 MHz

// Objek SPI untuk LoRa (menggunakan HSPI)
SPIClass SPIRadio(HSPI);

// Objek PCF8575 di alamat I2C 0x20
PCF8575 pcf(0x20);

// Alamat node LoRa ini
byte localAddress = 0xBB;       // ID penerima
byte lastMsgId = 255;           // Menyimpan ID pesan terakhir yang diterima

// Buffer untuk menyimpan pesan lengkap (maks 1 KB)
uint8_t messageBuffer[1024];
int bufferIndex = 0;            // Posisi penulisan di buffer
const int totalExpectedLength = 1024; // Panjang pesan yang diharapkan

void setup() {
  Serial.begin(115200);
  while (!Serial);

  Serial.println("LoRa Receiver via PCF8575 DIO Pins");

  // Inisialisasi I2C untuk PCF8575
  Wire.begin();
  pcf.begin();

  // Set semua pin DIO di PCF sebagai input pull-up
  // Karena LoRa module yang akan memberikan sinyal
  pcf.pinMode(P0, INPUT_PULLUP);
  pcf.pinMode(P1, INPUT_PULLUP);
  pcf.pinMode(P2, INPUT_PULLUP);

  // Inisialisasi SPI untuk LoRa
  SPIRadio.begin(LORA_SCK, LORA_MISO, LORA_MOSI, LORA_SS);
  LoRa.setSPI(SPIRadio);
  LoRa.setPins(LORA_SS, LORA_RST, -1); // DIO langsung diakses via PCF, jadi -1

  // Set frekuensi kerja LoRa
  Serial.printf("Setup LoRa freq : %.0f\n", BAND);
  if (!LoRa.begin(BAND)) { // Jika gagal inisialisasi
    Serial.println("LoRa init FAILED");
    while (1); // Berhenti
  }
  Serial.println("LoRa init OK");
}

void loop() {
  // Cek status DIO0 via PCF (HIGH menandakan RX selesai)
  if (pcf.digitalRead(DIO0_PIN) == HIGH) {
    int packetSize = LoRa.parsePacket(); // Ambil ukuran paket
    if (packetSize) {
      processPacket(); // Proses paket yang diterima
    }
  }

  // (Opsional) DIO1 bisa dipakai untuk TX done, DIO2 untuk CAD done, dll.
}

void processPacket() {
  // Struktur header paket:
  // [recipient][sender][msgId][payloadLen][payloadData...]
  int recipient = LoRa.read();        // Alamat tujuan paket
  byte sender = LoRa.read();          // Alamat pengirim paket
  byte msgId = LoRa.read();           // ID pesan
  byte payloadLen = LoRa.read();      // Panjang data payload

  // Abaikan jika pesan bukan untuk kita dan bukan broadcast
  if (recipient != localAddress && recipient != 0xFF) {
    Serial.println("This message is not for me.");
    return;
  }

  // Cek apakah panjang payload sesuai
  if (payloadLen > LoRa.available()) {
    Serial.println("Fragment length mismatch");
    return;
  }

  // Jika msgId berbeda dari sebelumnya → mulai pesan baru
  if (msgId != lastMsgId) {
    bufferIndex = 0;
    lastMsgId = msgId;
  }

  // Simpan payload ke buffer
  while (LoRa.available() && bufferIndex < totalExpectedLength) {
    messageBuffer[bufferIndex++] = (uint8_t)LoRa.read();
  }

  // Tampilkan status penerimaan
  Serial.printf("Receiving fragment from 0x%X, msgId: %d, size: %d\n", sender, msgId, payloadLen);
  Serial.printf("Buffer: %d / %d bytes\n", bufferIndex, totalExpectedLength);

  // Jika semua data sudah diterima
  if (bufferIndex >= totalExpectedLength) {
    Serial.println("\n=== FULL MESSAGE RECEIVED ===");
    Serial.println("Message from Node: 0x" + String(sender, HEX));
    Serial.println("Message ID: " + String(msgId));
    Serial.println("Length: " + String(bufferIndex));

    // Cetak 100 karakter pertama dari pesan
    Serial.print("Data (first 100 chars): ");
    for (int i = 0; i < 100 && i < totalExpectedLength; i++) {
      Serial.print((char)messageBuffer[i]);
    }
    Serial.println("...");

    // Info sinyal
    Serial.println("RSSI: " + String(LoRa.packetRssi()));
    Serial.println("SNR: " + String(LoRa.packetSnr()));
    Serial.println("=============================\n");

    // Reset buffer untuk pesan berikutnya
    bufferIndex = 0;
  }
}
