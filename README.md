png2c
===================
Program to convert a PNG file to an C header as an array of hexadecimal unsigned short integers in RGB565 format. This is useful for embedding pixmaps to display with a uC displays.

**Usage:** png2c --input=_input_file_ --output=_output_file_ --name=_c_structure_name_ [--verbose]

Compiled on Linux, but should work on any platform with libpng installed.
For example output, see mario.png and mario.c

___

Original implementation: https://github.com/oleg-codaio/png2c

