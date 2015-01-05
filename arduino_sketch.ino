// code to be devoloed for the modular synthesizer based on Pd, Teensy 3.1 and Odroid
// goes with modular_synth_test Pd patch
// written by Alexandros Drymonitis

#include <SPI.h>
#include "pin_states.h"

// global variables

// set global variables for output shift registers
const byte output_latch = 10;
// byte array to transfer data to output chips via SPI
byte output_data[NUM_OF_MODULES] = { 0 };
// array to hold number of banana pins used of each output chip
byte output_pins[NUM_OF_MODULES] = {7, 3, 3};
// array to hold the position of the first LED of each module
// if an LED is controlled over serial, exclude it
byte led_pos[NUM_OF_MODULES] = {7, 3, 4};
// boolean to exit the output pins loop when a new connection has been detected
boolean terminate_bananas;

// set global variables for input shift registers
const byte input_latch = 9;
// array to receive data from input chips via SPI
byte input_data[NUM_OF_MODULES];
// array to hold position of first switch of each module, starting from 0
// exclude switches that don't control LEDs
byte switch_pos[NUM_OF_MODULES] = {1, 1, 3};
// array to hold number of switches on each module
// exclude switches that don't control LEDs
byte num_of_switches[NUM_OF_MODULES] = {1, 1, 1};
// two-dimensional array to hold states of banana plugs
// row number = sum of elements of outputPins, column number = NUM_OF_MODULES
byte banana_states[13][NUM_OF_MODULES];
// array to store state of switches
byte switch_states[NUM_OF_MODULES];
// and this is the index of the ADC/DAC module, where this switch is attached to
int dac_module = 2;
// boolean to determine whether to store connection states to active_modules or back_up array
boolean patch_boolean = false;

// indices for connection and switch functions
const byte connection_index = NUM_OF_MODULES;
const byte switch_index = connection_index + 1;

// variable to receive data from the serial port
byte serial_value;
byte DSPstate = 0;

// set global variables for multiplexers
// master multiplexers controlling the multiplexers of each module
const byte num_of_master_mux = 1;
// number of slave multiplexers
int num_of_slave_mux[num_of_master_mux] = {3};
// two dimensional array to hold number of pins used on each slave multiplexer
// rows = num_of_master_mux, columns = greatest number in num_of_slave_mux array
int num_of_pots[num_of_master_mux][3] = { { 8, 4, 1 } };
const int total_pots = 13; // sum of elements of num_of_pots
// array to store multiple readings of each knob for smoothing
unsigned int multiple_pots[total_pots][SMOOTH];
// array to sum up readings for smoothing
unsigned long sums[total_pots];
// column variable to control multiple_pots
static byte tail;
// arrays to store and back up activity of modules (back up if we're not updating the patching)
byte active_modules[NUM_OF_MODULES];
byte back_up_modules[NUM_OF_MODULES];

// global index for connection, switch and potentiometer functions and loops
int ndx;

// maximum number of bytes to be transfered to Pure Data over serial
// 99% of the times only part of this will be transferred to Pd
// this number is totalPots * 2 plus total number of modules (NUM_OF_MODULES macro)
// plus five for the connections function
// plus four for the switches function
// plus one (for denoting the beginning of data)
const byte num_of_total_data = (total_pots * 2) + NUM_OF_MODULES + 10;
// buffer to hold data transfered to Pure Data over serial
byte transfer_data[num_of_total_data];


// Custom functions

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
void check_connections(int pin, int module)
{
  int local_index = ndx;
  // store the first altered state of input bytes to the transfer array
  for(int i = 0; i < NUM_OF_MODULES; i++){
    // mask the input byte to exclude any possible switches on the module and check if changed
    byte masked_banana = input_data[i] & banana_pins[i];
    if(masked_banana != banana_states[pin][i]){
      // store connection data index, input byte, output pin and input chip # to the transfer array
      transfer_data[local_index++] = connection_index;
      transfer_data[local_index++] = input_data[i] & 0x7f;
      transfer_data[local_index++] = input_data[i] >> 7;
      transfer_data[local_index++] = pin;
      transfer_data[local_index++] = i;
      // check if the current module is being activated or not
      if(masked_banana != banana_pins[i]){
        if(module == i) back_up_modules[i] += 1;
        else{
          back_up_modules[i] += 1;
          back_up_modules[module] += 1; 
        }
      }
      else{
        if(module == i) back_up_modules[i] -= 1;
        else{
          back_up_modules[i] -= 1;
          back_up_modules[module] -= 1;
        }
      }
      // update the banana_states array
      banana_states[pin][i] = masked_banana;
      // set the loop termination boolean to true, update the global index and exit loop
      terminate_bananas = true;
      ndx = local_index;
      break;
    }
  }
  // update the active_modules array if the patch update switch is on
  if(patch_boolean){
    for(int i = 0; i < NUM_OF_MODULES; i++) active_modules[i] = back_up_modules[i];
  }
}

