#include <SPI.h>
#include <MFRC522.h>
#include <EEPROM.h>
#include <Servo.h>

#define RST_PIN	9
#define SS_PIN 10
#define BUZZER_PIN 3
#define BUTTON_PIN 7
#define LED_PIN 0
#define SERVO_PIN 6
#define HOLA_PIN 0

MFRC522 mfrc522(SS_PIN, RST_PIN);	// Create MFRC522 instance
Servo servo;

bool isOpen = false;

void setup() {
	Serial.begin(9600);		// Initialize serial communications with the PC
	while (!Serial);		// Do nothing if no serial port is opened (added for Arduinos based on ATMEGA32U4)
	SPI.begin();			// Init SPI bus
  mfrc522.PCD_Init();		// Init MFRC522

  // sound
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(1, OUTPUT);
  pinMode(HOLA_PIN, INPUT_PULLUP);

  digitalWrite(1, HIGH);

  // servo init
  servo.attach(SERVO_PIN);
  openDoor();

  Serial.println("Waiting for card...");
}

void loop() {
  // close door
  if (digitalRead(HOLA_PIN) == LOW) {
    tryCloseDoor();
  }

	// Look for new cards
	if (!mfrc522.PICC_IsNewCardPresent()) {
		return;
	}

	// Select one of the cards
	if (!mfrc522.PICC_ReadCardSerial()) {
		return;
	}

  uint32_t card = getIdCard();

  // service button
  if (digitalRead(BUTTON_PIN) == LOW && isOpen) {
     bool addCardSuccess = tryAddCardInMremory(card);
     Serial.println("Try add card");

     if (addCardSuccess) {
       Serial.println("New card!");
       playSoundAddCard();
     } else {
       Serial.println("Try remove card");
       bool removeCardSuccess = tryRemoveCardFromMremory(card);

       if (removeCardSuccess) {
         Serial.println("Card removed!");
         playSoundRemoveCard();
       }
     }
     delay(2000);

    return;
   }
   
  // check card
  if (isCardInMremory(card)) {
    playSoundAccess();
    Serial.println("Access!");
    openDoor();

    return;
  }

  playSoundNotAccess();
  Serial.println("NotAccess!");

  delay(1000);
}

bool isCardInMremory(uint32_t id) {
  for (int i = 0; i < EEPROM.length(); i += 4) {
    uint32_t card = 0;
    EEPROM.get(i, card);

    if (card == id) return true;
  }

  return false;
}

bool tryAddCardInMremory(uint32_t card) {
  for (int i = 0; i < EEPROM.length(); i += 4) {
    uint32_t data = 0;
    EEPROM.get(i, data);

    if (data == card) return false;

    if (data == 255) {
      EEPROM.put(i, card);

      return true;
    }
  }
  return false;
}

void moveCardsInPositionInMremory(int position) {
  for (int i = position; i < EEPROM.length(); i += 4) {
    if (i + 4 >= EEPROM.length()) return;

    uint32_t nextData = 0;
    EEPROM.get(i + 4, nextData);

    if (nextData == 255) return;

    EEPROM.put(i + 4, 255);
    EEPROM.put(i, nextData);
  }
}

bool tryRemoveCardFromMremory(uint32_t card) {
  for (int i = 0; i < EEPROM.length(); i += 4) {
    uint32_t data = 0;
    EEPROM.get(i, data);

    if (data == card) {
      EEPROM.put(i, 255);

      // replacing
      moveCardsInPositionInMremory(i);

      return true;
    }
  }
  return false;
}

bool isHaveCardInMremory() {
  for (int i = 0; i < EEPROM.length(); i += 4) {
    uint32_t data = 0;
    EEPROM.get(i, data);

    if (data != 255) {
      return true;
    }
  }
  return false;
}

void clearMremory() {
  for (int i = 0; i < EEPROM.length(); i += 4) {
    uint32_t data = 255;
    EEPROM.put(i, data);
  }
}

void printAllMremory() {
  for (int i = 0; i < EEPROM.length(); i += 4) {
    uint32_t f = 0;
    EEPROM.get(i, f);

    Serial.println(f);
  }
}


uint32_t getIdCard() {
  return *(uint32_t *)mfrc522.uid.uidByte;
}

// sounds
void playSoundAddCard() {
  for (byte i = 0; i < 3; i++) {
    analogWrite(BUZZER_PIN, 100);
    delay(150);
    analogWrite(BUZZER_PIN, 0);
    delay(150);
  }
}

void playSoundRemoveCard() {
  for (byte i = 0; i < 2; i++) {
    analogWrite(BUZZER_PIN, 100);
    delay(150);
    analogWrite(BUZZER_PIN, 0);
    delay(150);
  }
}

void playSoundAccess() {
  for (byte i = 0; i < 2; i++) {
    analogWrite(BUZZER_PIN, 100);
    delay(150);
    analogWrite(BUZZER_PIN, 0);
    delay(150);
  }
}

void playSoundNotAccess() {
  for (byte i = 0; i < 1; i++) {
    analogWrite(BUZZER_PIN, 100);
    delay(250);
    analogWrite(BUZZER_PIN, 0);
    delay(150);
  }
}
void tryCloseDoor() {
  if (!isHaveCardInMremory()) return;

  closeDoor();
}

// door
void openDoor() {
  // servo
  isOpen = true;
  servo.write(90);
  delay(2000);
}

void closeDoor() {
  // servo
  isOpen = false;
  servo.write(-90);
  delay(2000);
}
