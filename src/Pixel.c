/*
 * Pixel.c
 *
 *  Created on: 02.12.2021
 *      Author: nkkasu
 */

#include "Pixel.h"



//Table for pixel dots.
//				 dots[X][Y][COLOR]
volatile uint8_t dots[8][8][3]={0};
volatile uint8_t* control = 0x41220008;
volatile uint8_t* channel = 0x41220000;
// Here the setup operations for the LED matrix will be performed
void setup(){


	//reseting screen at start
	*control &= 0b00000000;
	*channel &= 0b00000000;
	*control |= 0b00000001;
	usleep(500);
	*control &= ~0b00000001;
	usleep(500);
	*control |= 0b00000001;
	usleep(500);
	*control |= 0b00010000;
	//Code that sets 6-bit values in register of DM163 chip. Every bit in that register is set to 1. 6-bits and 24 "bytes".
	//24*6 bits are transmitted
	for (uint8_t a = 0; a < 24; a++)
	{
		uint8_t data = 0b111111;
		for (uint8_t b = 0; b < 6; b++)
		{
			if (data & 0x20)
			{
				*control |= 0b00010000;

			}
			else
			{
				*control &= ~0b00010000;
			}
			*control &= ~0b00001000;
			data <<= 1;
			*control |= 0b00001000;
		}
	}
	//Set SB-bit to 1 to enable transmission to 8-bit register.
	*control |= 0b00000100;

}

//Change value of one pixel at led matrix. This function is only used for changing values of dots array
void SetPixel(uint8_t x,uint8_t y, uint8_t r, uint8_t g, uint8_t b){

	dots[x][y][0]=b;
	dots[x][y][1]=g;
	dots[x][y][2]=r;
}


//Put new data to led matrix.
void run(uint8_t x){

	latch();
	for (uint8_t y = 0; y<8; y++)
	    {
	        for (uint8_t color=0; color < 3; color++)
	        {
	            uint8_t temp = dots[x][y][color];
	            // Read dots array to some temporary variable. This temporary variable is used to send data.
	            for (uint8_t byte_count = 0; byte_count < 8; byte_count++)
	            {
	                if (temp & 0x80)
	                {
	                    *control |= 0b00010000;
	                }
	                else
	                {
	                    *control &= ~0b00010000;
	                }
	                *control &= ~0b00001000;
	                temp <<=1;
	                *control |= 0b00001000;
	            }
	        }
	    }
		latch();
		*control &= ~0b00001000;
}

//Latch signal.
void latch(){
	*control |= 0b00000010;
	*control &= ~0b00000010;
}


//Set one line (channel) as active, one at a time.
void open_line(uint8_t x){
	switch (x){
	    case 0: *channel = 0b00000001; break;
	    case 1: *channel = 0b00000010; break;
	    case 2: *channel = 0b00000100; break;
	    case 3: *channel = 0b00001000; break;
	    case 4: *channel = 0b00010000; break;
	    case 5: *channel = 0b00100000; break;
	    case 6: *channel = 0b01000000; break;
	    case 7: *channel = 0b10000000; break;
	    default: *channel = 0b00000000;
	}
}



