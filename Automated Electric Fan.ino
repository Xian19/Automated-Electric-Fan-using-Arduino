#include <dht.h>
#include <Servo.h>

//DHT Variables
const int DHT11_PIN = 5;
dht DHT;
float dhtTemp = 0;
float dhtHum = 0;

unsigned long previousTempTime = 0;
const int tempInterval = 3000; // Time interval for temperature check (in milliseconds)


//Ultrasonic Variavles
const int trigPin = 9;
const int echoPin = 10;
long duration;
int distance;

//Servo Variables
Servo myservo;
bool fanSweep = false;
bool ledSweep = false;
int pos = 0;
const int servoPin = 11;

unsigned long previousServoTime = 0;
const int sweepInterval = 35; // Time interval between each position change (in milliseconds)

//display Variables
const int dataPin = 4;   // blue wire to 74HC595 pin 14
const int latchPin = 3;  // green to 74HC595 pin 12
const int clockPin = 2;  // yellow to 74HC595 pin 11
//const char common = 'a';    // common anode
const char common = 'c';  // common cathode
bool decPt = false;       // decimal point display flag
int fanMode = 0;

//Fan Variables
const int fan_control_PIN = 6;
const int relay_PIN = A5;
int prevMode = 0;

//Button Variables
const int offButton = 7;
const int lowButton = 8;
const int medButton = 12;
const int highButton = 13;
const int sweepButton = A4;
const int autoButton = 1;
bool isOff = false;
bool isLow = false;
bool isMed = false;
bool isHigh = false;
bool isSweep = false;
bool isAuto = false;



void setup() {
  Serial.begin(9600);
  pinMode(dataPin, OUTPUT);
  pinMode(latchPin, OUTPUT);
  pinMode(clockPin, OUTPUT);
  pinMode(trigPin, OUTPUT);  // Sets the trigPin as an Output
  pinMode(echoPin, INPUT);   // Sets the echoPin as an Input
  pinMode(offButton, INPUT);
  pinMode(lowButton, INPUT);
  pinMode(medButton, INPUT);
  pinMode(highButton, INPUT_PULLUP);
  pinMode(sweepButton, INPUT);
  pinMode(autoButton, INPUT);
  myservo.attach(servoPin);
  myservo.write(90);
  pinMode(fan_control_PIN, OUTPUT);
  pinMode(relay_PIN, OUTPUT);
  analogWrite(fan_control_PIN, 0);
  fanMode = 0;
}

void loop() {
  unsigned long currentMillis = millis();
  
  if (currentMillis - previousTempTime >= tempInterval) {
    previousTempTime = currentMillis;
    tempCheck();
  }

  sonarSense();
  buttonKeys();
  displayShift();
  servoSweep();
  fanSpeedMode();
}


void tempCheck() {
  int chk = DHT.read11(DHT11_PIN);
  Serial.print("Temperature = ");
  dhtTemp = DHT.temperature;
  Serial.println(dhtTemp);
  Serial.print("Humidity = ");
  dhtHum = DHT.humidity;
  Serial.println(dhtHum);
}

void servoSweep() {
  if (fanSweep) {
    unsigned long currentMillis = millis();
    if (currentMillis - previousServoTime >= sweepInterval) {
      previousServoTime = currentMillis;

      
      static int pos = 20;
      static bool increasing = true;
      
      // Increment or decrement the position
      if (increasing) {
        pos += 5;
        if (pos >= 160) {
          increasing = false;
        }
      } else {
        pos -= 5;
        if (pos <= 20) {
          increasing = true;
        }
      }

      myservo.write(pos); // Set the servo position
    }
  }
}


void displayShift() {

  byte bits = myfnNumToBits(fanMode);
  myfnUpdateDisplay(bits);
}

