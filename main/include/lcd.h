#pragma once

#include <stdio.h>
#include <string.h>
#include <sys/time.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "driver/i2c.h"
#include "esp_log.h"
#include "sdkconfig.h"

#include <mcp23x17.h>
#include <hd44780.h>
#include <pcf8574.h>


/* Define the I2C Master SDA pin for the LCD communication */
#define LCD_I2C_MASTER_SDA 18

/* Define the I2C Master SCL pin for the LCD communication */
#define LCD_I2C_MASTER_SCL 23

/* Define the I2C address of the LCD device */
#define LCD_I2C_ADDRESS 0x27

/** 
 * @brief Implements a simple menu for an LCD display using ESP32_LyraT board.
 * 
 * This module provides functionality to display a menu system on an LCD screen
 * using the ESP32_LyraT development board. It utilizes the HD44780 driver 
 * library along with the PCF8574 I2C expander for communication.
 */

// Method Declarations

/**
 * @brief Makes the necesseray configurations for the LCD and uploads custom characters.
 * 
 * This function is setting up the configurations for the LCD module and uploads the custom 
 * characters that are used for the menu. 
*/
void lcd_init();

/**
 * @brief Task function to display the menu on the LCD.
 * 
 * This task function runs in the FreeRTOS environment to manage the menu
 * displayed on the LCD screen.
 * 
 * @param pvParameters Pointer to task parameters (not used).
 */
void menu(void *pvParameters);

/**
 * @brief Writes a string at the specified position on the LCD.
 * 
 * This function writes a string at the specified (x, y) position on the LCD.
 * 
 * @param x X-coordinate position.
 * @param y Y-coordinate position.
 * @param string String to write on the LCD.
 */
void write_string_on_pos(int x, int y, const char *string);

/**
 * @brief Writes a char at the specified position on the LCD.
 * 
 * This function writes a character at the specified (x, y) position on the LCD.
 * 
 * @param x X-coordinate position.
 * @param y Y-coordinate position.
 * @param c Character to write on the LCD.
 */
void write_char_on_pos(int x, int y, char c);

/**
 * @brief Writes a character followed by a string at the specified position on the LCD.
 * 
 * This function writes a character followed by a string at the specified (x, y) position
 * on the LCD screen.
 * 
 * @param x X-coordinate position.
 * @param y Y-coordinate position.
 * @param c Character (icon) to write before the string.
 * @param string String to write after the character.
 * 
 * @example: write_and_upload_char(&lcd, 0, 0, 'C', " Internet Radio");
 * This example writes the character 'C' followed by the string "Internet Radio".
 * The output on the LCD would be: C Internet Radio
 */
void write_and_upload_char(int x, int y, char c,const char *string);

/**
 * @brief Clears the character at the specified position on the LCD.
 * 
 * This function clears a character at the specified (x, y) position on the LCD screen.
 * 
 * @param x X-coordinate position.
 * @param y Y-coordinate position.
 * 
 */
void clear_at_position(int x, int y);

/**
 * @brief Clears a line on the LCD.
 * 
 * This function clears a line at the specified line parameter.
 * Line 0: Page Bar.
 * Line 1: Internet Radio.
 * Line 2: Sampler.
 * Line 3: Tuner.
 * all entries except 0,1,2 and 3 are dismissed.
 * 
 * @param line Specified line to clear
 *
 */
void clear_line(int line);
