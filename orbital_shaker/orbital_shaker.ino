//
// GNU General Public License v3.0
//
// Copyright (C) 2022 Daniel Gigliotti
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.

/* --------------------------------- pinout --------------------------------- */

/**
 * pins needed for the 1602A LCD screen
 * 
 * VSS -> GND
 * VDD -> +5v
 * V0 -> al potenziometro
 * RS -> D13
 * R/W -> GND
 * E -> A1
 * D0 -> N/C
 * D1 -> N/C
 * D2 -> N/C
 * D3 -> N/C
 * D4 -> Arduino D12
 * D5 -> Arduino D10
 * D6 -> Arduino D9
 * D7 -> Arduino D8
 * A -> Arduino A0
 * K -> GND
 */
#define LCD_RS 13
#define LCD_E  A1
#define LCD_D4 12
#define LCD_D5 11
#define LCD_D6 8
#define LCD_D7 7
#define LCD_A A0

/**
 * we will need 3 push buttons
 * BTN_ENTER -> D4
 * ENC_LEFT -> D5
 * ENC_RIGHT -> D6
 */
#define BTN_ENTER 3
#define BTN_LEFT 4
#define BTN_RIGHT 5
#define PIN_CTRL 6

/**
 * And we will need two PWM pins to drive the motor.
 * The board will be always enabled when current is supplied
 * DRV8833_EEP -> 5V
 * DRV8833_IN1 -> D3
 * DRV8833_IN2 -> D11
 */
#define DRV8833_IN1 9
#define DRV8833_IN2 10

/* -------------------------------- libraries ------------------------------- */

#include <LiquidCrystal.h> // for the LCD
#include "CMBMenu.hpp"

/*
 * Library developed by Kalinin Edward
 * http://mypractic.com/ 
 * Lesson 18. 
 * 
 * I only added a method to check if a pin is enabled or not
 */
#include "MatrixKey.h"
#include <MsTimer2.h> // to use the library: it updates the status periodically

/*
 * Library developed by *me* :)
 */
#include "DRV8833.h"

/* --------------------------------- objects -------------------------------- */

LiquidCrystal lcd(LCD_RS, LCD_E, LCD_D4, LCD_D5, LCD_D6, LCD_D7);

// vertical line -> pin 7
// horizontal line -> 4, 5, 6
// number of confirmations of the status of contacts = 6
MatrixKeys keys (PIN_CTRL, 255, 255, 255, BTN_ENTER, BTN_LEFT, BTN_RIGHT, 255, 6);

DRV8833 motor_driver(DRV8833_IN1, DRV8833_IN2);

/* ---------------------------------- menu ---------------------------------- */

// create global CMBMenu instance
CMBMenu<100> menu; // max 100 menu entries

/**
 * the menu will have this structure:
 *   initial_screen
 *    back
 *    settings
 *      back
 *      speed
 *      time
 *      change direction (y/n)
 *    start
 */
// PROGMEM -> store the string  into the flash memory
const char main_menu_str[] PROGMEM = {"Orbital Shaker"};
const char main_settings_str[] PROGMEM = {"Settings"};
const char main_start_str[] PROGMEM = {"Start"};
const char settings_speed_str[] PROGMEM = {"Speed"};
const char settings_time_str[] PROGMEM = {"Time"};
const char settings_direction_str[] PROGMEM = {"Direction"};
const char main_info_str[] PROGMEM = {"Info"};
const char back_str[] PROGMEM = {"Back"};

// define function IDs
enum MenuFID {
  MainMenu,
  MainBack,
  MainSettings,
  MainStart,
  MainInfo,
  SettingsBack,
  SettingsSpeed,
  SettingsTime,
  SettingsDir
};

// define key types
enum KeyType {
  KeyNone, // no key is pressed
  KeyLeft,
  KeyRight,
  KeyEnter
};

void setup_lcd (void) {
  
  pinMode(LCD_A, OUTPUT); // A0
  digitalWrite(LCD_A, HIGH); // LCD turned on

  pinMode(A1, OUTPUT); // E (enable)
  digitalWrite(A1, HIGH);

  // (rows, cols)
  lcd.begin(16, 2);

  delay(500);
  lcd.print("Sup bitch");
  delay(1000);
  lcd.clear();
}

