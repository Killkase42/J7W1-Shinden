#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>


//Define basic voltage checker properties
//-----------------------------------------------
#define MOVING_AVERAGE_COUNT 16

unsigned int samples0[MOVING_AVERAGE_COUNT];
unsigned int samples1[MOVING_AVERAGE_COUNT];
unsigned int samples2[MOVING_AVERAGE_COUNT];
unsigned int samples3[MOVING_AVERAGE_COUNT];

int maIndex0 = 0;
int maIndex1 = 0;
int maIndex2 = 0;
int maIndex3 = 0;

int total0 = 0;
int total1 = 0;
int total2 = 0;
int total3 = 0;
//-----------------------------------------------

RF24 radio(10, 9);   // nRF24L01 (CE, CSN)
const byte address[6] = "00001";

unsigned long lastReceiveTime = 0;
unsigned long currentTime = 0;

//gyroscope
const int MPU = 0x68; // MPU6050 I2C address
float AccX, AccY, AccZ;
float GyroX, GyroY, GyroZ;
float accAngleX, accAngleY, gyroAngleX, gyroAngleY;
float angleX, angleY;
float AccErrorX, AccErrorY, GyroErrorX, GyroErrorY;
//float elapsedTime, currentTime, previousTime;----------------------------------------------------Causes error if uncommented. Needs to be fixed

int c = 0;


struct Data_Package {
  byte j1PotX;
  byte j1PotY;
  byte j2PotX;
  byte j2PotY;
  byte tSwitch1;
};


Data_Package data;

void setup() {
  Serial.begin(9600);
  radio.begin();
  radio.openReadingPipe(0, address);
  radio.setAutoAck(false);
  radio.setDataRate(RF24_250KBPS);
  radio.setPALevel(RF24_PA_LOW);
  radio.startListening(); //  Set the module as receiver
  
  // Initialize interface to the MPU6050
  initialize_MPU6050();

  resetData();

  //Set initial conditions of voltage checker--------------------------------------
  // not really necessary
  pinMode(A0, INPUT);
  pinMode(A1, INPUT);
  pinMode(A2, INPUT);
  pinMode(A3, INPUT);
  
  memset(samples0, 0, MOVING_AVERAGE_COUNT * sizeof(unsigned int));
  memset(samples1, 0, MOVING_AVERAGE_COUNT * sizeof(unsigned int));
  memset(samples2, 0, MOVING_AVERAGE_COUNT * sizeof(unsigned int));
  memset(samples3, 0, MOVING_AVERAGE_COUNT * sizeof(unsigned int));
  
  // fill the moving average buffer before displaying anything, 
  // otherwise numbers will take a few seconds to settle
  for (int i = 0; i < MOVING_AVERAGE_COUNT; i++)
    sample();
}

char vbuf0[16];
char vbuf1[16];
char vbuf2[16];
char vbuf3[16];

unsigned long lastScreenUpdate = 0;

#define DO_MOVING_AVERAGE(which)\
  total##which -= samples##which[ maIndex##which ];\
  samples##which[ maIndex##which ] = analogRead(A##which);\
  total##which += samples##which[ maIndex##which ];\
  maIndex##which = (maIndex##which + 1) % MOVING_AVERAGE_COUNT;

void sample()
{  
  DO_MOVING_AVERAGE(0)
  DO_MOVING_AVERAGE(1)
  DO_MOVING_AVERAGE(2)
  DO_MOVING_AVERAGE(3)
}

void updateScreen() 
{
  unsigned long now = millis();
  if ( now - lastScreenUpdate < 100 )
    return;
    
  sample();
    
  float v0 = 5 * (total0 / (float)MOVING_AVERAGE_COUNT) / 1023.0f;
  float v1 = 5 * (total1 / (float)MOVING_AVERAGE_COUNT) / 1023.0f;
  float v2 = 5 * (total2 / (float)MOVING_AVERAGE_COUNT) / 1023.0f;
  float v3 = 5 * (total3 / (float)MOVING_AVERAGE_COUNT) / 1023.0f;
    
  // set these to your resistor actual values
  float r11 =  980.0;
  float r12 =  981.0;
  float r21 =  984.0;
  float r22 = 1963;
  float r31 =  980.0;
  float r32 = 2941;
    
  v0 *= 1;
  v1 *= (r11 + r12) / r11;
  v2 *= (r21 + r22) / r21;
  v3 *= (r31 + r32) / r31;

Serial.println(v0);
Serial.println(v1);
Serial.println(v2);
Serial.println(v3);
  
  // adjust to match your voltmeter results
  v0 *= 1.0048;
  v1 *= 1.0045;
  v2 *= 1.0044;
  v3 *= 1.0043;

//For testing purposes

  
  v3 -= v2;
  v2 -= v1;
  v1 -= v0;
    
  int whole0 = (int)v0;
  int whole1 = (int)v1;
  int whole2 = (int)v2;
  int whole3 = (int)v3;
  int fraction0 = (v0 * 100) - (whole0 * 100);
  int fraction1 = (v1 * 100) - (whole1 * 100);
  int fraction2 = (v2 * 100) - (whole2 * 100);
  int fraction3 = (v3 * 100) - (whole3 * 100);

   // For testing purposes

  //sprintf(vbuf0, "v0: %d.%02d", whole0, fraction0);
  //sprintf(vbuf1, "v1: %d.%02d", whole1, fraction1);
  //sprintf(vbuf2, "v2: %d.%02d", whole2, fraction2);
  //sprintf(vbuf3, "v3: %d.%02d", whole3, fraction3);

 

 // Serial.println(vbuf0);
 ///// Serial.println(vbuf1);
 // Serial.println(vbuf2);
 // Serial.println(vbuf3);
  Serial.println("--------------");
  
  lastScreenUpdate = millis();  
}
//-------------------------------------------------------------------






