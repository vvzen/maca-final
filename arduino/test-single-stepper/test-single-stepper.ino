#include <AccelStepper.h> // Library by Mike McCauley at http://www.airspayce.com/mikem/arduino/AccelStepper/
#include <PacketSerial.h> // library by bakercp at https://github.com/bakercp/PacketSerial

SLIPPacketSerial packet_serial;

// PINOUT
const int Y1_DIR_PIN = 2;
const int Y1_STEP_PIN = 3;
const int Y2_DIR_PIN = 4;
const int Y2_STEP_PIN = 5;
const int X_DIR_PIN = 6;
const int X_STEP_PIN = 7;

// AccelStepper::DRIVER means we'll be using a driver, in my case the A4899
//AccelStepper stepper_y1(AccelStepper::DRIVER, Y1_STEP_PIN, Y1_DIR_PIN);
//AccelStepper stepper_y2(AccelStepper::DRIVER, Y2_STEP_PIN, Y2_DIR_PIN);
//AccelStepper stepper_x(AccelStepper::DRIVER, X_STEP_PIN, X_DIR_PIN);

const int X_HOME_SWITCH_PIN = 11;
const int Y_HOME_SWITCH_PIN = 12;

int move_finished = 1; // used to check if a move is completed
long initial_homing = -1;

bool one_shot = true;

void setup() {

  // init serial
  Serial.begin(9600);
  
  // set limit switch pin
  pinMode(X_HOME_SWITCH_PIN, INPUT_PULLUP);
  pinMode(Y_HOME_SWITCH_PIN, INPUT_PULLUP);

  // wait for the driver to wake up
  delay(5);
  
  //stepper_y.setCurrentPosition(0);

  // set max speeds and accelarations
  /*
  stepper_x.setMaxSpeed(200.0);
  stepper_y1.setMaxSpeed(200.0);
  stepper_y2.setMaxSpeed(200.0);
  stepper_x.setAcceleration(100.0);
  stepper_y1.setAcceleration(100.0);
  stepper_y2.setAcceleration(100.0);
  */

  packet_serial.begin(115200);
  packet_serial.setPacketHandler(&on_packet_received);
  
  delay(5);
}

void loop() {
  // update the steppers
  //stepper_y1.run();
  //stepper_y2.run();
  
  // read in any incoming serial data
  packet_serial.update();
}

// When an encoded packet is received and decoded, it will be delivered here.
// The `buffer` is a pointer to the decoded byte array. `size` is the number of
// bytes in the `buffer`.
void on_packet_received(const uint8_t * buff, size_t buff_size){

  // make a temporary buffer
  uint8_t temp_buffer [buff_size];

  // copy the received packet into our buffer
  memcpy(temp_buffer, buff, buff_size);

  // send back same message
  packet_serial.send(temp_buffer, buff_size);

  if (temp_buffer[0] == 'M'){
    handle_move_command(temp_buffer, buff_size);
  }
  else if (temp_buffer[0] == 'H'){
    handle_home_command();
  }
  
  //delay(1000);
}

void handle_move_command(const uint8_t * buff, size_t buff_size){

  int x_move, y_move;

  // parse the command
  // sscanf is safe here because we know that the buffer has fixed length
  sscanf(buff, "MX%dY%d", &x_move, &y_move);


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

void handle_home_command(){
  
}

