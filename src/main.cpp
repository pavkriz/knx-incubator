/*
 * Blink
 * Turns on an LED on for one second,
 * then off for one second, repeatedly.
 */

#include <Arduino.h>

enum ReceiverState {
  WAITING_FIRST_BIT,
  ALL_BITS_RECEIVED,
};

int n;
int previousAdc = 0;
uint32 timestamps[20];
uint32 samples[20];
#define TELEGRAM_MAX_LENGTH 24
int telegramLength = 0;
int telegram[TELEGRAM_MAX_LENGTH]; // telegram bytes (bit 0..7) including parity (bit 8) and stop bit (bit 9)
uint32 lastByteTimestamp = 0;
uint32 previousLastByteTimestamp = 0;
uint32 firstByteTimestamp = 0;

#define TX_US 35
#define BIT_US 104
#define INTER_TELEGRAM_GAP_US 1000
#define ADC_TRESHOLD 550

void setup()
{
  // initialize LED digital pin as an output.
  pinMode(PB1, OUTPUT);

  pinMode(PA0, INPUT_ANALOG);

  Serial.begin(9600);  // BAUD has no effect on USB serial: placeholder for physical UART
  // wait for serial monitor to be connected.
  while (!Serial)
  {
    digitalWrite(PB1,!digitalRead(PB1)); // Turn the LED from off to on, or on to off
    delay(100);         // fast blink
  }
  Serial.println("Blink LED & count Demo");
}

void testParityAndFraming(int byteNumber, int byteWithParityAndStopBit)
{
  int parity = 0;
  for (int i = 0; i < 8; i++) {
    parity ^= ((byteWithParityAndStopBit>>i) & 1) ? 1 : 0; // XOR    
  }
  if (((byteWithParityAndStopBit & 256) >> 8) != parity) {
    Serial.print("ERR: wrong parity bit in byte #");
    Serial.println(byteNumber);
  }
  if ((byteWithParityAndStopBit & 512) == 0) {
    Serial.print("ERR: wrong framing (no stop bit) in byte #");
    Serial.println(byteNumber);
  }
}

void dumpTelegram() 
{
  if (previousLastByteTimestamp > 0) {
    Serial.println("========== GAP ==========");  
    Serial.print(firstByteTimestamp - previousLastByteTimestamp);
    Serial.println(" us");
  }
  Serial.println("========== KNX TELEGRAM ==========");
  for (int i = 0; i < telegramLength; i++) {
    testParityAndFraming(i, telegram[i]);
  }
  Serial.print("RAW: ");
  for (int i = 0; i < telegramLength; i++) {
    Serial.print(telegram[i] & 255);
    Serial.print(" ");
  }
  Serial.println();
  if ((telegram[0] & 0b01010011) == 0b00010000) {
    // correct control field
    Serial.print("CTR:");
    Serial.print(" prio");
    switch ((telegram[0] & 0b1100) >> 2) {
      case 0: Serial.print("_system");
              break;
      case 1: Serial.print("_alarm");
              break;
      case 2: Serial.print("_normal_high");
              break;
      case 3: Serial.print("_normal_low");
              break;
    }
    if (telegram[0] & 0b00100000) {
      Serial.print(" not_repeated");
    } else {
      Serial.print(" repeated");
    }
    if (telegram[0] & 0b10000000) {
      Serial.print(" standard_frame");
    } else {
      Serial.print(" extended_frame");
    }
    Serial.println();
    Serial.print("SRC: ");
    Serial.print((telegram[1] & 0b11110000) >> 4);
    Serial.print(".");
    Serial.print(telegram[1] & 0b00001111);
    Serial.print(".");
    Serial.print(telegram[2] & 255);
    Serial.println();
    Serial.print("DST: ");
    if (telegram[5] & 0b10000000) {
      // group destination address
      Serial.print((telegram[3] & 0b11111000) >> 3);
      Serial.print("/");
      Serial.print(telegram[3] & 0b00000111);
      Serial.print("/");
      Serial.print(telegram[4] & 255);
      Serial.println();
    } else {
      // individual destination address
      Serial.print((telegram[3] & 0b11110000) >> 4);
      Serial.print(".");
      Serial.print(telegram[3] & 0b00001111);
      Serial.print(".");
      Serial.print(telegram[4] & 255);
      Serial.println();
    }
    Serial.print("NPDU:");
    Serial.print(" routing=");
    Serial.print((telegram[5] & 0b01110000) >> 4);
    Serial.print(" length=");
    Serial.print(telegram[5] & 0b1111);
    Serial.println();
    Serial.print("CMD: ");
    if ((telegram[6] & 0b11000000) == 0) {
      int command = ((telegram[6] & 0b11)*4 + ((telegram[7] & 0b11000000) >> 6));
      if (command == 0b0010) {
        Serial.print("value write ");
        Serial.print(telegram[7] & 0b111111);
      } else {
        Serial.print("unknown command ");
        Serial.print(command);
      }
    } else {
      Serial.print("Invalid command");
    }
    Serial.println();
  } else if ((telegram[0] & 255) == 0xcc) {
    Serial.println("ACK: positive");
  } else if ((telegram[0] & 255) == 0x0c) {
    Serial.println("ACK: negative");
  } else if ((telegram[0] & 255) == 0xc0) {
    Serial.println("ACK: busy");
  } else {
    Serial.print("Incorrect control field: ");
    Serial.println(telegram[0], BIN);
  }
}

void loop()
{
  // waiting start  bit
  int currentAdc = 0;
  do {
    currentAdc = analogRead(PA0);
    if ((telegramLength > 0) && (micros() - lastByteTimestamp > INTER_TELEGRAM_GAP_US)) {
      dumpTelegram();
      telegramLength = 0;
    }
  } while (currentAdc > ADC_TRESHOLD);
  firstByteTimestamp = micros();
  // voltage drop detected -> byte transmission (start bit)
  // should be about 35us low, align to the middle of the pulse
  delayMicroseconds(TX_US/2);
  int byteRead = 0;
  int mult = 1;  
  // read 10 following bits into byteRead (actually including parity and stop bit)
  for (int i = 0; i < 10; i++) {
    delayMicroseconds(BIT_US-8); // bit period at 9600bit/s minus ADC conversion time etc.
    int sample = analogRead(PA0);
    byteRead += mult * ((sample > ADC_TRESHOLD) ? 1 : 0);
    mult *= 2;    
  }
  if (telegramLength == 0) {
    previousLastByteTimestamp = lastByteTimestamp;
  }
  lastByteTimestamp = micros();
  telegram[telegramLength++] = byteRead;
}