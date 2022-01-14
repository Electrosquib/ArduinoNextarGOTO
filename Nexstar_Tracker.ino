//#include <MPU6050_tockn.h>
#include <Wire.h>

//MPU6050 mpu6050(Wire);

const int serial_baud = 9600;
const int EL_TACH = 1; // Pin 3
const int AZ_TACH = 0; // Pin 2
const int AZ_IN_1 = 5;
const int AZ_IN_2 = 6;
const int EL_IN_1 = 7;
const int EL_IN_2 = 8;
const int AZ_EN = 9;
const int EL_EN = 12;
const float ppd = 2939.0;

int motor_speed = 2; // Integer from 1-4 indicating Azimuth speed
String inputString = "";
boolean stringComplete = false;
float AZ_rotation_degrees;
float EL_rotation_degrees;
float AZ_angle;
float EL_angle;
int azState = 0; // -1: Left, 0: Neutral, 1: Right
int elState = 0; // -1: Down, 0: Neutral, 1: Up
volatile long azPulseCount;
volatile long elPulseCount;

void setup() {
  Serial.begin(serial_baud);
  motor_speed = map(motor_speed, 0, 4, 0, 1023);
  pinMode(AZ_IN_1, OUTPUT);
  pinMode(AZ_IN_2, OUTPUT);
  pinMode(EL_IN_1, OUTPUT);
  pinMode(EL_IN_2, OUTPUT);
  pinMode(AZ_EN, OUTPUT);
  pinMode(EL_EN, OUTPUT);
  digitalWrite(AZ_EN, HIGH);
  digitalWrite(EL_EN, HIGH);
  inputString.reserve(200);
  Wire.begin();
 // mpu6050.begin();
  //mpu6050.calcGyroOffsets(true);
  attachInterrupt(AZ_TACH, incrementAz, RISING);
  attachInterrupt(EL_TACH, incrementEl, RISING);
  help();
}

void serialEvent() {
  while (Serial.available()) {
    // get the new byte:
    char inChar = (char)Serial.read(); 
    // add it to the inputString:
    inputString += inChar;
    // if the incoming character is a newline, set a flag
    // so the main loop can do something about it:
    if (inChar == '\n') {
      stringComplete = true;
    } 
  }
}

void Rotate_Up() {
  elState = 1;
  digitalWrite(EL_IN_1, LOW);
  digitalWrite(EL_IN_2, HIGH);
  digitalWrite(EL_EN, HIGH);
}

void Rotate_Down() {
  elState = -1;
  digitalWrite(EL_IN_1, HIGH);
  digitalWrite(EL_IN_2, LOW);
  digitalWrite(EL_EN, HIGH);
}

void Rotate_Right() {
  azState = 1;
  Serial.println(motor_speed);
  digitalWrite(AZ_IN_1, HIGH);
  digitalWrite(AZ_IN_2, LOW);
  digitalWrite(AZ_EN, HIGH);
}

void Rotate_Left() {
  azState = -1;
  digitalWrite(AZ_IN_1, LOW);
  digitalWrite(AZ_IN_2, HIGH);
  digitalWrite(AZ_EN, HIGH);
}

void Stop_Az() {
  digitalWrite(AZ_IN_1, LOW);
  digitalWrite(AZ_IN_2, LOW);
}

void Stop_El() {
  digitalWrite(EL_IN_1, LOW);
  digitalWrite(EL_IN_2, LOW);

}

void help() {
  Serial.println("More Info: https://www.jzgelectronics.com/gs-232b-command-reference/");
  Serial.println("B - Report Elevation");
  Serial.println("C - Report Azimuth");
  Serial.println("C2 - Report Azimuth and Elevation");
  Serial.println("S - All Stop");
  Serial.println("A - CW/CCW Rotation Stop (Azimuth)");
  Serial.println("E - UP/DOWN Rotation Stop (Elevation)");
  Serial.println("L - Counter Clockwise Rotation");
  Serial.println("R - Clockwise Rotation");
  Serial.println("D - Down Rotation");
  Serial.println("U - Up Rotation");
  Serial.println("Mxxx - Move to Azimuth (Mxxx)");
  Serial.println("Wxxx yyy - Antenna AZ/EL Direction Setting (WXXX YYY)");
  Serial.println("X - Set Azimuth Rotation Speed (X1 - X4)");
  Serial.println("F - Calibrate. Input values are the current Alt/Az angle offset (Read on phone). Format: Z75 231");
  Serial.println("Z - Zero out current angle measurement");
  Serial.println("H - Display Help");
}

