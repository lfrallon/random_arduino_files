#include <Servo.h>
#include <SPI.h>


Servo motorl1;
Servo motorr1;

#define SPEEDSTEPS 75
#define SPEEDSTEPDISTANCE 2


const int motorl1_pin = 3;
const int motorr1_pin = 2;

const float motorl1_corr = 0.5;
const float motorr1_corr = 0.5;
unsigned long now_time;

const int topLeft = A2;
const int bottomLeft = A3;
const int topRight = A0;
const int bottomRight = A1;

const int bumper_frontRight = 8;
const int bumper_frontLeft = 9;
const int bumper_backRight = 6;
const int bumper_backLeft = 5;
const int bumper_sideRight = 7;
const int bumper_sideLeft = 4;

int lightThresholdValue = 0;

//These variables are needed to be able to ramp the speed up/down slowly
int targetSpeed = 0; //Used for static instructions to set what speed we want to reach.
int targetDistance = 0; //Used for static instruction to set what distance we eventually want to drive
bool changingSpeed = false; //Are we slowly changing the speed up/down?


int lineSensorDistance = 150; //the distance between the line sensors and the turning point of the rover (middle between the tracks) in mm

const int travel_speed = 17142; //The speed in mm/min that the rover drives at full speed (255)
const int turnspeed = 50; //The speed of turning.
const int turntime = 2500; //The time in ms the rover needs to turn 90° at speed turnspeed


//Drive state variables
unsigned long stop_at_time = 0; //The time in ms at which we should stop driving (reached the distance of our instruction)
bool driving = false; //Are we currently driving?
int currentSpeed = 0; //What is the current speed?
bool turning = false; //Are we currently turning?
bool backtracking = false; //Are we backtracking (because we lost the line)?
int rightSearchCount = 0; //The amount we have been searching on the right for the line
int leftSearchCount = 0; //The amount we have been searching on the left for the line
//
//const int leftLineSensorPin = A0;
//const int centerLineSensorPin = A1;
//const int rightLineSensorPin = A2;

//Driving functions
void setSpeed(Servo motor, int speed) {
  //Sets the speed for a motor

  //Map the speed (-255 (full reverse) to 255 (full forward)) to the servo/motor duty cycle.
  //1000µs is full reverse, 1500µs is neutral, 2000µs is full forward.
  //See https://learn.sparkfun.com/blog/1593
  int val = map(speed, -255, 255, 1000, 2000);
  motor.write(val);
}

void setDriveSpeed(int rightSpeed, int leftSpeed) {
  //Set the speed of each individual motor
  /*  Serial.print("Speed l1: ");
    Serial.print(leftSpeed*motorl1_corr);
    Serial.print(" l2: ");
    Serial.print(leftSpeed*motorl2_corr);
    Serial.print(" r1: ");
    Serial.print(rightSpeed*motorr1_corr);
    Serial.print(" r2: ");
    Serial.println(rightSpeed*motorr2_corr);
  */
  setSpeed(motorl1, leftSpeed * motorl1_corr);
  setSpeed(motorr1, rightSpeed * motorr1_corr);
}


void stopDriving() {
  //Stop all motors
  //Serial.println("Stopping driving");
  setDriveSpeed(0, 0);
  driving = false;
  turning = false;
  backtracking = false;

}

void drive(int distance, int speed) {
  //Start moving. Distance in mm and speed between -255(full backward) and 255 (full forward).
  if (abs(currentSpeed - speed) >= SPEEDSTEPS * 2) {
    //The speed difference between the current speed and new speed is too large.
    //Instead, step up/down slowly
    Serial.println("Stepping speed slowly");
    changingSpeed = true;
    if (speed < currentSpeed) {
      speed = currentSpeed - SPEEDSTEPS;
    } else {
      speed = currentSpeed + SPEEDSTEPS;
    }
    distance = SPEEDSTEPDISTANCE;
  }
  else {
    changingSpeed = false;
  }
  Serial.print("Driving ");
  Serial.print(distance);
  Serial.print("mm ");
  Serial.print(speed > 0 ? "forward" : "backward");
  Serial.print(" at speed ");
  Serial.println(abs(speed));
  int duration = calcTime(distance, abs(speed));
  now_time = millis();
  turning = false;
  driving = true;
  currentSpeed = speed;
  stop_at_time = now_time + duration;
  setDriveSpeed(speed, -speed);
}

void turn(float angle) {
  //Turn left or right. angle<0 means turn left, ange>0 means turn right
  turning = true;
  driving = true;
  now_time = millis();
  currentSpeed = 0;
  Serial.print("Turning ");
  Serial.print(angle);
  Serial.print(" at speed ");
  Serial.println(turnspeed);
  int factor = angle < 0 ? -1 : 1;
  stop_at_time = now_time + turntime * abs(angle) / 90.0;
  setDriveSpeed(factor * turnspeed, factor * turnspeed);
}


int calcTime(int distance, int rel_speed) {
  //Calculates the time in milliseconds the rover needs to drive <distance> mm at <speed> (0-255)
  float abs_speed = rel_speed / 255.0 * travel_speed; //the real speed in mm/min we'll drive
  return (distance / abs_speed) * 60000; //the time in ms we need to drive
}


void checkLineDriveDirection(){
  //Constantly check if the line is under the middle sensor. If not, correct.
//  if(reachedGoal || turning){
//    return;
//  }
  int topleftVal = analogRead(topLeft); //Value for left line sensor
  int toprightVal = analogRead(topRight); //Value for right line sensor
  int bottomrightVal = analogRead(bottomRight);
  int bottomleftVal = analogRead(bottomLeft);

  Serial.print("lightThresholdValue : ");
  Serial.print(lightThresholdValue);
  Serial.print("  Top Left Value: ");
  Serial.print(topleftVal);
  Serial.print("  Top Right Value: ");
  Serial.print(toprightVal);
  Serial.print("  Bottom Left Value: : ");
  Serial.println(bottomleftVal);
  Serial.print("  Bottom Right Value: : ");
  Serial.println(bottomrightVal);


  if (topleftVal != 1023 || toprightVal != 1023 || bottomrightVal != 1023 || bottomleftVal != 1023){
    Serial.println("DETECTED");
    driving = false;    
    if (topleftVal != 1023 && toprightVal < 1023 && bottomrightVal < 1023 && bottomleftVal < 1023){
     setDriveSpeed(255, 300);
     delay(3000);
    }
  } else {
    driving = true;  
    }


//    if topleftVal ==
  
}



void setup() {
  //Start serial port
  Serial.begin(115200);

  //Attach motors
  motorl1.attach(motorl1_pin);
  motorr1.attach(motorr1_pin);
  driving = true;

  pinMode(bumper_frontRight, INPUT);
  pinMode(bumper_frontLeft, INPUT);
  pinMode(bumper_backRight, INPUT);
  pinMode(bumper_backLeft, INPUT);
  pinMode(bumper_sideRight, INPUT);
  pinMode(bumper_sideLeft, INPUT);

}

void loop() {
  Serial.println(driving);
  
   checkLineDriveDirection();
   if(driving == true){
       drive(20, 255);
   } else {
    stopDriving();
   }
  
  // put your main code here, to run repeatedly:

}

