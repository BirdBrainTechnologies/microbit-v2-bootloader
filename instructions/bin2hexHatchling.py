#!/usr/bin/python

import sys,getopt

filename = None
blocksize = 32000
count = 0
total_count = 0


#Read the blocks 
#with open("C:\\Users\\raghu\\Dropbox (BirdBrain Tech)\\Engineering\\Firmware\\Finch\\Finch2.3\\SAMD21\\Finch_2_3\\Finch_2_3\\Debug\\Finch_2_3.bin","rb") as f:
#with open("C:\\Users\\raghu\\Dropbox (BirdBrain Tech)\\Firmware\\Firmware\\HummingbirdBit_Finch\\Microbit\\Bootloader\\Finch\\Bootloader_Script\\Finch_2_3.bin","rb") as f:
with open("HatchlingMain.bin","rb") as f:
	block = f.read(blocksize)
	str   = "#include<stdlib.h>\n#include<stdint.h>\n"
	str   += "const uint8_t PROG_MEMORY[] = {"
	str   += "\n"
	for ch in block:
		try:
			count = count +1
			#total_count = total_count +1
			str += hex(ord(ch))+",\t"
			if(count == 16) :
				count = 0
				str += "\n"
		except:
			#print total_count
			print "End of the file"
	#print total_count
	#print str
	str += "\n }; ";
f.close()

#f = open("C:\\Users\\raghu\\Dropbox (BirdBrain Tech)\\Firmware\\Firmware\\HummingbirdBit_Finch\\Microbit\\Bootloader\\Finch\\Bootloader_Script\\PROG_data.h","w+")
f = open("PROG_data.h","w+")
f.write(str)
f.close()


#Store the in .c File