void get_azel() {
  //mpu6050.update();
  //AZ_angle = mpu6050.getAngleX();
  //EL_angle = mpu6050.getAngleY();
  //Serial.println(AZ_angle+EL_angle);
  AZ_angle = float(azPulseCount)/ppd;
  EL_angle = float(elPulseCount)/ppd;
  Serial.println(AZ_angle);
}

void rotate_to(int az_degrees, int el_degrees) {
  get_azel();
  while (AZ_angle > az_degrees) {
    Rotate_Left();
    get_azel();
  }
  while (AZ_angle < az_degrees) {
    Rotate_Right();
    get_azel();
  }
  while (EL_angle < el_degrees) {
    Rotate_Up();
    get_azel();
  }
  while (EL_angle > el_degrees) {
    Rotate_Down();
    get_azel();
  }
  Stop_Az();
  Stop_El();
}

void parseGS232() {
  if (stringComplete) {
    Serial.println(inputString); 

    if (inputString.startsWith("B")) {
      // Report Elevation
      get_azel();
      Serial.print("+0");
      if(EL_angle < 100) Serial.print("0");
      if(EL_angle < 10) Serial.print("0");
      Serial.println(EL_angle, 0);
    } 
    else if (inputString.startsWith("C")) {
      // Report Azimuth
      get_azel();
      Serial.print("+0");
      if(AZ_angle < 100) Serial.print("0");
      if(AZ_angle < 10) Serial.print("0");
      Serial.println(AZ_angle, 0);
    } 
    else if (inputString.startsWith("C2")) {
      get_azel();
      Serial.print("+0");
      if(AZ_angle < 100) Serial.print("0");
      if(AZ_angle < 10) Serial.print("0");
      Serial.print(AZ_angle, 0);
      Serial.print("+0");
      if(EL_angle < 100) Serial.print("0");
      if(EL_angle < 10) Serial.print("0");
      Serial.println(EL_angle, 0);
    } 
    else if (inputString.startsWith("S")) {
      Stop_El();
      Stop_Az();
    } 
    else if (inputString.startsWith("A")) {
      Stop_Az();
    }
    else if (inputString.startsWith("E")) {
      Stop_El();
    } 
    else if (inputString.startsWith("L")) {
      Rotate_Left();
    } 
    else if (inputString.startsWith("R")) {
      Rotate_Right();
    } 
    else if (inputString.startsWith("D")) {
      Rotate_Down();
    } 
    else if (inputString.startsWith("U")) {
      Rotate_Up();
    } 
    else if (inputString.startsWith("M")) {
      // Set Azimuth Direction
      AZ_rotation_degrees = inputString.substring(1).toInt(); // Parse rotation degrees from input
      rotate_to(AZ_rotation_degrees, EL_angle);
    }
    else if (inputString.startsWith("F")) {
      AZ_rotation_degrees = inputString.substring(1, 4).toInt(); // Parse AZ rotation degrees from input
      EL_rotation_degrees = inputString.substring(4).toInt(); // Parse EL rotation degrees from input
      rotate_to(AZ_angle-AZ_rotation_degrees, EL_angle-EL_rotation_degrees);
      azPulseCount = 0;
      elPulseCount = 0;
    }
    else if (inputString.startsWith("W")) {
      // Set AZ/EL Direction Wxxx yyy
      AZ_rotation_degrees = inputString.substring(1, 4).toInt(); // Parse AZ rotation degrees from input
      EL_rotation_degrees = inputString.substring(4).toInt(); // Parse EL rotation degrees from input
      rotate_to(AZ_rotation_degrees, EL_rotation_degrees);
    }
    else if (inputString.startsWith("X")) {
      // Set azimuth motor speed (X1 - X4)
      motor_speed = inputString.substring(1).toInt();
    } 
    else if(inputString.startsWith("H")) {
      // Print Help
      help();
    }
    
    else {
      Serial.println("<< Unknown Command >>");
    }

    Serial.println("\r");
    
    // clear the string:
    inputString = "";
    stringComplete = false;
    Serial.print(">");
  }
}

void incrementAz() {
  if (azState >= 0) {
    azPulseCount += 1;
  }
  else {
    azPulseCount -= 1;
  }
}

void incrementEl() {
  if (elState == 1) {
    elPulseCount += 1;
  }
  else if (elState == -1) {
    elPulseCount -= 1;
  }
}

void loop() {
  get_azel();
  motor_speed = map(motor_speed, 0, 4, 0, 1023);
  parseGS232();
  delay(1000);
}
