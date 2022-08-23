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
#define DRV8833_IN1 10
#define DRV8833_IN2 9

/* -------------------------------- libraries ------------------------------- */

#include <LiquidCrystal.h> // for the LCD
#include "CMBMenu.hpp"

// timer to handle the routine, explained later in mainStart(...) function
#include <Simpletimer.h>

/*
 * Library developed by Kalinin Edward
 * http://mypractic.com/ 
 * Lesson 18. 
 * 
 * I only added a method to check if a pin is enabled or not
 */
#include "MatrixKey.h"
#include <MsTimer2.h> // to use the MatrixKey library: it updates the status periodically

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

DRV8833 motor_driver (DRV8833_IN1, DRV8833_IN2);

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
const char settings_dir_interval_str[] PROGMEM = {"Change dir int."};
const char settings_soft_start_str[] PROGMEM = {"Soft start"};
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
  SettingsDir,
  SettingsDirInterval,
  SettingsSoftStart
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

void setup_motor_driver (void) {
  motor_driver.hardware_setup(); // All done inside the library, poor design choices YOLOOOO
}

void timerInterrupt() {
  keys.scanState(); // scan matrix
}

/* -------------------------------- variables ------------------------------- */

/*
 * Timer0 is used for micros(), millis(), delay(), etc and can't be used
 * Timer1 is used to generate hardware pwm for the DRV8833 board
 * Timer2 is used for the keypad
 * 
 * since the timing doesn't have to be precise for the routine (the error is in the order of ms,
 * while once the job starts it can last several minutes) we can simply use millis() and so on
 * in the main loop to check if the job needs to be terminated or not.
 * The library SimpleTimer internally uses millis() and micros()
 */
bool job_running = false;
long running_time = 0;
Simpletimer jobTimer; // timer to handle the whole procedure

const int update_interval = 1000; // 1000 ms
long starting_time = 0;
long elapsed_time = 0;
long job_percent = 0;
Simpletimer lcdUpdateTimer; // timer to update the percentage on the lcd

// job duration
int time_m = 1;
int time_delta = 1;
int min_time = 1;
int max_time = 60;

// speed
int current_speed = 50;
const int speed_delta = 10;
int min_speed = 10;
int max_speed = 100;

// soft start
Simpletimer softStartTimer; // timer to increase speed for a soft start
bool soft_start = true;
int soft_start_speed = 10;
int soft_start_delta = 5;

// change direction
Simpletimer changeDirTimer; // timer to periodically change direction
bool change_dir = true;
int min_dir_interval = 1; // 1 minute
int change_dir_delta = 1;
long change_dir_interval = min_dir_interval;

void setup() {
  
  // open the serial line
  Serial.begin(115200);

  // setup all the stuff
  setup_lcd();
  setup_keypad();
  setup_motor_driver();

  // add nodes to menu (layer, string, function ID)
  menu.addNode(0, main_menu_str , MainMenu);
  menu.addNode(1, main_start_str, MainStart);
  menu.addNode(1, main_settings_str, MainSettings);
  menu.addNode(2, settings_time_str, SettingsTime);
  menu.addNode(2, settings_speed_str, SettingsSpeed);
  menu.addNode(2, settings_direction_str, SettingsDir);
  menu.addNode(2, settings_dir_interval_str, SettingsDirInterval);
  menu.addNode(2, settings_soft_start_str, SettingsSoftStart);
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

  if ((fid != 0) && (KeyEnter == key) && (!layerChanged)) {
    switch (fid) {
      
      case MainBack:
      case SettingsBack:

        back(info);
        break;

      case MainStart:
        mainStart(info);
        break;

      case SettingsSpeed:

        /*
         * we are now stuck in another cycle until enter is pressed again
         */
        settingsSpeed (info);
        break;

      case SettingsTime:

        settingsTime (info);
        break;

      case SettingsDir:

        settingsDir (info);
        break;

      case SettingsSoftStart:

        settingsSoftStart(info);
        break;

      case SettingsDirInterval:

        settingsDirInterval(info);
        break;

      case MainInfo:

        mainInfo (info);
        break;

      default:
        break;
    }
  }

  if (job_running && jobTimer.timer(running_time)){
    callback();
    job_running = false;
    printLines("Job done", "100 %");
    keys.enable();
    
    Serial.print("Job ended after ");
    Serial.print(running_time / 1000);
    Serial.println(" milliseconds");
  }

  /*
   * soft start starts from min speed and increases it until reaching
   * user input speed
   */
  if (job_running && soft_start && softStartTimer.timer(1000)) {
    if (soft_start_speed + soft_start_delta <= current_speed) {
      soft_start_speed += soft_start_delta;
      motor_driver.set_speed(soft_start_speed);

      Serial.print("Speed increased: ");
      Serial.print(soft_start_speed);
      Serial.print(" -> ");
      Serial.print(current_speed);
      Serial.println();
    }
  }

  if (job_running && lcdUpdateTimer.timer(update_interval)) {
    // percent : 100 = elapsed : total
    // percent = (elapsed * 100) / total
    // total = running_time
    elapsed_time = millis() - starting_time;
    job_percent = (elapsed_time * 100) / running_time;
    printLines("Job running...", String(job_percent) + " %");

    Serial.print("Elapsed time: ");
    Serial.print(elapsed_time);
    Serial.println(" ms");

    //Serial.print("Current job percent: ");
    //Serial.print(job_percent);
    //Serial.println(" %");
  }

  if (job_running && change_dir && changeDirTimer.timer(change_dir_interval * 60 * 1000)) {
    motor_driver.halt();
    delay(500); // so the motor doesn't complain
    motor_driver.invert_direction();

    Serial.println("Swapping direction");
  }
}

