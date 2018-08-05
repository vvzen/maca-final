// Library created by Mike McCauley at http://www.airspayce.com/mikem/arduino/AccelStepper/
#include <AccelStepper.h>

const int DIR_PIN = 4;
const int STEP_PIN = 5;
// AccelStepper::DRIVER means we'll be using a driver
AccelStepper stepper_x(AccelStepper::DRIVER, STEP_PIN, DIR_PIN);

const int home_switch = 7;

long travel_x; // will store the X value entered in serial monitor
int move_finished = 1; // used to check if a move is completed
long initial_homing = -1;

void setup() {

  pinMode(home_switch, INPUT_PULLUP);

  Serial.begin(9600);
  
  // wait for the driver to wake up
  delay(5);

  stepper_x.setMaxSpeed(100.0);
  stepper_x.setAcceleration(100.0);

  Serial.println("Stepper is Homing . . . . . . . . . . . ");
  Serial.print("home_switch: ");
  Serial.print(digitalRead(home_switch));

  // start homing procedure of motor at startup
  // make the motor move CCW until the switch is activated
  while (digitalRead(home_switch)){
    stepper_x.moveTo(initial_homing); // set the destination position
    initial_homing--;
    stepper_x.run(); // start moving the stepper
    delay(5);
  }

  // stepper is now at home
  stepper_x.setCurrentPosition(0);
  // set low speed and acc for better precision
  stepper_x.setMaxSpeed(100.0);
  stepper_x.setAcceleration(100.0);
  initial_homing = 1;

  // make the stepper move CW until the switch is deactivated
  while (!digitalRead(home_switch)){
    stepper_x.moveTo(initial_homing); // set the destination position
    stepper_x.run(); // start moving the stepper
    initial_homing++;
    delay(5);
  }

  // stepper is now at the final home
  Serial.println(F("Homing Completed\n"));
  stepper_x.setCurrentPosition(0);
  stepper_x.setMaxSpeed(100.0);
  stepper_x.setAcceleration(100.0);
  initial_homing = 1;

  // set normal speed and acc for performance
  stepper_x.setMaxSpeed(1000.0);
  stepper_x.setAcceleration(1000.0);
  
  Serial.println("Enter Travel distance (Positive for CW / Negative for CCW and Zero for back to Home): ");
}

void loop() {
  // put your main code here, to run repeatedly:
  bool is_home = digitalRead(home_switch);

  Serial.print("is home: ");
  Serial.println(is_home);

  //stepper_x.moveTo(initial_homing);
}
