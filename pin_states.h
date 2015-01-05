#define NUM_OF_MODULES 3

#define MASTER_CONTROL0 5
#define MASTER_CONTROL1 4
#define MASTER_CONTROL2 3
#define MASTER_CONTROL3 2

#define SLAVE_CONTROL0 8
#define SLAVE_CONTROL1 7
#define SLAVE_CONTROL2 6

#define SMOOTH 20

// array to hold binary numbers of input chips pins with banana sockets only
// for previous and current state comparison
byte banana_pins[NUM_OF_MODULES] = { B00000001, B00000001, B00000011 };

// array to hold binary numbers of input chips pins with switches only
// for previous and current state comparison
byte switch_pins[NUM_OF_MODULES] = { B00001110, B00000010, B00001100 };

// this byte represents the pin the patch update switch is attached to
// and it's used in the check_switches function
byte patch_update = B00001000;
