/*
 * Programmed by Zavier Yohe - 2019
 * Ctrl-F NOTE for important information
 * Controls the Theseus mini-forklift robot created for ATMAE 
 */

#include <PS4BT.h>
#include <usbhub.h>
#include <Servo.h>

// Satisfy the IDE, which needs to see the include statment in the ino too.
#ifdef dobogusinclude
#include <spi4teensy3.h>
#endif
#include <SPI.h>

// Defines the pins for wheel control
const int PIN_FRONT_LEFT_DIR  = 24;
const int PIN_FRONT_RIGHT_DIR = 28;
const int PIN_BACK_LEFT_DIR   = 22;
const int PIN_BACK_RIGHT_DIR  = 26;
const int PIN_FRONT_LEFT_PWM  = 3;
const int PIN_FRONT_RIGHT_PWM = 4;
const int PIN_BACK_LEFT_PWM   = 2;
const int PIN_BACK_RIGHT_PWM  = 5;

// Defines the pins for servo control
const int ARM_PIN = 8;
const int CLAW_PIN = 9;

// Defines speed values
const int SLOW_SPEED = 64;
const int WALK_SPEED = 128;
const int RUN_SPEED = 255;

// Defines color values
const int SLOW_COLOR[3] = {0,255,0};
const int WALK_COLOR[3] = {0,0,255};
const int RUN_COLOR[3] = {255,0,0};

/*
 * Initializes speed values
 *    NOTE: runMode 0 corresponds to slow, but pairing
 * the USB Controller triggers runMode change, so it'll
 * start at runMode 1. If the runMode button is changed,
 * this must be changed or robot will default to slow.
 */
 
int runMode = 0;
int currentSpeed;

/* 
 *  Create servo objects
 *    NOTE: The current servos are hacked LD20MG,
 * removing the position sensor and bridging them with 
 * resistors, allowing them to with the library. They 
 * behave in a non-standard way; the stop value is around 90,
 * but must be found per-servo via trial and error.
 * Current arm servo's stop value is 95
 * Current claw servo's stop value is 93       
 */
Servo arm;
Servo claw;

// Magic bluetooth setup stuff
USB Usb;
BTD Btd(&Usb);

/*
 *    NOTE: Two ways to connect controller. ONLY use one! 
 * To change connection modes, comment the currently-used line
 * below, and uncomment the other.
 * To pair controller, hold share, then press the PS button for 
 * 3 seconds. Should flash quickly. If it pulses slowly, you did 
 * it wrong.
 * To connect a paired controller, press and hold PS button. 
 * Connecting controllers is buggy; pairing should be preferred.
 */

// Pair:
PS4BT PS4(&Btd, PAIR); 
// Connect:
//PS4BT PS4(&Btd);

// Most important line
String chair = "Dr. Lawrence";

// Runs once when arduino starts
void setup() 
{
  //More magic bluetooth stuff
  if (Usb.Init() == -1) 
  {
    while (1); // Halt
  }

  // Put all pins we care about into the right mode
  pinMode(PIN_FRONT_LEFT_DIR, OUTPUT);
  pinMode(PIN_BACK_LEFT_DIR, OUTPUT);
  pinMode(PIN_FRONT_RIGHT_DIR, OUTPUT);
  pinMode(PIN_BACK_RIGHT_DIR, OUTPUT);
  pinMode(PIN_FRONT_LEFT_PWM, OUTPUT);
  pinMode(PIN_BACK_LEFT_PWM, OUTPUT);
  pinMode(PIN_FRONT_RIGHT_PWM, OUTPUT);
  pinMode(PIN_BACK_RIGHT_PWM, OUTPUT);

  // Setup servo controllers
  arm.attach(ARM_PIN);
  claw.attach(CLAW_PIN);
  arm.write(95);
  claw.write(93);
  
  // Setup swag
  PS4.setLed(WALK_COLOR[0], WALK_COLOR[1], WALK_COLOR[2]);
  
}

// Runs approximately 100 times per second
void loop() 
{
  // More magic Bluetooth stuff
  Usb.Task();
  if (PS4.connected()) 
  {

    // Remote kill switch
    if (PS4.getButtonClick(OPTIONS))
    {
      while(1){}; // Halt
    }

    // "Autonomously" moves robot forward a set distance to please the judges
    if (PS4.getButtonClick(TOUCHPAD))
    {
      moveRobot(true, WALK_SPEED, true, WALK_SPEED, true, WALK_SPEED, true, WALK_SPEED);
      delay(3250);
    }

    // Toggle run mode
    if(PS4.getButtonClick(SHARE))
    {
      if (runMode == 0)
      {
        runMode = 1;
        PS4.setLed(WALK_COLOR[0], WALK_COLOR[1], WALK_COLOR[2]);
        currentSpeed = WALK_SPEED;
      }

      else if(runMode == 1)
      {
        runMode = 2;
        PS4.setLed(RUN_COLOR[0], RUN_COLOR[1], RUN_COLOR[2]);
        currentSpeed = RUN_SPEED;
      }

      else if(runMode == 2)
      {
        runMode = 0;
        PS4.setLed(SLOW_COLOR[0], SLOW_COLOR[1], SLOW_COLOR[2]);
        currentSpeed = SLOW_SPEED;
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

    /*
     * Arm controls
     *    NOTE: Servo values are 0-180. 0 is ful reverse,
     * 180 is full forward. Stop value is roughly 90,
     * but due to the servo hack, this varies.
     * See "Create servo objects" for more info.
     */

    if(armUp)
    {
      arm.write(0);
    }

    else if(armDown)
    {
      arm.write(180);
    }

    else
    {
      arm.write(95);
    }
    

    if(clawOpen)
    {
      claw.write(45);
    }

    else if(clawClose)
    {
      claw.write(135);
    }

    else
    {
      claw.write(93);
    }

    /*
     * Movement controls
     *    NOTE: Current supported controls are forward,
     * reverse, strafing, and turning in place.
     * moveRobot takes pairs of direction and speed arguments. 
     * True is forward, false is reverse. Speed values are 
     * 0-255. These pairs correspond, in order, to the front-left,
     * back-left, front-right, and back-right wheels.
     */

    if (forward && !turnLeft && !turnRight)
    {
      moveRobot(true, currentSpeed, true, currentSpeed, true, currentSpeed, true, currentSpeed);
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

  int normalizedFrontL = powerFrontL;
  int normalizedBackL = powerBackL;
  int normalizedFrontR = powerFrontR;
  int normalizedBackR = powerBackR;
  
  analogWrite(PIN_FRONT_LEFT_PWM, normalizedFrontL);
  analogWrite(PIN_BACK_LEFT_PWM, normalizedBackL);
  analogWrite(PIN_FRONT_RIGHT_PWM, normalizedFrontR);
  analogWrite(PIN_BACK_RIGHT_PWM, normalizedBackR);
}