void resetData() {
  // Reset the values when there is no radio connection - Set initial default values
  data.j1PotX = 127;
  data.j1PotY = 127;
  data.j2PotX = 127;
  data.j2PotY = 127;
  data.tSwitch1 = 1;
}

void initialize_MPU6050() {
 // Wire.begin();                      // Initialize comunication----------------------------------------------------Causes error if uncommented. Needs to be fixed
  //Wire.beginTransmission(MPU);       // Start communication with MPU6050 // MPU=0x68
  //Wire.write(0x6B);                  // Talk to the register 6B
  //Wire.write(0x00);                  // Make reset - place a 0 into the 6B register
  //Wire.endTransmission(true);        //end the transmission
  // Configure Accelerometer
  //Wire.beginTransmission(MPU);
  //Wire.write(0x1C);                  //Talk to the ACCEL_CONFIG register
  //Wire.write(0x10);                  //Set the register bits as 00010000 (+/- 8g full scale range)
  //Wire.endTransmission(true);
  // Configure Gyro
  //Wire.beginTransmission(MPU);
  //Wire.write(0x1B);                   // Talk to the GYRO_CONFIG register (1B hex)
  //Wire.write(0x10);                   // Set the register bits as 00010000 (1000dps full scale)
  //Wire.endTransmission(true);
}


void calculate_IMU_error() {
  // We can call this funtion in the setup section to calculate the accelerometer and gury data error. From here we will get the error values used in the above equations printed on the Serial Monitor.
  // Note that we should place the IMU flat in order to get the proper values, so that we then can the correct values
  // Read accelerometer values 200 times
  while (c < 200) {
//    Wire.beginTransmission(MPU);
  //  Wire.write(0x3B);
    //Wire.endTransmission(false);
    //Wire.requestFrom(MPU, 6, true);
    //AccX = (Wire.read() << 8 | Wire.read()) / 4096.0 ;
    //AccY = (Wire.read() << 8 | Wire.read()) / 4096.0 ;
    //AccZ = (Wire.read() << 8 | Wire.read()) / 4096.0 ;
    // Sum all readings
    AccErrorX = AccErrorX + ((atan((AccY) / sqrt(pow((AccX), 2) + pow((AccZ), 2))) * 180 / PI));
    AccErrorY = AccErrorY + ((atan(-1 * (AccX) / sqrt(pow((AccY), 2) + pow((AccZ), 2))) * 180 / PI));
    c++;
  }
  //Divide the sum by 200 to get the error value
  AccErrorX = AccErrorX / 200;
  AccErrorY = AccErrorY / 200;
  c = 0;
  // Read gyro values 200 times
  while (c < 200) {
//    Wire.beginTransmission(MPU);----------------------------------------------------Causes error if uncommented. Needs to be fixed
  //  Wire.write(0x43);
    //Wire.endTransmission(false);
    //Wire.requestFrom(MPU, 4, true);
    //GyroX = Wire.read() << 8 | Wire.read();
    //GyroY = Wire.read() << 8 | Wire.read();
    // Sum all readings
    GyroErrorX = GyroErrorX + (GyroX / 32.8);
    GyroErrorY = GyroErrorY + (GyroY / 32.8);
    c++;
  }
  //Divide the sum by 200 to get the error value
  GyroErrorX = GyroErrorX / 200;
  GyroErrorY = GyroErrorY / 200;
  // Print the error values on the Serial Monitor
  Serial.print("AccErrorX: ");
  Serial.println(AccErrorX);
  Serial.print("AccErrorY: ");
  Serial.println(AccErrorY);
  Serial.print("GyroErrorX: ");
  Serial.println(GyroErrorX);
  Serial.print("GyroErrorY: ");
  Serial.println(GyroErrorY);
}



