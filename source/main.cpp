/* Program used for bootloading firmware onto the SAMD daughter controllers of the Finch 2, Hummingbird Bit, and Hatchling. 
   Authors: Tom Lauwers and Raghu Jangam  */

#include "MicroBit.h"
#include "nrf_delay.h"
#include "PROG_data.h"

#define CONFIRM_BL_FINCH                0x55
#define VERIFY_COMMAND                  '#'                   //Verify command
#define VERIFY_FLASH_COMMAND            'v'										//puts SAMD verification mode
#define ERASE_COMMAND                   'e'										//Erase SAMD flash
#define PROGRAM_COMMAND                 'p'										//Start programming
#define RESET_COMMAND                   'r'                   //reset the device
#define SAMD10_BLOCK_SIZE                64										//block size //page size of SAMD
#define PUTBL_COMMAND                    0x55									//Send this command to put SAMD into bootloader mode
#define POINT_INIT_COMMAND               'm'									//Initialize memory pointers
#define CONFIRM_COMMON 					 's'									//Common confirmation after sending each page , or reading each page


#define NO_LED_ROWS                      5										//Total number of LED rows
/*******************************************************************/
/*********************   Variables    ******************************/
/*******************************************************************/
volatile uint32_t count_data = 0;

MicroBit uBit;

MicroBitSerial serial(MICROBIT_PIN_P13, MICROBIT_PIN_P14, 255, 255); 

// Used for printing the progress while programming
MicroBitImage row1img("255,255,255,255,255\n0,0,0,0,0\n0,0,0,0,0\n0,0,0,0,0\n0,0,0,0,0\n");
MicroBitImage row2img("255,255,255,255,255\n255,255,255,255,255\n0,0,0,0,0\n0,0,0,0,0\n0,0,0,0,0\n");
MicroBitImage row3img("255,255,255,255,255\n255,255,255,255,255\n255,255,255,255,255\n0,0,0,0,0\n0,0,0,0,0\n");
MicroBitImage row4img("255,255,255,255,255\n255,255,255,255,255\n255,255,255,255,255\n255,255,255,255,255\n0,0,0,0,0\n");
MicroBitImage row5img("255,255,255,255,255\n255,255,255,255,255\n255,255,255,255,255\n255,255,255,255,255\n255,255,255,255,255\n");

MicroBitImage check("0,0,0,0,0\n0,0,0,0,255\n0,0,0,255,0\n255,0,255,0,0\n0,255,0,0,0\n");
MicroBitImage ximg("255,0,0,0,255\n0,255,0,255,0\n0,0,255,0,0\n0,255,0,255,0\n255,0,0,0,255\n");


/*******************************************************************/
//Print the 'X' symbol on the LED Matrix screen 
//Wait there
/*******************************************************************/
void wrong_LEDarray()
{
    uBit.display.print(ximg);

	while(1)
	{
        uBit.sleep(10);
    }
}
/*******************************************************************/

/*******************************************************************/
//Check if confirm common command value is received , if not print X on the screen
/*******************************************************************/
void check_confirm(uint8_t return_value)
{
	if(return_value != CONFIRM_COMMON)
	{
			wrong_LEDarray();
	}
}

// Function to clear out the serial buffer - something wonky is happening with serial communications
void clear_buffer()
{
    int count = 0;
	while(serial.read(ASYNC) != MICROBIT_NO_DATA)
    {
        uBit.sleep(6);
        count++;
    }
}

/*******************************************************************/
//Ask information about the Application size that is acceptable
//This mostly for double checking if the SAMD is in bootloader mode
/*******************************************************************/
uint8_t send_verify()
{
	uint8_t  received_value[2];
    uint8_t count = 0;
	serial.sendChar(VERIFY_COMMAND, SYNC_SPINWAIT);

    received_value[0] = serial.read(SYNC_SLEEP); // Should send back an 's' - decimal 115 or hex 73

    // Read until you get a confirmation - oddly the Finch SAMD bootloader seems to be sending extra bytes
    while(received_value[0] != CONFIRM_COMMON)
    {
        received_value[0] = serial.read(SYNC_SLEEP); 
        count++;
        if(count > 250)
            return 0;
    }

    /* Not used, but a way to error check that our program isn't too big
	app_size = sizeof(PROG_MEMORY);
	if((received_value[1]*1024) < app_size)
	{
			
	}*/
	return received_value[0];
}
/*******************************************************************/

/*******************************************************************/
//Send the Erase command over to SAMD
/*******************************************************************/
void send_erase()
{
	//uint8_t received_value = 0;
    //uint8_t count = 0;
    serial.sendChar(ERASE_COMMAND, SYNC_SLEEP);
    //received_value = serial.read(SYNC_SLEEP);

    // Read until you get a confirmation - oddly the Finch SAMD bootloader seems to be sending extra bytes
    /*while(received_value != CONFIRM_COMMON)
    {
        received_value = serial.read(SYNC_SLEEP);
        count++;
        if(count > 250)
            return 0;        
    }
    return received_value;*/

}
/*******************************************************************/

