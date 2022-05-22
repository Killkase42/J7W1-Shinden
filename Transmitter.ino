#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <Wire.h>
#include <LiquidCrystal.h>


// Define the digital inputs
#define t1 7   // Toggle switch 1

RF24 radio(5, 6);   // nRF24L01 (CE, CSN)
const byte address[6] = "00001"; // Address

// Max size of this struct is 32 bytes - NRF24L01 buffer limit
struct Data_Package {
  byte j1PotX;
  byte j1PotY;
  byte j2PotX;
  byte j2PotY;
  byte tSwitch1;
};

Data_Package data; //Create a variable with the above structure

// Define LCD screen properties
int Contrast = 10;
LiquidCrystal lcd(2, 3, 4, 7, 8, 9); //LiquidCrystal(rs, enable, d4, d5, d6, d7)


void setup() {
    
  // Define the radio communication
  radio.begin();
  Serial.begin(9600);
  radio.openWritingPipe(address);
  radio.setAutoAck(false);
  radio.setDataRate(RF24_250KBPS);
  radio.setPALevel(RF24_PA_LOW);
  
  // Activate the Arduino internal pull-up resistors
  pinMode(t1, INPUT_PULLUP);

  // Set initial default values
  data.j1PotX = 127; // Values from 0 to 255. When Joystick is in resting position, the value is in the middle, or 127. We actually map the pot value from 0 to 1023 to 0 to 255 because that's one BYTE value
  data.j1PotY = 127;
  data.j2PotX = 127;
  data.j2PotY = 127;
  data.tSwitch1 = 1;

  // Set initial LCD screen properties
  analogWrite(10, Contrast);
  lcd.begin(16, 2);
  
}

void loop() {
  
  //Change LCD display
  lcd.setCursor(0, 0);
  lcd.print("C1:     ");
  lcd.print("C2: ");


  lcd.setCursor(0, 1);
  lcd.print("C3:     ");
  lcd.print("C4: ");
  
  
  // Read all analog inputs and map them to one Byte value
  // 1Pot is left hand and 2Pot is right hand
  data.j1PotX = map(analogRead(A0), 0, 1023, 0, 255); // Convert the analog read value from 0 to 1023 into a BYTE value from 0 to 255
  data.j1PotY = map(analogRead(A1), 0, 1023, 0, 255);
  data.j2PotX = map(analogRead(A3), 0, 1023, 0, 255);
  data.j2PotY = map(analogRead(A3), 0, 1023, 0, 255);

  // For testing puroposes, prints values to serial monitor
  Serial.print("Yaw: ");
  Serial.println(data.j1PotX);
  Serial.print("Throttle: ");
  Serial.println(data.j1PotY);
  Serial.print("Roll: ");
  Serial.println(data.j2PotX);
  Serial.print("Pitch: ");
  Serial.println(data.j2PotY);
  Serial.println("--------------");
  
    // If toggle switch 1 is switched on
  if (digitalRead(t1) == 0) {
    
  }
  
  // Send the whole data from the structure to the receiver
  radio.write(&data, sizeof(Data_Package));

  //delay(1000); //Just for testing purposes.
  
}