void buttonKeys() {
  isOff = digitalRead(offButton);
  if (isOff == HIGH && isLow == LOW && isMed == LOW && isHigh == HIGH && isSweep == LOW && isAuto == LOW) {
    Serial.println("OFF");
    prevMode = fanMode;
    fanMode = 0;
    isOff = false;
  }
  isLow = digitalRead(lowButton);
  if (isLow == HIGH && isMed == LOW && isHigh == HIGH && isSweep == LOW && isAuto == LOW) {
    Serial.println("LOW");
    prevMode = fanMode;
    fanMode = 1;
    isLow = false;
  }
  isMed = digitalRead(medButton);
  if (isLow == LOW && isMed == HIGH && isHigh == HIGH && isSweep == LOW && isAuto == LOW) {
    Serial.println("MED");
    prevMode = fanMode;
    fanMode = 2;
    isMed = false;
  }
  isHigh = digitalRead(highButton);
  if (isLow == LOW && isMed == LOW && isHigh == LOW && isSweep == LOW && isAuto == LOW) {
    Serial.println("HIGH");
    prevMode = fanMode;
    fanMode = 3;
    isHigh = false;
  }
  isSweep = digitalRead(sweepButton);
  if (isLow == LOW && isMed == LOW && isHigh == HIGH && isSweep == HIGH && isAuto == LOW) {  // Adjust the threshold value (900) based on your button's behavior
    Serial.println("SWEEP");
    prevMode = fanMode;
    fanMode = 4;
    fanMode = prevMode;
    if (fanSweep == true) {
      fanSweep = false;
    }
    else {
      fanSweep = true;
    }
  }
  isAuto = digitalRead(autoButton);
  if (isLow == LOW && isMed == LOW && isHigh == HIGH && isSweep == LOW && isAuto == HIGH) {  // Adjust the threshold value (900) based on your button's behavior
    Serial.println("AUTO");
    prevMode = fanMode;
    fanMode = 5;
  }
}

void myfnUpdateDisplay(byte eightBits) {
  if (common == 'a') {                  // using a common anonde display?
    eightBits = eightBits ^ B11111111;  // then flip all bits using XOR
  }
  digitalWrite(latchPin, LOW);                       // prepare shift register for data
  shiftOut(dataPin, clockPin, LSBFIRST, eightBits);  // send data
  digitalWrite(latchPin, HIGH);                      // update display
}

byte myfnNumToBits(int someNumber) {
  switch (someNumber) {
    case 0:
      return B11111100;
      break;
    case 1:
      return B01100000;
      break;
    case 2:
      return B11011010;
      break;
    case 3:
      return B11110010;
      break;
    case 4:
      fanSweep = !fanSweep;
      ledSweep = !ledSweep;
      break;
    case 5:
      return B11101110;  // Hexidecimal A
      break;
  }
}

void fanSpeedMode() {
  switch (fanMode) {
    case 0:
      analogWrite(relay_PIN, 0);
      analogWrite(fan_control_PIN, 0);
      break;
    case 1:
      analogWrite(relay_PIN, 1055);
      analogWrite(fan_control_PIN, 15);
      break;
    case 2:
      analogWrite(relay_PIN, 1055);
      analogWrite(fan_control_PIN, 140);
      break;
    case 3:
      analogWrite(relay_PIN, 1055);
      analogWrite(fan_control_PIN, 255);
      break;
    case 5:
      if (dhtTemp <= 25) {
        analogWrite(relay_PIN, 1055);
        analogWrite(fan_control_PIN, 0);
      } else if (dhtTemp >= 26 && dhtTemp <= 29) {
        analogWrite(relay_PIN, 1055);
        analogWrite(fan_control_PIN, 15);
      } else if (dhtTemp >= 30 && dhtTemp <= 33) {
        analogWrite(relay_PIN, 1055);
        analogWrite(fan_control_PIN, 105);
      } else {
        analogWrite(relay_PIN, 1055);
        analogWrite(fan_control_PIN, 255);
      }
      break;
  }
  Serial.println(fanMode);
}

void sonarSense() {
  // Clears the trigPin
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  // Sets the trigPin on HIGH state for 10 micro seconds
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  // Reads the echoPin, returns the sound wave travel time in microseconds
  duration = pulseIn(echoPin, HIGH);
  // Calculating the distance
  distance = duration * 0.034 / 2;
  delay(200);
  // Prints the distance on the Serial Monitor
  Serial.print("Distance: ");
  Serial.println(distance);
  if (distance >= 1190) {
    fanMode = 0;
  }
}