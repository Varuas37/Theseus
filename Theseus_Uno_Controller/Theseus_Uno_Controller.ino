#include <Servo.h>

#define const int ARM_PIN = 1;
#define const int CLAW_PIN = 0;

#define const int ARM_STOP = 95;
#define const int CLAW_STOP = 90;


Servo arm;
Servo claw;

void setup()
{
  Serial.begin(9600);
  arm.attach(ARM_PIN);
  claw.attach(CLAW_PIN);
  arm.write(ARM_STOP);
  claw.write(CLAW_STOP);  
}

void loop() 
{
  int arm_value = Serial.read();
  int claw_value = Serial.read();

  if (arm_value == -1 or claw_value == -1)
  {
    return;
  }

  if (arm_value == 0)
  {
    arm.write(ARM_STOP);
  }

  else if(arm_value == 1)
  {
    arm.write(180);
  }

  else if(arm_value == 2)
  {
    arm.write(0);
  }

  if (claw_value == 0)
  {
    claw.write(CLAW_STOP);
  }

  else if(claw_value == 1)
  {
    claw.write(180);
  }

  else if(claw_value == 2)
  {
    claw.write(0);
  }
}
