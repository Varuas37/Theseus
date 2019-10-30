  #include <PS4BT.h>
#include <usbhub.h>

// Satisfy the IDE, which needs to see the include statment in the ino too.
#ifdef dobogusinclude
#include <spi4teensy3.h>
#endif
#include <SPI.h>

const int DEADZONE = 13;
const int STICK_CENTER = 127;

const int PIN_FRONT_LEFT_DIR  = 24;
const int PIN_FRONT_RIGHT_DIR = 28;
const int PIN_BACK_LEFT_DIR   = 22;
const int PIN_BACK_RIGHT_DIR  = 26;
const int PIN_FRONT_LEFT_PWM  = 3;
const int PIN_FRONT_RIGHT_PWM = 4;
const int PIN_BACK_LEFT_PWM   = 2;
const int PIN_BACK_RIGHT_PWM  = 5;

const int SERVO_STOP = 95;

const int ARM_PIN = 9;
const int CLAW_PIN = 10;

const int WALK_SPEED = 64;
const int RUN_SPEED = 127;

const int WALK_COLOR[3] = {0,0,255};
const int RUN_COLOR[3] = {255,0,0};


bool runMode = true;
int currentSpeed = WALK_SPEED;

Servo arm;
Servo claw;

// Magic bluetooth setup stuff
USB Usb;
//USBHub Hub1(&Usb); // Some dongles have a hub inside
BTD Btd(&Usb); // You have to create the Bluetooth Dongle instance like so

/* You can create the instance of the PS4BT class in two ways */
// This will start an inquiry and then pair with the PS4 controller - you only have to do this once
// You will need to hold down the PS and Share button at the same time, the PS4 controller will then start to blink rapidly indicating that it is in pairing mode
PS4BT PS4(&Btd, PAIR);

// After that you can simply create the instance like so and then press the PS button on the device
//PS4BT PS4(&Btd);

bool printAngle, printTouch;
uint8_t oldL2Value, oldR2Value;

int[2] unoMessage = {0, 0};

void setup() 
{
  
  if (Usb.Init() == -1) 
  {
    while (1); // Halt
  }

  // Put all pins we care about into the right mode)
  pinMode(PIN_FRONT_LEFT_DIR, OUTPUT);
  pinMode(PIN_BACK_LEFT_DIR, OUTPUT);
  pinMode(PIN_FRONT_RIGHT_DIR, OUTPUT);
  pinMode(PIN_BACK_RIGHT_DIR, OUTPUT);
  pinMode(PIN_FRONT_LEFT_PWM, OUTPUT);
  pinMode(PIN_BACK_LEFT_PWM, OUTPUT);
  pinMode(PIN_FRONT_RIGHT_PWM, OUTPUT);
  pinMode(PIN_BACK_RIGHT_PWM, OUTPUT);

  // Setup servo controllers
 // pinMode(SERVO, OUTPUT);
 //pinMode(ARM_PIN, OUTPUT);
 //pinMode(CLAW_PIN, OUTPUT);n
 // raise.attach(SERVO);
 arm.attach(ARM_PIN);
 claw.attach(CLAW_PIN);
 arm.write(SERVO_STOP);
 claw.write(SERVO_STOP);

  // Setup swag
  PS4.setLed(WALK_COLOR[0], WALK_COLOR[1], WALK_COLOR[2]);


}

