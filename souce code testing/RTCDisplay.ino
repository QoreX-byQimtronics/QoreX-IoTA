// Date and time functions using a PCF8563 RTC connected via I2C and Wire lib
#include "RTClib.h"
#include <TFT_eSPI.h>
#include <SPI.h>

// RTC
RTC_PCF8563 rtc;
char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

// TFT
TFT_eSPI tft = TFT_eSPI();

void setup () {
  Serial.begin(115200);
  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    Serial.flush();
    while (1) delay(10);
  }

  if (rtc.lostPower()) {
    Serial.println("RTC is NOT initialized, setting the time!");
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }

  rtc.start();

  // Inisialisasi TFT
  tft.init();
  tft.setRotation(2);
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_GREEN, TFT_BLACK);
  tft.setTextSize(2);
  tft.setCursor(20, 20);
  tft.println("RTC Module Ready");
  analogWrite(13,0);
  analogWrite(13,255);
  delay(2000);
  tft.fillScreen(TFT_BLACK);

}

void loop () {
  DateTime now = rtc.now();

  // Tampilkan di Serial Monitor
  Serial.print(now.year(), DEC);
  Serial.print('/');
  Serial.print(now.month(), DEC);
  Serial.print('/');
  Serial.print(now.day(), DEC);
  Serial.print(" (");
  Serial.print(daysOfTheWeek[now.dayOfTheWeek()]);
  Serial.print(") ");
  Serial.print(now.hour(), DEC);
  Serial.print(':');
  Serial.print(now.minute(), DEC);
  Serial.print(':');
  Serial.print(now.second(), DEC);
  Serial.println();

  // Tampilkan di TFT
  tft.fillRect(0, 0, 240, 100, TFT_BLACK); // hapus area atas
  tft.setCursor(10, 10);
  tft.setTextSize(2);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.printf("%04d/%02d/%02d", now.year(), now.month(), now.day());
  tft.setCursor(10, 40);
  tft.printf("%s", daysOfTheWeek[now.dayOfTheWeek()]);
  tft.setCursor(10, 70);
  tft.printf("%02d:%02d:%02d", now.hour(), now.minute(), now.second());

  delay(1000);
}
