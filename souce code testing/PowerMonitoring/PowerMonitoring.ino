#include <Wire.h>
#include <INA226_WE.h>

#define INA226_I2C_ADDR 0x41  // Alamat I2C sensor INA226 (Input Batt) || Jika ingin melihat VINPUT pada adventure bricks IoT ganti 0x41 menjadi 0X40

// Membuat objek INA226 dengan alamat I2C yang ditentukan
INA226_WE sensorINA226 = INA226_WE(INA226_I2C_ADDR);

void setup() {
  Serial.begin(9600);
  Wire.begin();

  // Inisialisasi sensor, jika gagal maka berhenti
  if (!sensorINA226.init()) {
    Serial.println("Gagal menginisialisasi INA226. Periksa sambungan kabel!");
    while (1) {}
  }

  /* Mengatur jumlah pengukuran rata-rata untuk shunt dan bus voltage
     Pilihan:
     AVERAGE_1     -> 1 kali pengukuran (default)
     AVERAGE_4     -> 4 kali
     AVERAGE_16    -> 16 kali
     AVERAGE_64    -> 64 kali
     AVERAGE_128   -> 128 kali
     AVERAGE_256   -> 256 kali
     AVERAGE_512   -> 512 kali
     AVERAGE_1024  -> 1024 kali
  */
  // sensorINA226.setAverage(AVERAGE_16);

  /* Mengatur waktu konversi (µs) untuk pengukuran
     Satu set pengukuran shunt + bus voltage membutuhkan:
     jumlah rata-rata x waktu konversi x 2
     
     CONV_TIME_140   -> 140 µs
     CONV_TIME_204   -> 204 µs
     CONV_TIME_332   -> 332 µs
     CONV_TIME_588   -> 588 µs
     CONV_TIME_1100  -> 1.1 ms (default)
     CONV_TIME_2116  -> 2.116 ms
     CONV_TIME_4156  -> 4.156 ms
     CONV_TIME_8244  -> 8.244 ms
  */
  // sensorINA226.setConversionTime(CONV_TIME_1100);

  /* Mengatur mode pengukuran
     POWER_DOWN -> sensor mati
     TRIGGERED  -> ukur hanya saat diminta
     CONTINUOUS -> ukur terus menerus (default)
  */
  // sensorINA226.setMeasureMode(CONTINUOUS);

  /* Koreksi faktor arus jika hasil berbeda dari alat ukur terkalibrasi
     Rumus: faktor_koreksi = arus_alat_ukur / arus_INA226
  */
  // sensorINA226.setCorrectionFactor(0.95);

  Serial.println("Contoh Penggunaan Sensor INA226 - Mode Continuous");

  // Menunggu hingga konversi pertama selesai
  sensorINA226.waitUntilConversionCompleted();
}

void loop() {
  // Variabel hasil pengukuran
  float teganganShunt_mV = sensorINA226.getShuntVoltage_mV(); // Tegangan di resistor shunt (mV)
  float teganganBus_V    = sensorINA226.getBusVoltage_V();    // Tegangan bus (V)
  float arus_mA          = sensorINA226.getCurrent_mA();      // Arus (mA)
  float daya_mW          = sensorINA226.getBusPower();        // Daya (mW)
  float teganganBeban_V  = teganganBus_V + (teganganShunt_mV / 1000.0); // Tegangan beban total (V)

  // Mengecek apakah ada error I2C
  cekErrorI2C();

  // Menampilkan hasil pengukuran
  Serial.print("Tegangan Shunt [mV]: "); Serial.println(teganganShunt_mV);
  Serial.print("Tegangan Bus   [V] : "); Serial.println(teganganBus_V);
  Serial.print("Tegangan Beban [V] : "); Serial.println(teganganBeban_V);
  Serial.print("Arus           [mA]: "); Serial.println(arus_mA);
  Serial.print("Daya Bus       [mW]: "); Serial.println(daya_mW);

  if (!sensorINA226.overflow) {
    Serial.println("Status: OK - Tidak ada overflow");
  } else {
    Serial.println("Status: OVERFLOW - Gunakan range arus lebih tinggi");
  }
  Serial.println();

  delay(3000); // jeda 3 detik
}

// Fungsi untuk memeriksa error komunikasi I2C
void cekErrorI2C() {
  byte kodeError = sensorINA226.getI2cErrorCode();
  if (kodeError) {
    Serial.print("Error I2C: ");
    Serial.println(kodeError);
    switch (kodeError) {
      case 1: Serial.println("Data terlalu panjang untuk buffer"); break;
      case 2: Serial.println("NACK saat mengirim alamat"); break;
      case 3: Serial.println("NACK saat mengirim data"); break;
      case 4: Serial.println("Error lain"); break;
      case 5: Serial.println("Timeout"); break;
      default: Serial.println("Error tidak dikenali");
    }
    while (1) {} // Hentikan program
  }
}