void loop() 
{
  Serial.begin(9600);
  
  Usb.Task();
  if (PS4.connected()) 
  {

    if (PS4.getButtonClick(OPTIONS))
    {
      while(1){}; // Halt
    }

    if (PS4.getButtonClick(TOUCHPAD))
    {
      moveRobot(true, WALK_SPEED, true, WALK_SPEED, true, WALK_SPEED, true, WALK_SPEED);
      delay(3250);
    }
    
    if(PS4.getButtonClick(SHARE))
    {
      if (runMode)
      {
        runMode = false;
        PS4.setLed(WALK_COLOR[0], WALK_COLOR[1], WALK_COLOR[2]);
        currentSpeed = WALK_SPEED;
      }

      else
      {
        runMode = true;
        PS4.setLed(RUN_COLOR[0], RUN_COLOR[1], RUN_COLOR[2]);
        currentSpeed = RUN_SPEED;
      }
    }
    
    bool forward = PS4.getButtonPress(UP);
    bool backward = PS4.getButtonPress(DOWN);
    bool strafeLeft = PS4.getButtonPress(LEFT);
    bool strafeRight = PS4.getButtonPress(RIGHT);
    bool turnLeft = PS4.getButtonPress(CIRCLE);
    bool turnRight = PS4.getButtonPress(SQUARE);
    bool armUp = PS4.getButtonPress(R1);
    bool armDown = PS4.getButtonPress(L1);
    bool clawOpen = PS4.getButtonPress(TRIANGLE);
    bool clawClose = PS4.getButtonPress(CROSS);

    
    if (armUp)
    {
      unoMessage[0] = 1;
    }

    else if (armDown)
    {
      unoMessage[0] = 2;
    }

    else
    {
      unoMessage[0] = 0;
    }

    if (clawOpen)
    {
      unoMessage[1] = 1;
    }

    else if(clawClose)
    {
      unoMessage[1] = 2;
    }

    else
    {
      unoMessage[1] = 0;
    }

    Serial.write(unoMessage, 2);
    
    if (forward && !turnLeft && !turnRight)
    {
      moveRobot(true, currentSpeed, true, currentSpeed, true, currentSpeed, true, currentSpeed);
    }

    else if(forward && turnLeft)
    {
      moveRobot(true, RUN_SPEED, true, RUN_SPEED, false, 0, false, 0);
    }

    else if(forward && turnRight)
    {
      moveRobot(false, 0, false, 0, true, RUN_SPEED, true, RUN_SPEED);
    }

    else if(strafeLeft)
    {
      moveRobot(false, currentSpeed, true, currentSpeed, true, currentSpeed, false, currentSpeed);
    }

    else if(strafeRight)
    {
      moveRobot(true, currentSpeed, false, currentSpeed, false, currentSpeed, true, currentSpeed);
    }

    else if(backward)
    {
      moveRobot(false, currentSpeed, false, currentSpeed, false, currentSpeed, false, currentSpeed);
    }

    else if(turnLeft)
    {
      moveRobot(true, currentSpeed, true, currentSpeed, false, currentSpeed, false, currentSpeed);
    }

    else if(turnRight)
    {
      moveRobot(false, currentSpeed,  false, currentSpeed, true, currentSpeed, true, currentSpeed);
    }
    else
    {
      moveRobot(false, 0, false, 0, false, 0, false, 0);
    }
  }
}

// Helper function to check state of analog stick in regards to deadzone
// Value of 0 means it is out of the deadzone, in the negative direction
// Value of 2 means it is out of the deadzone, in the positive direction
// value of 1 means it is in the deadzone
int checkInDeadzone(int value)
{
  if (value < STICK_CENTER - DEADZONE)
  {
    return 0;
  }

  else if (value > STICK_CENTER + DEADZONE)
  {
    return 2;
  }

  else
  {
    return 1;
  }
}

// Clamps value between minValue and maxValue
// Has no effect on values between the two
int clampInt(int value, int minValue, int maxValue)
{
  if (value < minValue)
  {
    return minValue;
  }

  else if (value > maxValue)
  {
    return maxValue;
  }

  else
  {
    return value;
  }
}

void moveRobot(boolean dirFrontL, int powerFrontL, boolean dirBackL, int powerBackL, boolean dirFrontR, int powerFrontR, boolean dirBackR, int powerBackR)
{
  bool FL;
  bool BL;
  bool FR;
  bool BR;

  if (dirFrontL) { FL = LOW; }
  else { FL = HIGH; }

  if (dirBackL) { BL = LOW; }
  else { BL = HIGH; }

  if (dirFrontR) { FR = HIGH; }
  else { FR = LOW; }

  if (dirBackR) { BR = HIGH; }
  else { BR = LOW; }

  
  digitalWrite(PIN_FRONT_LEFT_DIR, FL);
  digitalWrite(PIN_BACK_LEFT_DIR, BL);
  digitalWrite(PIN_FRONT_RIGHT_DIR, FR);
  digitalWrite(PIN_BACK_RIGHT_DIR, BR);

  int normalizedFrontL = clampInt(powerFrontL * 2, 0, 255);
  int normalizedBackL = clampInt(powerBackL * 2, 0, 255);
  int normalizedFrontR = clampInt(powerFrontR * 2, 0, 255);
  int normalizedBackR = clampInt(powerBackR * 2, 0, 255);
  
  analogWrite(PIN_FRONT_LEFT_PWM, normalizedFrontL);
  analogWrite(PIN_BACK_LEFT_PWM, normalizedBackL);
  analogWrite(PIN_FRONT_RIGHT_PWM, normalizedFrontR);
  analogWrite(PIN_BACK_RIGHT_PWM, normalizedBackR);
}