void read_IMU() {
  // === Read acceleromter data === //
//  Wire.beginTransmission(MPU);----------------------------------------------------Causes error if uncommented. Needs to be fixed
//  Wire.write(0x3B); // Start with register 0x3B (ACCEL_XOUT_H)
//  Wire.endTransmission(false);
//  Wire.requestFrom(MPU, 6, true); // Read 6 registers total, each axis value is stored in 2 registers
  //For a range of +-8g, we need to divide the raw values by 4096, according to the datasheet
//  AccX = (Wire.read() << 8 | Wire.read()) / 4096.0; // X-axis value----------------------------------------------------Causes error if uncommented. Needs to be fixed
 // AccY = (Wire.read() << 8 | Wire.read()) / 4096.0; // Y-axis value
 // AccZ = (Wire.read() << 8 | Wire.read()) / 4096.0; // Z-axis value

  // Calculating angle values using
  accAngleX = (atan(AccY / sqrt(pow(AccX, 2) + pow(AccZ, 2))) * 180 / PI) + 1.15; // AccErrorX ~(-1.15) See the calculate_IMU_error()custom function for more details
  accAngleY = (atan(-1 * AccX / sqrt(pow(AccY, 2) + pow(AccZ, 2))) * 180 / PI) - 0.52; // AccErrorX ~(0.5)

  // === Read gyro data === //
//  previousTime = currentTime;        // Previous time is stored before the actual time read----------------------------------------------------Causes error if uncommented. Needs to be fixed
  currentTime = millis();            // Current time actual time read
//  elapsedTime = (currentTime - previousTime) / 1000;   // Divide by 1000 to get seconds----------------------------------------------------Causes error if uncommented. Needs to be fixed
//  Wire.beginTransmission(MPU);----------------------------------------------------Causes error if uncommented. Needs to be fixed
//  Wire.write(0x43); // Gyro data first register address 0x43----------------------------------------------------Causes error if uncommented. Needs to be fixed
//  Wire.endTransmission(false);
  //Wire.requestFrom(MPU, 4, true); // Read 4 registers total, each axis value is stored in 2 registers
  //GyroX = (Wire.read() << 8 | Wire.read()) / 32.8; // For a 1000dps range we have to divide first the raw value by 32.8, according to the datasheet
  //GyroY = (Wire.read() << 8 | Wire.read()) / 32.8;
  //GyroX = GyroX + 1.85; //// GyroErrorX ~(-1.85)
  //GyroY = GyroY - 0.15; // GyroErrorY ~(0.15)
  // Currently the raw values are in degrees per seconds, deg/s, so we need to multiply by sendonds (s) to get the angle in degrees
  //gyroAngleX = GyroX * elapsedTime;
  //gyroAngleY = GyroY * elapsedTime;

  // Complementary filter - combine acceleromter and gyro angle values
  angleX = 0.98 * (angleX + gyroAngleX) + 0.02 * accAngleX;
  angleY = 0.98 * (angleY + gyroAngleY) + 0.02 * accAngleY;
  
  Serial.println(angleX);
  Serial.println(angleY);
  
  if(data.j2PotX == 127 && data.j2PotY == 127 && data.j1PotX == 127 && data.j1PotY == 127){
    
  }
  
  
  // Map the angle values from -90deg to +90 deg into values from 0 to 255, like the values we are getting from the Joystick
  data.j1PotX = map(angleX, -90, +90, 255, 0);
  data.j1PotY = map(angleY, -90, +90, 0, 255);
  
}







void loop() {
  // Check whether there is data to be received
  if (radio.available()) {
    radio.read(&data, sizeof(Data_Package)); // Read the whole data and store it into the 'data' structure
    lastReceiveTime = millis(); // At this moment we have received the data
  }
  // Check whether we keep receving data, or we have a connection between the two modules
  currentTime = millis();
  if ( currentTime - lastReceiveTime > 1000 ) { // If current time is more then 1 second since we have recived the last data, that means we have lost connection
    resetData(); // If connection is lost, reset the data. It prevents unwanted behavior, for example if a drone has a throttle up and we lose connection, it can keep flying unless we reset the values
  }
  // Print the data in the Serial Monitor
  //Serial.print("j1PotX: ");
  //Serial.print(data.j1PotX);
  //Serial.print("; j1PotY: ");
  //Serial.print(data.j1PotY);
  //Serial.print("; j2PotX: ");
  //Serial.println(data.j2PotX);
  //Serial.print("; j2PotY: ");
  //Serial.println(data.j2PotY);  
  //Serial.print("; tSwitch1: ");
  //Serial.print(data.tSwitch1);

  updateScreen();
  

}
