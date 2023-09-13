Create Bootloader Hex file for Finch/Hummingbird bit:

Project SAMD Application Folder Finch:

Firmware\HummingbirdBit_Finch\SAMD\Finch\Finch2_SAMD_Application

Use the CODAL to create the bootlaoder hex for Finch 2 and Hummingbird Bit . 

Theory:

Load the all the code that is required for microbit to transfer on to SAMD chip using  UART.

1. Get the .bin file generated which is presented in the SAMD project under debug (for example, Finch_2_3.bin)

2. Copy and paste the file into this folder

3. Use command tools to run the python script to generate the PROG_data.h

   py -2 bin2hex.py 

4. Copy and paste the PROG_data.h file into the source directory 

5. Compile the project, you get a MICROBIT.hex file 

6. This hex file is the bootloader hex file for V2 micro:bits



Batch File : Needs to be updated, do not rely on it right now

Make sure all the commands in batch file go through.

