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
for you.

Run a program. Right now, the only thing is a framebuffer
test.

Run it with `./fbtest`.
