/******************************************************************************
*
* Copyright (C) 2009 - 2014 Xilinx, Inc.  All rights reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* Use of the Software is limited solely to applications:
* (a) running on a Xilinx device, or
* (b) that interact with a Xilinx device through a bus or interconnect.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
* OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* Except as contained in this notice, the name of the Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Xilinx.
*
******************************************************************************/

/*
 *
 *
 * This application configures UART 16550 to baud rate 9600.
 * PS7 UART (Zynq) is not initialized by this application, since
 * bootrom/bsp configures it to baud rate 115200
 *
 * ------------------------------------------------
 * | UART TYPE   BAUD RATE                        |
 * ------------------------------------------------
 *   uartns550   9600
 *   uartlite    Configurable only in HW design
 *   ps7_uart    115200 (configured by bootrom/bsp)
 */

// Main program for exercise

//****************************************************
//By default, every output used in this exercise is 0
//****************************************************
#include <stdio.h>
#include "platform.h"
#include "xil_printf.h"
#include "sleep.h"
#include "xgpiops.h"
#include "xttcps.h"
#include "xscugic.h"
#include "xparameters.h"
#include "Pixel.h"
#include "Interrupt_setup.h"

//********************************************************************
//***************TRY TO READ COMMENTS*********************************
//********************************************************************



//Comment this if you want to disable all interrupts
#define enable_interrupts
/***************************************************************************************

Brief description:

Ufo shooter game where you can move ship to left and right from BTN 2 and BTN 1. You can shoot bullets
from ship with BTN3. Bullets are supposed to hit alien moving from left to right. Hits increase score. When alien is hit 8 times
game ends and shows a checkmark on the screen. Game can be restarted at any given time from switch SW1. Score is updated to
leds above buttons. One led is lit every 2 points gained. Impacts on alien are shown as quickly blinking red light.

*****************************************************************************************/
// Function to turn off all led lights on the screen.

// Counter variable for tickhandler.
int tick_counter = 0;

// Ship coordinate.
int ship_coord = 3;

// Current bullet y coordinate.
int bullet_y_coord = 5;

// Current bullet x coordinate.
int bullet_x_coord = 3;

// Alien coordinate
int alien_coord = 0;

// Counter to move alien across the led matrix.
int alien_counter = 0;

// Current points for player.
int points = 0;

// "Bool" for bullet movement
int bullet_is_moving = 0;

// "Bool" for checking if game is over.
int game_is_over = 0;

// Function to clear all pixels of colors.
void clear_all_pixels()
{
	for (uint8_t x = 0; x < 8; x++)
	{
		for (uint8_t y = 0; y < 8; y++)
		{
			SetPixel(x, y, 0, 0, 0);
		}
	}
}

// Useless function to clear ship pixels (could be done much better way)
void clear_ship_pixels(){
	for (uint8_t x = 0; x<8; x++)
		    {
		    	for (uint8_t y = 0; y<2; y++)
		    	{
		    		SetPixel(x,7-y,0,0,0);
		    	}
		    }
}
// Function to draw ship again (or if position changes).
void update_ship(uint8_t x)
{
	if (game_is_over == 1)
	{
		return;
	}
	clear_ship_pixels();
	SetPixel(x, 7, 255, 0, 255);
	SetPixel(x, 6, 255, 0, 255);
	SetPixel(x-1, 7, 255, 0, 255);
	SetPixel(x+1, 7, 255, 0, 255);
}
// Moves ship to right if possible.
void move_ship_right()
{
	if (ship_coord > 5)
	{
		return;
	}
	ship_coord++;
	update_ship(ship_coord);
	// SetPixel(ship_coord, 7, 255, 0, 0);

}
// Moves ship left if possible.
void move_ship_left()
{
	if (ship_coord < 2)
	{
		return;
	}
	ship_coord--;
	update_ship(ship_coord);
}
// Shoots bullets if there isn't bullet already traveling.
void ship_shoot()
{
	if (bullet_is_moving == 1 || game_is_over == 1)
	{
		return;
	}
	bullet_is_moving = 1;
	bullet_x_coord = ship_coord;
	bullet_y_coord = 5;
	SetPixel(ship_coord, bullet_y_coord, 255, 100, 100);

}
// Function to check if bullet hits alien.
void check_for_hit()
{
	if (bullet_y_coord == 0 && bullet_x_coord == alien_coord)
	{
		SetPixel(bullet_x_coord, bullet_y_coord, 255, 0, 0);
		points++;
		if (points == 8)
		{
			game_is_over = 1;
		}
	}
	if (bullet_y_coord == 0)
	{
		bullet_is_moving = 0;
		bullet_y_coord = 5;
	}

}
// Update position of bullet.
void update_bullet()
{
	SetPixel(bullet_x_coord, 0, 0, 0, 0);
	if (bullet_y_coord == 0)
	{
		SetPixel(bullet_x_coord, bullet_y_coord, 0, 0, 0);
	}
	SetPixel(bullet_x_coord, bullet_y_coord, 0, 0, 0);
	if (bullet_y_coord > 0 && bullet_is_moving == 1)
	{
		bullet_y_coord--;
		SetPixel(bullet_x_coord, bullet_y_coord, 255, 100, 100);
	}
}
// Move alien to right or left, depending on position and counter number size.
void move_alien()
{
	alien_counter++;
	if (alien_counter <= 7)
	{
		alien_coord++;
		SetPixel(alien_coord-1, 0, 0, 0, 0);
		SetPixel(alien_coord, 0, 255, 255, 0);
	}
	if (alien_counter >= 8)
	{
		alien_coord--;
		SetPixel(alien_coord+1, 0, 0, 0, 0);
		SetPixel(alien_coord, 0, 255, 255, 0);
	}
	if (alien_counter == 14)
	{
		alien_counter = 0;
	}
	check_for_hit();
}