/* ---------------------------- utility functions --------------------------- */

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

/*
 * clears the lcd and prints the strings, one on each line
 */
void printLines (String str1, String str2) {
  
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(str1);
  lcd.setCursor(0, 1);
  lcd.print(str2);  
}

void printLines (String str, int value) {

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(str);
  lcd.setCursor(0, 1);
  lcd.print(value);
}

/* ----------------------------- menu functions ----------------------------- */

void back (const char*& info) {
  
  // if the action is "back", then go back duh wtf
  menu.exit();
  
  // update the menu
  menu.getInfo(info);
  printMenuEntry(info);
}

void mainInfo(const char*& info) {

  printLines("Info", ">:( go away");

}

/*
 * TODO: store the speed
 */
void settingsSpeed(const char*& info) {

  printLines("Select speed:", current_speed);

  // determine pressed key
  KeyType key;

  while ((key = getKey()) != KeyEnter) {

    if (key == KeyLeft && current_speed - speed_delta >= min_speed) {
      current_speed -= speed_delta;
      printLines("Select speed:", current_speed);
    } else if (key == KeyRight && current_speed + speed_delta <= max_speed) {
      current_speed += speed_delta;
      printLines("Select speed:", current_speed);
    }
  }

  Serial.print("Speed set to ");
  Serial.print(current_speed);
  Serial.println(" %");

  /*
   * at low speed the plate can't move
   */
  current_speed = map(current_speed, min_speed, max_speed, 40, 100);
  
  back(info); // go back and update the menu
}

/*
 * set the spinning time (minutes); 
 * min = 1 minute
 * max = 60 minutes
 * 
 * TODO: store the duration
 */
void settingsTime(const char*& info) {

  printLines("Select time (m):", time_m);

  // determine pressed key
  KeyType key;

  while ((key = getKey()) != KeyEnter) {

    if (key == KeyLeft && time_m - time_delta >= min_time ) {
      time_m-= time_delta;
      printLines("Select time (m):", time_m);
    } else if (key == KeyRight && time_m + time_delta <= max_time) {
      time_m += time_delta;
      printLines("Select time (m):", time_m);
    }
  }

  Serial.print("Time set to ");
  Serial.print(time_m);
  Serial.println(" minute(s)");

  back(info); // go back and update the menu
}

/*
 * change spinning direction each n seconds (default every minute)
 * 
 */
void settingsDir(const char*& info) {

  printLines("Change dir?", change_dir ? "YES" : "NO");

  // determine pressed key
  KeyType key;

  while ((key = getKey()) != KeyEnter) {
    if (key == KeyLeft && change_dir == false) {
      change_dir = true;
      printLines("Change dir?", "YES");
    } else if (key == KeyRight && change_dir == true) {
      change_dir = false;
      printLines("Change dir?", "NO");
    }
  }

  Serial.print("Change dir?");
  Serial.println(change_dir ? "YES" : "NO");

  back(info); // go back and update the menu
}

/*
 * change direction periodically every n minutes/seconds
 */
void settingsDirInterval(const char*& info) {

  printLines("Select interval (m):", change_dir_interval);

  // determine pressed key
  KeyType key;

  while ((key = getKey()) != KeyEnter) {

    if (key == KeyLeft && change_dir_interval - change_dir_delta >= min_dir_interval ) {
      change_dir_interval -= change_dir_delta;
      printLines("Select interval (m):", change_dir_interval);
    } else if (key == KeyRight && change_dir_interval + change_dir_delta <= time_m) {
      change_dir_interval += change_dir_delta;
      printLines("Select time (m):", change_dir_interval);
    }
  }

  Serial.print("Swapping time set to ");
  Serial.print(change_dir_interval);
  Serial.println(" minute(s)");

  back(info); // go back and update the menu
}

/*
 * starts with low speed and slowly increase it
 */
void settingsSoftStart(const char*& info) {

  printLines("Soft start?", soft_start ? "YES" : "NO");

  // determine pressed key
  KeyType key;

  while ((key = getKey()) != KeyEnter) {
    if (key == KeyLeft && soft_start == false) {
      soft_start = true;
      printLines("Soft start", "YES");
    } else if (key == KeyRight && soft_start == true) {
      soft_start = false;
      printLines("Soft start", "NO");
    }
  }

  Serial.print("Soft start?");
  Serial.println(soft_start ? "YES" : "NO");

  back(info); // go back and update the menu
}

void callback(){
  motor_driver.halt();
}

void mainStart (const char*& info) {

  job_running = true;
  running_time = 60 * (long)time_m * 1000; // ms
  starting_time = millis();

  keys.disable();

  if (soft_start)
    motor_driver.forward(current_speed);
  else
    motor_driver.forward(min_speed);
  
  Serial.print("Starting job (");
  Serial.print(running_time);
  Serial.println(") ms");
}