void setup_keypad (void) {
  MsTimer2::set(2, timerInterrupt); // set the timer interrupt period 2 ms
  MsTimer2::start(); // enable timer interrupt  
}

void timerInterrupt() {
  keys.scanState(); // scan matrix
}

void setup() {
  
  // open the serial line
  Serial.begin(115200);

  // setup all the stuff
  setup_lcd();
  setup_keypad();

  // add nodes to menu (layer, string, function ID)
  menu.addNode(0, main_menu_str , MainMenu);
  menu.addNode(1, main_start_str, MainStart);
  menu.addNode(1, main_settings_str, MainSettings);
  menu.addNode(2, settings_time_str, SettingsTime);
  menu.addNode(2, settings_speed_str, SettingsSpeed);
  menu.addNode(2, settings_direction_str, SettingsDir);  
  menu.addNode(2, back_str, SettingsBack);
  menu.addNode(1, back_str, MainBack);
  menu.addNode(1, main_info_str, MainInfo);

  // build menu
  const char* info;
  menu.buildMenu(info);

  // print current menu entry
   printMenuEntry(info);
}

void loop() {

  // function ID
  int fid = 0;

  // info text from menu
  const char* info;

  // go to deeper or upper layer?
  bool layerChanged = false;

  // determine pressed key
  KeyType key = getKey();

  // call menu methods regarding pressed key
  switch(key) {
    case KeyEnter:
      menu.enter(layerChanged);
      break;
    case KeyRight:
      menu.right();
      break;
    case KeyLeft:
      menu.left();
      break;
    default:
      break;
  }

  // pint/update menu when key was pressed
  // and get current function ID "fid"
  if (key != KeyNone) {
    fid = menu.getInfo(info);
    printMenuEntry(info);
  }

  // do action regarding function ID "fid"
  if ((fid != 0) && (key == KeyEnter) && (!layerChanged)) {
    switch (fid) {
      case MainBack:
      case SettingsBack:
      
        // go back
        menu.exit();

        // update the menu
        menu.getInfo(info);
        printMenuEntry(info);

        break;
      case SettingsSpeed:
        settingsSpeed();
        break;
      case SettingsTime:
        settingsTime();
        break;
      case SettingsDir:
        settingsDir();
        break;
      case MainInfo:
        mainInfo();
        break;
      default:
        break;
    }
  }
}

/*
 * prints the menu entry on the LCD display
 */
void printMenuEntry(const char* f_Info) {

  String info_s;
  MBHelper::stringFromPgm(f_Info, info_s);

  // print on LCD
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(info_s);
}

KeyType getKey() {
  
  KeyType key = KeyNone;

  // MatrixKeys keys (PIN_CTRL, 255, 255, 255, BTN_ENTER, BTN_LEFT, BTN_RIGHT, 255, 6);
  // BTN_ENTER -> 0
  // BTN_LEFT -> 1
  // BTN_RIGHT -> 2
  if (keys.flagClick[0][0] == true) {
    keys.flagClick[0][0] = false; // consume the click
    key = KeyEnter;
    Serial.println("Pressed: Enter");
  } else if (keys.flagClick[0][1] == true) {
    keys.flagClick[0][1] = false;
    key = KeyLeft;
    Serial.println("Pressed: Left");
  } else if (keys.flagClick[0][2] == true) {
    keys.flagClick[0][2] = false;
    key = KeyRight;
    Serial.println("Pressed: Right");
  }

 return key;
}

/* -------------------------------- functions ------------------------------- */

void mainInfo() {
  // lcd.setCursor(0, 0);
  // lcd.print(">:(");
  lcd.setCursor(0, 1);
  lcd.print(">:( go away");
}

void settingsSpeed() {
  lcd.setCursor(0,1);
  lcd.print("select speed");
}

void settingsTime() {
  lcd.setCursor(0,1);
  lcd.print("select time");
}

void settingsDir() {
  lcd.setCursor(0,1);
  lcd.print("change direction?");
}
