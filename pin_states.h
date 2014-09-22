#define NUM_OF_MODULES 3

// array to hold binary numbers representing which pins are banana plugs plugged into
// excluding the pins with switches, for previous and current state comparison
byte banana_pins[NUM_OF_MODULES] = { B00000001, B00000001, B00000011 };

// array to hold pins with switches only, for previous and current state comparison
byte switch_pins[NUM_OF_MODULES] = { B00000010, B00000110, B00001100 };