/*******************************************************************/
//Reset SAMD, which basically turns off the entire system
/*******************************************************************/
void reset_controller()
{
	serial.sendChar(RESET_COMMAND);
}
/*******************************************************************/

/*******************************************************************/
//Puts SAMD into programming mode
/*******************************************************************/
void send_program()
{
    serial.sendChar(PROGRAM_COMMAND, SYNC_SLEEP);
}
/*******************************************************************/

/*******************************************************************/

/*******************************************************************/
//Sets the memory pointers on SAMD
/*******************************************************************/
uint8_t send_point_init()
{
	uint8_t received_value;
    uint8_t count = 0;
    serial.sendChar(POINT_INIT_COMMAND, SYNC_SPINWAIT);

    received_value = serial.read(SYNC_SLEEP);

    // Read until you get a confirmation - oddly the Finch SAMD bootloader seems to be sending extra bytes
    while(received_value != CONFIRM_COMMON)
    {
        received_value = serial.read(SYNC_SLEEP);
        count++;
        if(count > 250)
            return 0;
    }
	return received_value;
}

/*******************************************************************/
//Put SAMD in bootloader mode by sending PUTBL_COMMAND
//Later wait if the expected value is returned
/*******************************************************************/
void send_putBL()
{
	serial.sendChar(PUTBL_COMMAND, SYNC_SPINWAIT);
    // May need to wait here before moving to the next step
    uBit.sleep(50);
	
}


/*******************************************************************/
//Send program command , wait for confirmation 
//Send 64 bytes at a time
/*******************************************************************/
void program_flash_block()
{
    uint16_t block_size = SAMD10_BLOCK_SIZE;
    uint16_t  i          = 0;

    //Send program command    
    send_program();

    //Start sending data block by block
    for(i=0;i<block_size;i++)
    {
        while(!serial.isWriteable());

        serial.sendChar(PROG_MEMORY[count_data], SYNC_SLEEP);
        count_data++;
        nrf_delay_ms(1); // Probably be a bug, but the program does not copy properly if there is no delay between character sends
    }
    clear_buffer();

}
/*******************************************************************/


/*******************************************************************/
//Send program command , wait for confirmation
//Send the useful bytes which are less than 64 
//later send 0xff into the remainder bytes
/*******************************************************************/
void program_flash_last_block(uint8_t rem_block_data)
{
    uint16_t block_size = SAMD10_BLOCK_SIZE;
    uint16_t  i          = 0;

    //Send program command
    send_program();

    //Start sending data 
    for(i=0;i<rem_block_data;i++)
    {
        while(!serial.isWriteable());
        serial.sendChar(PROG_MEMORY[count_data], SYNC_SLEEP);
        count_data++;
        nrf_delay_ms(1);
    }
    for(i=rem_block_data;i<block_size;i++)
    {
        while(!serial.isWriteable());
        serial.sendChar(0xFF, SYNC_SLEEP);
        count_data++;      
        nrf_delay_ms(1);  
    }
    /* Error check - not used
    return_value = serial.read(SYNC_SLEEP);
    while(return_value != CONFIRM_COMMON)
    {
        return_value = serial.read(SYNC_SLEEP);
        count++;
        if(count > 250)
            break;       
    }     
    check_confirm(return_value);*/
    clear_buffer();
}


/*******************************************************************/
//Program Flash
//Program 64 bytes at a time . that is page size of SAMD
//Update LED Matrix with progress of programming
//At the end if you have any extra bytes , that total bytes are divisible by 64
/*******************************************************************/
void program_flash()
{
	volatile uint32_t program_size = sizeof(PROG_MEMORY);   // Approximately 24212 bytes right now (9.13.2023)
    volatile uint16_t NO_blocks  = 0; // Total 64 byte blocks
	uint16_t rem_blocks = 0;          // Remainder of bytes in last block
	uint16_t i          = 0;
	uint8_t  LED_update = 0;
    uint8_t  led_count  = 0;

    count_data = 0;
	NO_blocks  = program_size/64;
	rem_blocks = program_size%64;
	LED_update = NO_blocks/NO_LED_ROWS;	  //Each LED represents certain number of blocks being programmed
	for(i=0;i<NO_blocks;i++)
	{
		//Program each block
		program_flash_block();
		//Update the LED screen every time certain number of blocks have been programmed
		if(i%LED_update == 0 )
		{
			if(led_count == 0)
            {
                uBit.display.print(row1img);
            }
			else if(led_count == 1)
            {
                uBit.display.print(row2img);
            }
			else if(led_count == 2)
            {
                uBit.display.print(row3img);
            }
			else if(led_count == 3)
            {
                uBit.display.print(row4img);
            }
			else if(led_count == 4)
            {
                uBit.display.print(row5img);
            }
            led_count++;
		}
	}
	program_flash_last_block(rem_blocks);
}


/*******************************************************************/
//Puts SAMD into verification mode
/*******************************************************************/
void send_verify_flash()
{
    
    serial.sendChar(VERIFY_FLASH_COMMAND, SYNC_SLEEP);
}



