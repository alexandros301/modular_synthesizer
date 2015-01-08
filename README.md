modular_synthesizer
===================

This is a work in progress. A modular synthesizer based on Pure Data, Teensy (programmed in the Arduino language) and Odroid.

8th of January 2015:
Having the synth patched before booting doesn't seem to work as expected, have to impove it.
Additive synthesis with many instances of one oscillator works, but looks like a lot of work for the Odroid as I was getting drop outs. Have to check the CPU with top...
Changed a few things in pow_sine~.pd as well so I could test the multi-instance feature. pow_sine~.pd won't work as a normal oscillator with the current update. Had to do that cause I need more modules (scaling and offset modules) to be able to use normal oscillators to control the frequency and amplitude of a multi-instance oscillator.

Older updates:
I've changed the Arduino sketch and the Pd patch and now only the active modules have their potentiometers read and transferred to Pd.
This means that there is a different approach to patching modules in Pd. All module abstractions (pow_sine~.pd, adc_dac~.pd, var_shapes~.pd, and template_module~.pd) have been updated in this repository. Some generic abstractions have changed slighty and they have also been updated.
Also, you can have your synth patched and Pd will receive all the connections from the Teensy on load, whether the patching update is on or off.

var_shapes~.pd has been changed so that you can use many instances of the oscillator for additive synthesis. A future MIDI module has been taken into account, so you'll see some stuff that concern it inside the abstraction. Have to document this technique further, will do so in the near future. For now, all you need to know is that you have to create a directory inside the 'modules/' directory, called 'module_instances/' and there you should place the var_shapes_instance.pd abstraction which is called by var_shapes~.pd

Notes:
I've uploaded the code both for back-up and for people to fork it, if anyone wants. It is an open source project, but it's still far from complete. Still, you're free to experiment and develop it.

In the directory of this project create a directory with the name 'generic_abstractions' and place there the patches inlet_state.pd, set_inputs.pd, set_outputs.pd and switch_state.pd, and another directory (in the parent directory), called 'modules', and there place adc_dac~.pd, pow_sine~.pd, var_shapes~.pd and template_module~.pd, cause all these abstractions are being called by the main patch, and their relative paths are being used in the declare object (template_module~.pd is not being called, it's just there for people to understand how they should program their modules).
debounce_float.pd and loop.pd are general purpose abstractions I use and placed them in a directory specified in Pd's search path. It's up to you how you'll manage this (debounce_float.pd needs to be revised, it doesn't really work as expected).

In this specific demo patch, two external objects I wrote are being used, [powSine~] and [varShapesOsc~]. You can get them from here http://drymonitis.me/code/ (there are binaries for Linux and OS X, source code, Makefile and help patches, look for the 'Various oscillators' link, under 'Pure Data external objects').

As for the Arduino code, there is the main code and a header file, 'pin_states.h'. Put it in the same directory with the code, so the Arduino IDE can find it.

All modules share the same circuit board, where two DIP switches and a 16 pin header set the direction of data, and the pin of the main multiplexer where each module's 8-channel multiplexer goes. More details will be posted once this project is finished. Get the schematic (quite messy, it's my very first one) and .brd eagle file here https://drive.google.com/file/d/0B907liaBQD8uUmFkX2tuNmtOTWs/view?usp=sharing (both files are named udoo_modular, although no Udoo is used. This project started off with a Udoo but then switched to Odroid, only the name remained. I guess I have to change that).

Written by Alexandros Drymonitis
