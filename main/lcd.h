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

#include "mcp23x17.h"
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
 * @param lcd Pointer to the hd44780_t LCD structure.
 * @param x X-coordinate position.
 * @param y Y-coordinate position.
 * @param string String to write on the LCD.
 */
void write_string_on_pos(const hd44780_t *lcd, int x, int y, const char *string);

/**
 * @brief Writes a char at the specified position on the LCD.
 * 
 * This function writes a character at the specified (x, y) position on the LCD.
 * 
 * @param lcd Pointer to the hd44780_t LCD structure.
 * @param x X-coordinate position.
 * @param y Y-coordinate position.
 * @param c Character to write on the LCD.
 */
void write_char_on_pos(const hd44780_t *lcd, int x, int y, char c);

/**
 * @brief Writes a character followed by a string at the specified position on the LCD.
 * 
 * This function writes a character followed by a string at the specified (x, y) position
 * on the LCD screen.
 * 
 * @param lcd Pointer to the hd44780_t LCD structure.
 * @param x X-coordinate position.
 * @param y Y-coordinate position.
 * @param c Character (icon) to write before the string.
 * @param string String to write after the character.
 * 
 * @example: write_and_upload_char(&lcd, 0, 0, 'C', " Internet Radio");
 * This example writes the character 'C' followed by the string "Internet Radio".
 * The output on the LCD would be: C Internet Radio
 */
void write_and_upload_char(const hd44780_t *lcd, int x, int y, char c,const char *string);