// Update score to button leds.
void update_score()
{
	uint8_t *led = 0x41200000;
	switch(points)
	{
	case 2: *led |= 0b00001000; break;
	case 4: *led |= 0b00000100; break;
	case 6: *led |= 0b00000010; break;
	case 8: *led |= 0b00000001; break;
	}
}

// Clear button leds of colors.
void clear_leds()
{
	uint8_t *led = 0x41200000;
	*led &= ~0b00001000;
	*led &= ~0b00000100;
	*led &= ~0b00000010;
	*led &= ~0b00000001;
}

// Restart game from switch SW1.
void restart_game()
{
	clear_all_pixels();

	tick_counter = 0;
	ship_coord = 3;
	bullet_y_coord = 5;
	bullet_x_coord = 3;
	alien_coord = 0;
	alien_counter = 0;
	points = 0;
	bullet_is_moving = 0;
	game_is_over = 0;
	update_ship(ship_coord);
	SetPixel(0, 0, 255, 255, 0);
	clear_leds();
}

// Draw checkmark to tell game is won.
void draw_winning_screen()
{
	SetPixel(6, 2, 0, 255, 0);
	SetPixel(6, 3, 0, 255, 0);
	SetPixel(5, 3, 0, 255, 0);
	SetPixel(5, 4, 0, 255, 0);
	SetPixel(4, 4, 0, 255, 0);
	SetPixel(4, 5, 0, 255, 0);
	SetPixel(3, 5, 0, 255, 0);
	SetPixel(3, 6, 0, 255, 0);
	SetPixel(3, 5, 0, 255, 0);
	SetPixel(2, 5, 0, 255, 0);
	SetPixel(2, 4, 0, 255, 0);
	SetPixel(3, 5, 0, 255, 0);
	SetPixel(4, 5, 0, 255, 0);
	SetPixel(1, 4, 0, 255, 0);

}

// End game if conditions are met.
void end_game()
{
	if (points < 8)
	{
		return;
	}
	SetPixel(alien_coord, 0, 0, 0, 0);
	clear_all_pixels();
	draw_winning_screen();
	clear_leds();

}
int main()
{
	    init_platform();
		


#ifdef	enable_interrupts
	    init_interrupts();
#endif


	    //setup screen
	    setup();

	    update_ship(ship_coord);
	    SetPixel(0, 0, 255, 255, 0);

	    Xil_ExceptionEnable();




		while(1){


		}


		cleanup_platform();
		return 0;
}


//Timer interrupt handler for led matrix update. Frequency is 800 Hz
void TickHandler(void *CallBackRef){

	uint32_t StatusEvent;

	//exceptions must be disabled when updating screen
	Xil_ExceptionDisable();

	if (tick_counter == 8)
	{
		tick_counter = 0;
	}

	open_line(8);
	run(tick_counter);
	open_line(tick_counter);
	tick_counter++;

	StatusEvent = XTtcPs_GetInterruptStatus((XTtcPs *)CallBackRef);
	XTtcPs_ClearInterruptStatus((XTtcPs *)CallBackRef, StatusEvent);

	//enable exceptions
	Xil_ExceptionEnable();
}


//Timer interrupt for moving alien, shooting... Frequency is 10 Hz by default
void TickHandler1(void *CallBackRef){

	uint32_t StatusEvent;

	if(game_is_over != 1)
	{
		update_bullet();
		move_alien();
		update_score();
	}
	else
	{
		end_game();
	}

	StatusEvent = XTtcPs_GetInterruptStatus((XTtcPs *)CallBackRef);
	XTtcPs_ClearInterruptStatus((XTtcPs *)CallBackRef, StatusEvent);

}


//Interrupt handler for switches and buttons.
//Reading Status will tell which button or switch was used
//Bank information is useless in this exercise
void ButtonHandler(void *CallBackRef, u32 Bank, u32 Status){

	//If true, btn0 was used to trigger interrupt
	if(Status==0x02)
	{
		move_ship_right();
	}
	if(Status==0x04)
	{
		move_ship_left();
	}
	if(Status==0x08)
	{
		ship_shoot();
	}
	if(Status==0x20)
	{
		restart_game();
	}
}
