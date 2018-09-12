#include <AccelStepper.h> // Library by Mike McCauley at http://www.airspayce.com/mikem/arduino/AccelStepper/
#include <PacketSerial.h> // library by bakercp at https://github.com/bakercp/PacketSerial
#include <Wire.h>
#include <Adafruit_INA219.h>

SLIPPacketSerial packet_serial;

// PINOUT
const int Y1_DIR_PIN = 5;
const int Y1_STEP_PIN = 6;
const int Y2_DIR_PIN = 7;
const int Y2_STEP_PIN = 8;
//const int X_DIR_PIN = 6;
//const int X_STEP_PIN = 7;

// AccelStepper::DRIVER means we'll be using a driver, in my case the A4899
AccelStepper stepper_y1(AccelStepper::DRIVER, Y1_STEP_PIN, Y1_DIR_PIN);
AccelStepper stepper_y2(AccelStepper::DRIVER, Y2_STEP_PIN, Y2_DIR_PIN);
//AccelStepper stepper_x(AccelStepper::DRIVER, X_STEP_PIN, X_DIR_PIN);

//const int X_HOME_SWITCH_PIN = 11;
//const int Y_HOME_SWITCH_PIN = 12;

const int DEBUG_SWITCH = 2;

Adafruit_INA219 ina219; // Declare and instance of INA219

void setup() {

  // init ina129 sensor
  ina219.begin();
  
  // set limit switch pin
  pinMode(DEBUG_SWITCH, INPUT_PULLUP);
  //pinMode(X_HOME_SWITCH_PIN, INPUT_PULLUP);
  //pinMode(Y_HOME_SWITCH_PIN, INPUT_PULLUP);

  // wait for the driver to wake up
  delay(5);

  // set max speeds and accelarations

  stepper_y1.setCurrentPosition(0);
  stepper_y2.setCurrentPosition(0);
  
  //stepper_x.setMaxSpeed(200.0);
  stepper_y1.setMaxSpeed(5000.0);
  stepper_y2.setMaxSpeed(5000.0);
  //stepper_x.setAcceleration(100.0);
  stepper_y1.setAcceleration(1000.0);
  stepper_y2.setAcceleration(1000.0);

  packet_serial.begin(115200);
  packet_serial.setPacketHandler(&on_packet_received);
  
  delay(5);

  delay(1000);
  stepper_y1.moveTo(-1000);
  stepper_y2.moveTo(-1000);
}

void loop() {
  // update the steppers
  stepper_y1.run();
  stepper_y2.run();
  h
  // read in any incoming serial data
  packet_serial.update();
}

// When an encoded packet is received and decoded, it will be delivered here.
// The `buffer` is a pointer to the decoded byte array. `size` is the number of
// bytes in the `buffer`.
void on_packet_received(const uint8_t * buff, size_t buff_size){

  // make a temporary buffer
  uint8_t temp_buffer[buff_size];

  // copy the received packet into our buffer
  memcpy(temp_buffer, buff, buff_size);

  // send back same message
  packet_serial.send(temp_buffer, buff_size);

  if (temp_buffer[0] == 'M'){
    handle_move_command(temp_buffer, buff_size);
  }
  else if (temp_buffer[0] == 'H'){
    handle_home_command(temp_buffer, buff_size);
  }
  
  //delay(1000);
}

void handle_move_command(const uint8_t * buff, size_t buff_size){

  int x_move, y_move;

  // parse the command
  // sscanf is safe here because we know that the buffer has fixed length
  sscanf(buff, "MX%dY%d", &x_move, &y_move);

  // set the next position for the motors
  stepper_y1.moveTo(y_move);
  stepper_y2.moveTo(y_move);

  // FIXME:
  // This could definitely be optimized, but for now it works
  // (and premature optimization is the root of all evil)
  char x_move_buffer[3];
  char y_move_buffer[3];
  String x_move_string = String(x_move);
  String y_move_string = String(y_move);
  x_move_string.toCharArray(x_move_buffer, 3);
  y_move_string.toCharArray(y_move_buffer, 3);
  
  packet_serial.send("x move:", 6);
  packet_serial.send(x_move_buffer, 3);
  packet_serial.send("y move:", 6);
  packet_serial.send(y_move_buffer, 3);
}

void handle_home_command(const uint8_t * buff, size_t buff_size){

  // HG --> Get Home
  if (buff[1] == 'G'){
    packet_serial.send("gethome", 7);
    int home_mov = 0;
    stepper_y1.moveTo(home_mov);
  }
  // HS --> Set Home
  else if (buff[1] == 'S'){
    packet_serial.send("sethome", 7);
    stepper_y1.setCurrentPosition(0);
  }
  
}