/*******************************************************************/
//Send Verify command and receive 64 bytes of data at a time
/*******************************************************************/
void verify_flash_block()
{
	  uint8_t return_value = 0;
	  //uint8_t received_value[2];
	  
	  uint16_t block_size = 0;
//	  uint16_t NO_blocks  = 0;
//	  uint16_t rem_blocks = 0; 
		
	  uint16_t  i          = 0;
	  block_size = SAMD10_BLOCK_SIZE; 
	  //Send verify command
		send_verify_flash();
        return_value = serial.read(SYNC_SLEEP);
		//Wait for ackownledgment
		check_confirm(return_value);
		for(i=0;i<block_size;i++)
		{
			//Read 64 bytes received and compare it against the Prog memory variables which is used in programming
			return_value = serial.read(SYNC_SLEEP);
			if(return_value != PROG_MEMORY[count_data] )
			{
				wrong_LEDarray();
			}
			count_data++;
            nrf_delay_ms(1); // Probably be a bug, but the program does not copy properly if there is no delay between character sends
		}
        clear_buffer();
		
}
/*******************************************************************/

/*******************************************************************/
//Send verification 
//Send the left over bytes over UART and verify them
/*******************************************************************/
void verify_flash_last_block(uint8_t rem_block_data)
{
	  uint8_t return_value = 0;
	  //uint8_t received_value[2];
	  uint16_t block_size = 0;
	  uint16_t  i          = 0;
	  block_size = SAMD10_BLOCK_SIZE;
	  
	  //similar to generic verify block but here we stop with few bytes in the last block
		send_verify_flash();
        return_value = serial.read(SYNC_SLEEP);
		check_confirm(return_value);
		for(i=0;i<rem_block_data;i++)
		{
			return_value = serial.read(SYNC_SLEEP);
			if(return_value != PROG_MEMORY[count_data] )
			{
				wrong_LEDarray();
			}
			count_data++;
            nrf_delay_ms(1); // Probably be a bug, but the program does not copy properly if there is no delay between character sends
			
		}
		//Reamining bytes to complete the block size
		for(i=rem_block_data;i<block_size;i++)
		{
			//Check if the block values are 0xFF , which should be that as we erased initially
			return_value = serial.read(SYNC_SLEEP);
			if(return_value != 0xff )
			{
				wrong_LEDarray();
			}
			count_data++;
            nrf_delay_ms(1); // Probably be a bug, but the program does not copy properly if there is no delay between character sends
		}
        clear_buffer();
}

/*******************************************************************/
//Verify flash 
//Read 64 bytes at a time that is the page size of SAMD Flash
//Update the LED Matrix with the progress
//At the end if you have any extra bytes fill it at the end
/*******************************************************************/
void verify_flash()
{
	uint32_t program_size = sizeof(PROG_MEMORY);
    uint16_t NO_blocks  = 0;
	uint16_t rem_blocks = 0;
	uint16_t  i 				  =	0;	
	uint8_t  LED_update = 0;
    uint8_t  led_count  = 0;

	NO_blocks  = program_size/64;
	rem_blocks = program_size%64;
	count_data = 0;
	LED_update = NO_blocks/NO_LED_ROWS; //Each LED represnts certain number of blocks being verified
    
	for(i=0;i<NO_blocks;i++)
	{
		//read each block
		verify_flash_block();
		//Update the LED screen every time certain number of blocks have been verified
		if(i%LED_update == 0 )
		{
			if(led_count == 0)
            {
                uBit.display.print(row1img);
            }
			else if(led_count == 1)
            {
                uBit.display.print(row2img);
            }
			else if(led_count == 2)
            {
                uBit.display.print(row3img);
            }
			else if(led_count == 3)
            {
                uBit.display.print(row4img);
            }
			else if(led_count == 4)
            {
                uBit.display.print(row5img);
            }
            led_count++;
		}
	}
	//Last few bytes which are less than 64 bytes
	verify_flash_last_block(rem_blocks);
}

int main()
{
    uint8_t return_value = 0; // Value to get from the function that puts the Finch in bootloader mode

    // Just in case it isn't already 115200
    serial.setBaud(115200);
 
    // Enter bootloader mode
    send_putBL();
    //Now that we have enter the bootloader mode of SAMD , just verify that it is responding
    return_value = send_verify();
    check_confirm(return_value);
   
    // Initialize the pointer on the SAMD
    return_value = send_point_init();
    check_confirm(return_value);

    // Erase the current program
    send_erase();
    uBit.display.print('E');
    uBit.sleep(1000);       
    // Print P and enter programming mode
    uBit.display.print('P');
    uBit.sleep(1000);
    program_flash();
    // Got into verify mode
   // uBit.display.print('V');

   // uBit.sleep(1000);
    // Old V1 code would verify that we programmed things correctly - but we no longer check since it seems to pretty much always work

	//read the data page by page
	//verify_flash();
    uBit.display.print(check);
    uBit.sleep(1000);
    reset_controller();
}

