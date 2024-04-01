#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Keypad.h>
#include <Servo.h>
#include <SPI.h>
#include <MFRC522.h>

#define SS_PIN 53
#define RST_PIN 9

const int numRows = 4;
const int numCols = 4;
char k[numRows][numCols] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};
byte rowPins[numRows] = {44,42,40,38};
byte colPins[numCols] = {36,34,32,30};
const int buzzerPin = 12; // 蜂鸣器引脚
const int RELAY_PIN = 11; // 电磁锁控制引脚

Keypad myKeypad = Keypad(makeKeymap(k), rowPins, colPins, numRows, numCols);
LiquidCrystal_I2C lcd(0x27, 16, 2);
MFRC522 mfrc522(SS_PIN, RST_PIN);

const String password = "1014ABC";
String in_pass;
bool unlocked = false;
unsigned long unlockTime = 0;
unsigned long buzzStartTime = 0;

void setup() {
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("    welcome!    ");
  delay(1000);
  lcd.setCursor(0, 1);
  lcd.print(" UNLOCK ");
  delay(1000);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("PZ password");
  delay(1000);
  lcd.setCursor(0, 1);
  lcd.print("wait & '*' !");
  delay(1000);
  lcd.clear();
  delay(1000);
  lcd.setCursor(0, 0);
  lcd.print("wait.........");
  delay(2000);
  lcd.clear();

  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW);

  pinMode(buzzerPin, OUTPUT);
  digitalWrite(buzzerPin, LOW);

  SPI.begin();
  mfrc522.PCD_Init();
}

void loop() {
  char keypressed = myKeypad.getKey();
  if (keypressed != NO_KEY) {
    lcd.setCursor(0, 0);
    if (keypressed == '*') { 
      if (password == in_pass) {
        lcd.print("....Unlocked....");
        unlocked = true;
        digitalWrite(RELAY_PIN, HIGH);
        lcd.setCursor(0, 1);
        lcd.print("     OPEN      ");
        unlockTime = millis();
        buzzStartTime = millis();
        tone(buzzerPin, 1000);
      } else {
        lcd.setCursor(0, 0);
        lcd.print("   wrong password!    ");
        delay(1000);
        lcd.setCursor(0, 1);
        lcd.print(" Try again ");
        delay(1000);
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("PZ password");
        delay(1000);
        lcd.setCursor(0, 1);
        lcd.print("thenpress '*' !");
        delay(1000);
        lcd.clear();
        in_pass = "";
        tone(buzzerPin, 1500);
        delay(500);
        noTone(buzzerPin);
      }
    } else {
      in_pass += keypressed;
    }
  }
  
  if (unlocked) {
    digitalWrite(RELAY_PIN, HIGH);
  }
  
  if (unlocked && (millis() - unlockTime >= 8000)) {
    digitalWrite(RELAY_PIN, LOW);
    unlocked = false;
    noTone(buzzerPin);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("    welcome!    ");
    delay(1000);
    lcd.setCursor(0, 1);
    lcd.print(" UNLOCK ");
    delay(1000);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("PZ password");
    delay(1000);
    lcd.setCursor(0, 1);
    lcd.print("wait & '*' !");
    delay(1000);
    lcd.clear();
    in_pass = "";
  }
  
  if (unlocked && (millis() - buzzStartTime >= 1500)) {
    noTone(buzzerPin);
  }

  if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {
    String uid = getUID();
    if (uid == "13 8E 7F FA") { // Change this to your RFID tag's UID
      lcd.clear();
      lcd.setCursor(0, 1);
      lcd.print("RFID Access");
      unlockDoor();
    } else {
      lcd.clear();
      lcd.setCursor(0, 1);
      lcd.print("Invalid RFID");
      tone(buzzerPin, 1500);
      delay(500);
      noTone(buzzerPin);
    }
    delay(1000);
  }
}

String getUID() {
  String content= "";
  byte letter;
  for (byte i = 0; i < mfrc522.uid.size; i++) 
  {
     content.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
     content.concat(String(mfrc522.uid.uidByte[i], HEX));
  }
  content.toUpperCase();
  return content.substring(1);
}

void unlockDoor() {
  unlocked = true;
  digitalWrite(RELAY_PIN, HIGH); //
    lcd.setCursor(0, 1);
  lcd.print("     OPEN      ");
  unlockTime = millis(); // 記錄解鎖時間
  buzzStartTime = millis(); // 記錄蜂鳴器開始時間
  tone(buzzerPin, 1000); // 播放 1kHz 的聲音
}
