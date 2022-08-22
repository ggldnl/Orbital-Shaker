#include "DRV8833.h"
// #include <TimerOne.h> // PWM generation
#include "Arduino.h"

DRV8833::DRV8833 (int IN_1, int IN_2, int EEP):
  _IN_1(IN_1), _IN_2(IN_2), _EEP(EEP) {}

DRV8833::DRV8833 (int IN_1, int IN_2):
  _IN_1(IN_1), _IN_2(IN_2), _EEP(255) {}

void DRV8833::forward (int speed_percent) {
  
  int normalized_speed = normalize_speed(speed_percent);
  _direction = Direction::FORWARD;

  if (this -> _decay_mode == Decay::SLOW_DECAY) {
    //Timer1.pwm(IN_1, 100);
    //Timer1.pwm(IN_2, normalized_speed);
  } else {
    //Timer1.pwm(IN_1, normalized_speed);
    //Timer1.pwm(IN_2, 0);
  }
}

void DRV8833::backward (int speed_percent) {
  
  int normalized_speed = normalize_speed(speed_percent);
  _direction = Direction::BACKWARD;

    if (this -> _decay_mode == Decay::SLOW_DECAY) {
      //Timer1.pwm(IN_1, normalized_speed);
      //Timer1.pwm(IN_2, 100);
    } else {
      //Timer1.pwm(IN_1, 0);
      //Timer1.pwm(IN_2, normalized_speed);
    }
}

void DRV8833::halt () {
  
  //Timer1.pwm(IN_1, 0);
  //Timer1.pwm(IN_2, 0);
}

void DRV8833::set_slow_decay () {
  _decay_mode = Decay::SLOW_DECAY;
}

void DRV8833::set_fast_decay () {
  _decay_mode = Decay::FAST_DECAY;
}

void DRV8833::set_speed (int new_speed) {
  if (_direction == Direction::BACKWARD)
    backward(new_speed);
  else
    forward(new_speed);
}

void DRV8833::hardware_setup () {

  // enable the board
  if (_EEP != 255) {
    pinMode(_EEP, OUTPUT);
    digitalWrite (_EEP, HIGH); 
  }
  
  //softPwmCreate(IN_1, 0, 100);
  //softPwmCreate(IN_2, 0, 100);

}

int DRV8833::normalize_speed (int input_speed) {
  
  /*
   * clamp the value in range (threshold, 100)
   */
  if (input_speed > 100)
    input_speed = 100;

  if (input_speed < 0)
    input_speed = _speed_threshold;

  /* 
   * speed = 0 used to stop the motors
   * but a speed value between 0 and the threshold should be ignored
   */
  if (input_speed == 0)
    return 100;

  /*
   * If we set a pwm value of 70 it means that the signal will be high for 70% of the time 
   * and low for the remaining 30. I don't know why but this behaviour results in a 30% speed
   * in our case (with the drv8833).
   * To avoid this, we invert the speed argument: speed -> 100 - speed. In addition to this 
   * the initial value should be an integer between 0 and 100, at least theoretically: from the 
   * observations we get that with a 10% speed the wheel doesn't even move. We can set the limits 
   * between 20 and 100 (operative limit).
   */
  return 100 - input_speed; // (threshold, 100) speed percentage
}
