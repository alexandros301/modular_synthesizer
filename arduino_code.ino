// code to be devoloed for the modular synthesizer based on Pd, Teensy 3.1 and Odroid
// goes with modular5 Pd patch
// written by Alexandros Drymonitis

#include <SPI.h>
#include "pin_states.h"

// global variables

#define MASTER_CONTROL0 5
#define MASTER_CONTROL1 4
#define MASTER_CONTROL2 3
#define MASTER_CONTROL3 2

#define SLAVE_CONTROL0 8
#define SLAVE_CONTROL1 7
#define SLAVE_CONTROL2 6

#define NUM_OF_MODULES 3

#define SMOOTH 3

// set global variables for output shift registers
const byte output_latch = 10;
// byte array to transfer data to output chips via SPI
byte output_data[NUM_OF_MODULES] = { 0 };
// array to hold number of banana pins used of each output chip
byte output_pins[NUM_OF_MODULES] = {6, 3, 3};
// array to hold the position of the first LED of each module
// if an LED is controlled over serial, exclude it
byte led_pos[NUM_OF_MODULES] = {6, 3, 4};
// boolean to exit the output pins loop when a new connection has been detected
boolean terminate;

// set global variables for input shift registers
const byte input_latch = 9;
// array to receive data from input chips via SPI
byte input_data[NUM_OF_MODULES];
// array to hold position of first switch of each module, starting from 0
// exclude switches that don't control LEDs
byte switch_pos[NUM_OF_MODULES] = {1, 1, 3};
// array to hold number of switches on each module
// exclude switches that don't control LEDs
byte num_of_switches[NUM_OF_MODULES] = {1, 2, 1};
// two-dimensional array to hold states of banana plugs
// row number = sum of elements of outputPins, column number = NUM_OF_MODULES
byte banana_states[12][NUM_OF_MODULES];
// array to store state of switches
byte switch_states[NUM_OF_MODULES];

// variable to hold the DSP state to control the DSP LED
byte DSPstate = 0;

// set global variables for multiplexers
// master multiplexers controlling the multiplexers of each module
const byte num_of_master_mux = 1;
// number of slave multiplexers
int num_of_slave_mux[num_of_master_mux] = {3};
// two dimensional array to hold number of pins used on each slave multiplexer
// rows = num_of_master_mux, columns = greatest number in num_of_slave_mux array
int num_of_pots[num_of_master_mux][3] = { { 7, 4, 1 } };
const int total_pots = 12; // sum of elements of num_of_pots
// array to store multiple readings of each knob for smoothing
unsigned int multiple_pots[total_pots][SMOOTH];
// last element index of each row of the two dimensional array
// just so that you don't need to subtract one from SMOOTH in each loop
int last_col = SMOOTH - 1;

// indices for banana plugs and switches functions
int banana_index = (total_pots * 2) + 1;
int switch_index = banana_index + 4;

// number of bytes to be transfered to Pure Data over serial
// this number is totalPots * 2
// plus four for the banana plugs function
// plus three for the switches function
// plus one (for denoting the beginning of data)
const byte num_of_transfer_data = 32;
// buffer to hold data transfered to Pure Data over serial
byte transfer_data[num_of_transfer_data];


// Custom functions

// analog readings smoothing function
unsigned int smooth(int row_index)
{
  unsigned int smoothed;
  unsigned long accumulate;
  // accumulate analog readings
  for(int i = 0; i < SMOOTH; i++)
    accumulate += multiple_pots[row_index][i];
  // divide readings sum by the number of stored values  
  smoothed = accumulate / SMOOTH;
  
  return smoothed;
}

// function to transfer data to the output shift registers via SPI
void refresh_output()
{
  digitalWrite(output_latch, LOW);
  for(int i = NUM_OF_MODULES - 1; i >= 0; i--)
    SPI.transfer(output_data[i]);
  digitalWrite(output_latch, HIGH);
}

// function to receive data from the input shift registers via SPI
void refresh_input()
{
  digitalWrite(input_latch, LOW);
  digitalWrite(input_latch, HIGH);
  for(int i = 0; i < NUM_OF_MODULES; i++)
    input_data[i] = SPI.transfer(0);
}

// function to check if any connection has changed
void check_connections(int pin)
{
  int index = banana_index;
  // store the first altered state of input bytes to the transfer array
  for(int i = 0; i < NUM_OF_MODULES; i++){
    // mask the input byte to exclude any possible switches on the module and check if changed
    byte masked_banana = input_data[i] & banana_pins[i];
    if(masked_banana != banana_states[pin][i]){
      // store input byte, output pin and input chip # to the transfer array
      transfer_data[index++] = input_data[i] & 0x7f;
      transfer_data[index++] = input_data[i] >> 7;
      transfer_data[index++] = pin;
      transfer_data[index] = i;
      // update the banana_states array
      banana_states[pin][i] = masked_banana;
      // set the loop termination boolean to true and exit loop
      terminate = true;
      break;
    }
  }
}

// function to check if the state of any switch has changed
void check_switches()
{
  int index = switch_index;
  for(int i = 0; i < NUM_OF_MODULES; i++){
    // mask the input byte according to the position of the switches and check if changed
    byte masked_switch = input_data[i] & switch_pins[i];
    if(masked_switch != switch_states[i]){
      // if changed, store to transfer array which chip and the corresponding byte
      transfer_data[index++] = input_data[i] & 0x7f;
      transfer_data[index++] = input_data[i] >> 7;
      transfer_data[index] = i;
      // update the switch_states array and exit loop
      switch_states[i] = masked_switch;
      break;
    }
  }
}


