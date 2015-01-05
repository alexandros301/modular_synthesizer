modular_synthesizer
===================

This is a work in progress. A modular synthesizer based on Pure Data, Teensy (programmed in the Arduino language) and Odroid.

Updates:
I've changed the Arduino sketch and the Pd patch and now only the active modules have their potentiometers read and transferred to Pd.
This means that there is a different approach to patching modules in Pd. All module abstractions (pow_sine~.pd, adc_dac~.pd and var_shapes~.pd) have been updated in this repository, only template_module~.pd hasn't been updated yet, it will be in the near future. Some generic abstractions have changed slighty and they have also been updated.

Notes:
I've uploaded the code both for back-up and for people to fork it, if anyone wants. It is an open source project, but it's still far from complete. Still, you're free to experiment and develop it.

In the directory of this project create a directory with the name 'generic_abstractions' and place there the patches inlet_state.pd, set_inputs.pd, set_outputs.pd and switch_state.pd, and another directory (in the parent directory), called 'modules', and there place adc_dac~.pd, pow_sine~.pd, var_shapes~.pd and template_module~.pd, cause all these abstractions are being called by the main patch, and their relative paths are being used in the declare object (template_module~.pd is not being called, it's just there for people to understand how they should program their modules).
debounce_float.pd and loop.pd are general purpose abstractions I use and placed them in a directory specified in Pd's search path. It's up to you how you'll manage this (debounce_float.pd needs to be revised, it doesn't really work as expected).

In this specific demo patch, two external objects I wrote are being used, [powSine~] and [varShapesOsc~]. You can get them from here http://drymonitis.me/code/ (there are binaries for Linux and OS X, source code, Makefile and help patches, look for the 'Various oscillators' link, under 'Pure Data external objects'.

As for the Arduino code, there is the main code and a header file, 'pin_states.h'. Put it in the same directory with the code, so the Arduino IDE can find it.

All modules share the same circuit board, where two DIP switches and a 16 pin header set the direction of data, and the pin of the main multiplexer where each module's 8-channel multiplexer goes. More details will be posted once this project is finished. Get the schematic (quite messy, it's my very first one) and .brd eagle file here https://drive.google.com/file/d/0B907liaBQD8uUmFkX2tuNmtOTWs/view?usp=sharing (both files are named udoo_modular, although no Udoo is used. This project started off with a Udoo but then switched to Odroid, only the name remained. I guess I have to change that).

Written by Alexandros Drymonitis
