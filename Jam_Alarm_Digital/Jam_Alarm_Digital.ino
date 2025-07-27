#include <Wire.h>
#include <RTClib.h>
#include <LiquidCrystal_I2C.h>
#include <TM1637Display.h>

#define CLK 2
#define DIO 3

RTC_DS3231 rtc;
LiquidCrystal_I2C lcd(0x27, 16, 2);
TM1637Display display(CLK, DIO);

const int buzzerPin = 6;

bool alarm1Triggered = false;
bool alarm2Triggered = false;

unsigned long previousMillisJam = 0;
const unsigned long intervalJam = 1000;

void setup() {
  Wire.begin();
  Serial.begin(9600);
  lcd.init();
  lcd.backlight();
  display.setBrightness(0x0f);

  pinMode(buzzerPin, OUTPUT);

  if (!rtc.begin()) {
    lcd.print("RTC Error!");
    Serial.println("RTC tidak terdeteksi!");
    while (1);
  }

  if (rtc.lostPower()) {
    rtc.adjust(DateTime(F(_DATE), F(TIME_))); // Set waktu otomatis saat upload
  }
}

void loop() {
  unsigned long currentMillis = millis();

  // => perubahan waktu tiap 1 detik
  if (currentMillis - previousMillisJam >= intervalJam) {
    previousMillisJam = currentMillis;

    DateTime now = rtc.now();
    float suhu = rtc.getTemperature();
    int suhuInt = (int)suhu;

    // => LCD
    lcd.setCursor(0, 0);
    lcd.print("JAM -> ");
    lcd.print(now.hour() < 10 ? "0" : ""); lcd.print(now.hour());
    lcd.print(":");
    lcd.print(now.minute() < 10 ? "0" : ""); lcd.print(now.minute());
    lcd.print(":");
    lcd.print(now.second() < 10 ? "0" : ""); lcd.print(now.second());

    lcd.setCursor(0, 1);
    lcd.print("TGL -> ");
    lcd.print(now.day()); lcd.print("-");
    lcd.print(now.month()); lcd.print("-");
    lcd.print(now.year());

    // => SEVEN SEGMENT
    int puluhan = suhuInt / 10;
    int satuan = suhuInt % 10;
    #define SEG_C (SEG_A | SEG_D | SEG_E | SEG_F)
    const uint8_t suhuSeg[] = {
      display.encodeDigit(puluhan),
      display.encodeDigit(satuan),
      SEG_A | SEG_B | SEG_F | SEG_G,
      SEG_C
    };
    display.setSegments(suhuSeg);

    // => ALARM 1
    if (now.hour() == 7 && now.minute() == 35 && now.second() == 0 && !alarm1Triggered) {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("TANGI OO!!!");
      tone(buzzerPin, 10000);
      delay(30000);
      noTone(buzzerPin);
      alarm1Triggered = true;
    }

    // => ALARM 2
    if (now.hour() == 20 && now.minute() == 45 && now.second() == 30 && !alarm2Triggered) {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("SIAP-SIAP!!!");
      tone(buzzerPin, 10000);
      delay(30000);
      noTone(buzzerPin);
      alarm2Triggered = true;
    }

    // => RESET ALARM TENGAH MALAM
    if (now.hour() == 0 && now.minute() == 0 && now.second() == 1) {
      alarm1Triggered = false;
      alarm2Triggered = false;
    }
  }
}