// Built-in functions

void setup()
{
  // initialize the SPI library
  SPI.begin();
  // initialize the pins used to control the shift registers
  pinMode(input_latch, OUTPUT);
  pinMode(output_latch, OUTPUT);
  digitalWrite(input_latch, HIGH);
  digitalWrite(output_latch, HIGH);
  
  // initialize output chips with all banana plug pins high
  // and LED pins low and call the output shift registers function
  for(int i = 0; i < NUM_OF_MODULES; i++){
    for(int j = 0; j < output_pins[i]; j++)
      bitSet(output_data[i], j);
  }
  refresh_output();
  
  //initialize the banana sockets states two-dimensional array
  for(int i = 0; i < 12; i++){
    for(int j = 0; j < NUM_OF_MODULES; j++)
      banana_states[i][j] = banana_pins[j];
  }
  
  // initialize the switch states array
  for(int i = 0; i < NUM_OF_MODULES; i++)
    switch_states[i] = switch_pins[i];
    
  // initialize the last seven bytes of the transfer array to 255
  int index = banana_index;
  for(int i = 0; i < 7; i++)
    transfer_data[index++] = 0xff;
    
  // initialize the multiple_pots array with zeros
  for(int i = 0; i < total_pots; i++){
    for(int j = 0; j < SMOOTH; j++)
      multiple_pots[i][j] = 0;
  }
  
  // initialize Arduino's control pins for multiplexers
  for(int i = 2; i < 9; i++){
    pinMode(i, OUTPUT);
    digitalWrite(i, LOW);
  }
  
  // analog resolution for the Teensy
  analogReadResolution(13);
  // open the serial port
  Serial.begin(115200);
}

void loop()
{
  transfer_data[0] = 0xc0; // denote start of data stream
  int index = 1; // transfer data array index offset
  
  // store DSP state input from Pd
  if(Serial.available()){
    DSPstate = Serial.read() - '0';
    bitWrite(output_data[2], 3, DSPstate);
  }
  
  // read potentiometers
  int row_index = 0;
  // run througn all master multiplexers
  for(int master_mux = 0; master_mux < num_of_master_mux; master_mux++){
    // first move the elements of current row one position to the left so we can add the new value at the end
    for(int i = 0; i < last_col; i++) multiple_pots[row_index][i] = multiple_pots[row_index][i + 1];
    // run through all slave multiplexers
    for(int slave_mux = 0; slave_mux < num_of_slave_mux[master_mux]; slave_mux++){
      digitalWrite(MASTER_CONTROL0, (slave_mux&15)>>3);
      digitalWrite(MASTER_CONTROL1, (slave_mux&7)>>2);
      digitalWrite(MASTER_CONTROL2, (slave_mux&3)>>1);
      digitalWrite(MASTER_CONTROL3, (slave_mux&1));
      // run through the pins used on each slave multiplexer
      for(int pot = 0; pot < num_of_pots[master_mux][slave_mux]; pot++){
        digitalWrite(SLAVE_CONTROL0, (pot&7)>>2);
        digitalWrite(SLAVE_CONTROL1, (pot&3)>>1);
        digitalWrite(SLAVE_CONTROL2, (pot&1));
        // store new reading to the last element of pots row
        multiple_pots[row_index][last_col] = analogRead(master_mux);
        // call the smoothing function
        unsigned int smoothed = smooth(row_index);
        // and store the smoothed value to the transfer_data array
        transfer_data[index++] = smoothed & 0x007f;
        transfer_data[index++] = smoothed >> 7;
        row_index++;
      }
    }
  }
  
  // set all LEDs according to switches
  for(int i = 0; i < NUM_OF_MODULES; i++){  
    int pos_switch = switch_pos[i];
    int pos_led = led_pos[i];
    // read the switches of the current chip and set LEDs accordingly
    for(int j = 0; j < num_of_switches[i]; j++){
      int switch_state = bitRead(input_data[i], pos_switch++);
      bitWrite(output_data[i], pos_led++, !switch_state);
    }
  }
  
  // write to output shift registers and read input shift registers
  // first set terminate boolean to false
  terminate = false;
  // pin number variable
  int pin = 0;
  // update the shift registers
  for(int i = 0; i < NUM_OF_MODULES; i++){
    // add number of pins of previous chip
    if(i) pin += output_pins[i - 1];
    // variable to control the bits of the current byte
    int bit_increment = 0;
    // then go through the banaba plug pins; start loop from 1 to set the pin variable correctly
    for(int j = 0; j < output_pins[i]; j++){
      // local variable to store pin number
      int local_pin = pin;
      // set the current pin LOW
      bitClear(output_data[i], bit_increment);
      // call the output shift registers function
      refresh_output();
      // give some time to the shift registers to do their job
      delayMicroseconds(1);
      // call the input shift registers function
      refresh_input();
      // add current and previous pins of current chip
      local_pin += j;
      // check if there's a new connection
      check_connections(local_pin);
      // reset the pin HIGH
      bitSet(output_data[i], bit_increment);
      // check if a new connection has been detected and if so, exit loop
      if(terminate) break;
      // increment byte's bit
      bit_increment++;
    }
    if(terminate) break;
  }
  
  // call the function that stores altered states of switches
  check_switches();
  // write all data to the serial port
  Serial.write(transfer_data, num_of_transfer_data);
}
