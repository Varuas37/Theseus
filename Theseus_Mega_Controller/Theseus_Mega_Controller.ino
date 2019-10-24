#include <PS4BT.h>
#include <usbhub.h>

// Satisfy the IDE, which needs to see the include statment in the ino too.
#ifdef dobogusinclude
#include <spi4teensy3.h>
#endif
#include <SPI.h>

#define DEADZONE 13
#define STICK_CENTER 127

#define PIN_FRONT_LEFT_DIR  26
#define PIN_FRONT_RIGHT_DIR 22
#define PIN_BACK_LEFT_DIR   28
#define PIN_BACK_RIGHT_DIR  24
#define PIN_FRONT_LEFT_PWM  2
#define PIN_FRONT_RIGHT_PWM 3
#define PIN_BACK_LEFT_PWM   4
#define PIN_BACK_RIGHT_PWM  5

#define DIGITAL_SPEED 127

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
// Default to digital mode
bool analogMode = false;

void setup() 
{
  //More magic bluetooth stuff
  Serial.begin(115200);
#if !defined(__MIPSEL__)
  while (!Serial); // Wait for serial port to connect - used on Leonardo, Teensy and other boards with built-in USB CDC serial connection
#endif
  if (Usb.Init() == -1) 
  {
    Serial.print(F("\r\nOSC did not start"));
    while (1); // Halt
  }
  Serial.print(F("\r\nPS4 Bluetooth Library Started"));

  // Put all pins we care about into the right mode)
  pinMode(PIN_FRONT_LEFT_DIR, OUTPUT);
  pinMode(PIN_BACK_LEFT_DIR, OUTPUT);
  pinMode(PIN_FRONT_RIGHT_DIR, OUTPUT);
  pinMode(PIN_BACK_RIGHT_DIR, OUTPUT);
  pinMode(PIN_FRONT_LEFT_PWM, OUTPUT);
  pinMode(PIN_BACK_LEFT_PWM, OUTPUT);
  pinMode(PIN_FRONT_RIGHT_PWM, OUTPUT);
  pinMode(PIN_BACK_RIGHT_PWM, OUTPUT);
}

void loop() 
{
  Usb.Task();
  if (PS4.connected()) 
  {
    if(PS4.getButtonClick(OPTIONS))
    {
      analogMode = !analogMode;
    }

    if(analogMode)
    {
      int leftStickX = PS4.getAnalogHat(LeftHatX);
      int leftStickY = PS4.getAnalogHat(LeftHatY);
      int rightStickX = PS4.getAnalogHat(RightHatX);
      int rightStickY = PS4.getAnalogHat(RightHatY);
  
      int leftDeadzoneX = checkInDeadzone(leftStickX);
      int leftDeadzoneY = checkInDeadzone(leftStickY);
      int rightDeadzoneX = checkInDeadzone(rightStickX);
      int rightDeadzoneY = checkInDeadzone(rightStickY);
  
      if (leftDeadzoneY != 1 && leftDeadzoneX == 1 && rightDeadzoneX == 1)
      {
        boolean dir = (leftDeadzoneY == 2 ? true : false);
        int normalizedPower = (leftDeadzoneY == 2 ? leftStickY : abs(leftStickY - STICK_CENTER));
        moveRobot(dir, normalizedPower, dir, normalizedPower, dir, normalizedPower, dir, normalizedPower);
      }
  
      if (leftDeadzoneY == 1 && leftDeadzoneX == 0 && rightDeadzoneX == 1)
      {
        int normalizedPower = (leftDeadzoneX == 2 ? leftStickX : abs(leftStickX - STICK_CENTER));
        moveRobot(false, normalizedPower, true, normalizedPower, false, 0, false, 0);
      }
  
      if (leftDeadzoneY == 1 && leftDeadzoneX == 0 && rightDeadzoneX == 1)
      {
        int normalizedPower = (leftDeadzoneX == 2 ? leftStickX : abs(leftStickX - STICK_CENTER));
        moveRobot(false, 0, false, 0, false, normalizedPower, true, normalizedPower);
      }
    }

    else
    {
      bool forward = PS4.getButtonPress(UP);
      bool backward = PS4.getButtonPress(DOWN);
      bool strafeLeft = PS4.getButtonPress(LEFT);
      bool strafeRight = PS4.getButtonPress(RIGHT);
      bool turnLeft = PS4.getButtonPress(CIRCLE);
      bool turnRight = PS4.getButtonPress(SQUARE);

      if (forward)
      {
        moveRobot(true, DIGITAL_SPEED, true, DIGITAL_SPEED, true, DIGITAL_SPEED, true, DIGITAL_SPEED);
      }

      else if(strafeLeft)
      {
        moveRobot(false, DIGITAL_SPEED, true, DIGITAL_SPEED, false, 0, false, 0);
      }

      else if(strafeRight)
      {
        moveRobot(false, 0, false, 0, false, DIGITAL_SPEED, true, DIGITAL_SPEED);
      }

      else if(backward)
      {
        moveRobot(false, DIGITAL_SPEED, false, DIGITAL_SPEED, false, DIGITAL_SPEED, false, DIGITAL_SPEED);
      }

      else if(turnLeft)
      {
        moveRobot(false, DIGITAL_SPEED, false, DIGITAL_SPEED, true, DIGITAL_SPEED, true, DIGITAL_SPEED);
      }

      else if(turnRight)
      {
        moveRobot(true, DIGITAL_SPEED,  true, DIGITAL_SPEED, false, DIGITAL_SPEED, false, DIGITAL_SPEED);
      }
      else
      {
        moveRobot(false, 0, false, 0, false, 0, false, 0);
      }
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
// Value may be any int type, minValue and maxValue MUST be 8-bit
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
  digitalWrite(PIN_FRONT_LEFT_DIR, dirFrontL);
  digitalWrite(PIN_BACK_LEFT_DIR, dirBackL);
  digitalWrite(PIN_FRONT_RIGHT_DIR, !dirFrontR);
  digitalWrite(PIN_BACK_RIGHT_DIR, !dirBackR);

  int normalizedFrontL = clampInt(powerFrontL * 2, 0, 255);
  int normalizedBackL = clampInt(powerBackL * 2, 0, 255);
  int normalizedFrontR = clampInt(powerFrontR * 2, 0, 255);
  int normalizedBackR = clampInt(powerBackR * 2, 0, 255);
  
  analogWrite(PIN_FRONT_LEFT_PWM, normalizedFrontL);
  analogWrite(PIN_BACK_LEFT_PWM, normalizedBackL);
  analogWrite(PIN_FRONT_RIGHT_PWM, normalizedFrontR);
  analogWrite(PIN_BACK_RIGHT_PWM, normalizedBackR);
}