// function to check if the state of any switch has changed
void check_switches()
{
  int local_index = ndx;
  for(int i = 0; i < NUM_OF_MODULES; i++){
    // mask the input byte according to the position of the switches and check if changed
    byte masked_switch = input_data[i] & switch_pins[i];
    if(masked_switch != switch_states[i]){
      // if changed, store switch data index, chip # and the corresponding byte
      transfer_data[local_index++] = switch_index;
      transfer_data[local_index++] = input_data[i] & 0x7f;
      transfer_data[local_index++] = input_data[i] >> 7;
      transfer_data[local_index++] = i;
      // check if we're in the dac_module and read the patch update switch
      if(i == dac_module){
        if((masked_switch & patch_update) == patch_update) patch_boolean = false;
        else patch_boolean = true;
      }
      // update the switch_states array and the global index and exit loop
      switch_states[i] = masked_switch;
      ndx = local_index;
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
  for(int i = 0; i < 13; i++){
    for(int j = 0; j < NUM_OF_MODULES; j++)
      banana_states[i][j] = banana_pins[j];
  }
  
  // initialize the switch states array
  for(int i = 0; i < NUM_OF_MODULES; i++)
    switch_states[i] = switch_pins[i];
  
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
  ndx = 1;
  
  // store DSP state input from Pd
  if(Serial.available()){
    byte inByte = Serial.read();
    if((inByte >= '0') && (inByte <= '9'))
      serial_value = inByte - '0';
    else{
      if(inByte == 'd') DSPstate = serial_value;
    }
    bitWrite(output_data[dac_module], 3, DSPstate);
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
  terminate_bananas = false;
  // pin number variable
  int pin = 0;
  for(int i = 0; i < NUM_OF_MODULES; i++){
    // add number of pins of previous chip
    if(i) pin += output_pins[i - 1];
    // variable to control the bits of the current byte
    int bit_increment = 0;
    // then go through the banaba plug pins
    for(int j = 0; j < output_pins[i]; j++){
      int local_pin = pin;
      // set the current pin LOW
      bitClear(output_data[i], bit_increment);
      refresh_output();
      // give some time to the shift registers to do their job
      delayMicroseconds(1);
      refresh_input();
      // add current and previous pins of current chip
      local_pin += j;
      check_connections(local_pin, i);
      // reset the pin HIGH
      bitSet(output_data[i], bit_increment);
      // check if a new connection has been detected and if so, exit loop
      if(terminate_bananas) break;
      // increment byte's bit
      bit_increment++;
    }
    if(terminate_bananas) break;
  }
  
  check_switches();
  
  // set local index to global index's current value
  int local_index = ndx;
  // set row index for 2D array multiple pots
  int row_index = 0;
  int module_index = 0;
  // run througn all master multiplexers
  for(int master_mux = 0; master_mux < num_of_master_mux; master_mux++){
    // run through all slave multiplexers
    for(int slave_mux = 0; slave_mux < num_of_slave_mux[master_mux]; slave_mux++){
      digitalWrite(MASTER_CONTROL0, (slave_mux&15)>>3);
      digitalWrite(MASTER_CONTROL1, (slave_mux&7)>>2);
      digitalWrite(MASTER_CONTROL2, (slave_mux&3)>>1);
      digitalWrite(MASTER_CONTROL3, (slave_mux&1));
      // if this module is active, store its index to the transfer array
      if(active_modules[module_index]) transfer_data[local_index++] = module_index;
      // run through the pins used on each slave multiplexer
      for(int pot = 0; pot < num_of_pots[master_mux][slave_mux]; pot++){
        if(active_modules[module_index]){ // if this module is active, read its pots
          digitalWrite(SLAVE_CONTROL0, (pot&7)>>2);
          digitalWrite(SLAVE_CONTROL1, (pot&3)>>1);
          digitalWrite(SLAVE_CONTROL2, (pot&1));
          // smooth out the analog reading
          sums[row_index] -= multiple_pots[row_index][tail];
          sums[row_index] += multiple_pots[row_index][tail] = analogRead(master_mux);
          unsigned int smoothed = sums[row_index] / SMOOTH;
          // and store the smoothed value to the transfer_data array
          transfer_data[local_index++] = smoothed & 0x007f;
          transfer_data[local_index++] = (smoothed >> 7) & 0x003f;
        }
        row_index++; // update the 2D array row index anyway
      }
      module_index++;
    }
  }
  tail++;
  if(tail >= SMOOTH) tail = 0;
  
  // write all data to the serial port
  int current_data = local_index; // set amount of data to transfer
  Serial.write(transfer_data, current_data);
}
