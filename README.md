# Low Level Norns

Some small snippets of C code that do relatively low-level
things on the norns.

## Setup

You will need to be able to enter a norns shell, either
through SSH or serial. An internet connection is helpful,
but not necessarily needed.

Clone this repository onto your norns.

Compile things with `make`.

Temporarily disable default norns stuff by running
`sh kill_stuff.sh`. This stops supercollider, matron,
and crone. Rebooting the machine will restart these
for you. I do this by running `poweroff`, then manually
powering on the device.

The programs available are as follows:
* audio.c: simple JACK client example, audio stops on key press
* button.c: button handling example
* fbtest.c: uses the framebuffer to print a smiley face to norns screen
* input_monitor: watches for button/encoder input and prints to console when detected
* knob.c: similar to button.c, but uses the encoders

Run one with `./(program name)`, replacing (program name) with your target program. Press Ctrl-C to stop it.
