modular_synthesizer
===================

This is a work in progress. A modular synthesizer based on Pure Data, Teensy (programmed in the Arduino language) an
Odroid.

I've uploaded the code both for back-up and for people to grab it and modify it, if anyone wants. It is an open
source project, but it's still far from complete. Still, you're free to experiment and develop it.

In the directory of this project create a directory with the name 'generic_abstractions' and place there the patches
inlet_state.pd, set_inputs.pd, set_outputs.pd and switch_state.pd, and another directory (in the parent directory),
called 'modules', and there place adc_dac~.pd, pow_sine~.pd, var_shapes~.pd and template_module~.pd, cause all these
abstractions are being called by the main patch, and their relative paths are being used in the declare object
(template_module~.pd is not being called, it's just there for people to understand how they should program their
modules).
debounce_float.pd and loop.pd are general purpose abstractions I use and placed them in a directory specified in Pd's
search path. It's up to you how you'll manage this (debounce_float.pd needs to be revised, it doesn't really work as
expected).

In this specific demo patch, two external objects I wrote are being used, [powSine~] and [varShapesOsc~]. You can get
them from here http://drymonitis.me/code/ (there are binaries for Linux and OS X, source code, Makefile and help
patches, look for the 'Various oscillators' link, under 'Pure Data external objects'.

As for the Arduino code, there is the main code and a header file, 'pin_states.h'. Put it in the same directory with
the code, so the Arduino IDE can find it.

Written by Alexandros Drymonitis
