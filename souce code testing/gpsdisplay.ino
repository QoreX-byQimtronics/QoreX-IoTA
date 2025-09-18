#include <TinyGPS++.h>
#include <HardwareSerial.h>
#include <TFT_eSPI.h>
#include <SPI.h>

#define RXPin (33)
#define TXPin (32)

static const uint32_t GPSBaud = 9600;

// Objek TinyGPS++
TinyGPSPlus gps;

// Serial GPS (UART2)
HardwareSerial ss(2);

// Objek TFT
TFT_eSPI tft = TFT_eSPI();

void setup()
{
  Serial.begin(115200);
  ss.begin(GPSBaud, SERIAL_8N1, RXPin, TXPin, false);

  // Inisialisasi TFT
  tft.init();
  tft.setRotation(2);      // Atur orientasi
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_GREEN, TFT_BLACK);
  tft.setTextSize(2);
  tft.setCursor(20, 20);
  tft.println("GPS Module Ready");
  analogWrite(13,0);
  analogWrite(13,255);

  delay(2000);
  tft.fillScreen(TFT_BLACK);


  Serial.println(TinyGPSPlus::libraryVersion());
}

void loop()
{
  while (ss.available() > 0) {
    if (gps.encode(ss.read())) {
      displayInfo();
      delay(2000);
    }
  }
  if (millis() > 5000 && gps.charsProcessed() < 10)
  {
    Serial.println(F("No GPS detected: check wiring."));
    tft.fillScreen(TFT_BLACK);
    tft.setCursor(20, 40);
    tft.setTextColor(TFT_RED, TFT_BLACK);
    tft.println("No GPS detected!");
    while (true);
  }
}

void displayInfo()
{
  Serial.print(F("Location: ")); 

  tft.fillScreen(TFT_BLACK);   // Bersihkan layar
  tft.setCursor(10, 10);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(2);

  if (gps.location.isValid())
  {
    double lat = gps.location.lat();
    double lng = gps.location.lng();

    Serial.print(lat, 6);
    Serial.print(F(","));
    Serial.print(lng, 6);

    tft.printf("Lat: %.6f\n", lat);
    tft.printf("Lng: %.6f\n", lng);
  }
  else
  {
    Serial.print(F("INVALID"));
    tft.println("Location: INVALID");
  }

  Serial.println();

  // Tambahan: tampilkan info lain di layar
  tft.println();
  tft.printf("Satellites: %d\n", gps.satellites.value());
  
  if (gps.date.isValid() && gps.time.isValid())
  {
    tft.printf("Date: %02d/%02d/%d\n", gps.date.day(), gps.date.month(), gps.date.year());
    tft.printf("Time: %02d:%02d:%02d\n", gps.time.hour(), gps.time.minute(), gps.time.second());
  }
  else
  {
    tft.println("Date/Time: INVALID");
  }
}
