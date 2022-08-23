#include "DRV8833.h"
#include <TimerOne.h> // PWM generation
#include "Arduino.h"

DRV8833::DRV8833 (int IN_1, int IN_2, int EEP):
  _IN_1(IN_1), _IN_2(IN_2), _EEP(EEP) {}

DRV8833::DRV8833 (int IN_1, int IN_2):
  _IN_1(IN_1), _IN_2(IN_2), _EEP(255) {}

/*
 * forward:
 * 1   pwm   forward, slow decay
 * pwm   0   forward, fast decay
 */
void DRV8833::forward (int speed_percent) {

  _input_speed = speed_percent;
  _normalized_speed = normalize_speed(speed_percent);
  _direction = Direction::FORWARD;

  Serial.print("Normalized speed: ");
  Serial.println(_normalized_speed);

  if (this -> _decay_mode == Decay::SLOW_DECAY) {
    digitalWrite(_IN_1, HIGH);
    Timer1.pwm(_IN_2, _normalized_speed);
  } else {
    Timer1.pwm(_IN_1, _normalized_speed);
    digitalWrite(_IN_2, LOW);
  }
}

/*
 * bacward:
 * pwm   1   backward, slow decay
 * 0   pwm   backward, fast decay
 */
void DRV8833::backward (int speed_percent) {

  _input_speed = speed_percent;
  _normalized_speed = normalize_speed(speed_percent);
  _direction = Direction::BACKWARD;

  Serial.print("Normalized speed: ");
  Serial.println(_normalized_speed);

  if (this -> _decay_mode == Decay::SLOW_DECAY) {
    Timer1.pwm(_IN_1, _normalized_speed);
    digitalWrite(_IN_2, HIGH);
  } else {
    digitalWrite(_IN_2, LOW);
    Timer1.pwm(_IN_2, _normalized_speed);
  }
}

void DRV8833::halt () {

  Timer1.pwm(_IN_1, 0);
  Timer1.pwm(_IN_2, 0);
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

void DRV8833::invert_direction () {
  if (_input_speed != 0)
    if (_direction == Direction::BACKWARD)
      forward(_input_speed);
    else
      backward(_input_speed);
}

void DRV8833::hardware_setup () {

  // enable the board
  if (_EEP != 255) {
    pinMode(_EEP, OUTPUT);
    digitalWrite (_EEP, HIGH); 
  }

  pinMode(_IN_1, OUTPUT);
  pinMode(_IN_2, OUTPUT);

  Timer1.initialize(500000);

}

int DRV8833::normalize_speed (int input_speed) {

  /* 
   * speed = 0 used to stop the motors
   * but a speed value between 0 and the threshold should be ignored
   */
  if (input_speed == 0)
    return 0;
  
  /*
   * clamp the value in range (threshold, 100)
   */
  if (input_speed > 100)
    input_speed = 100;

  if (input_speed < _speed_threshold)
    input_speed = _speed_threshold;

  /*
   * now we have a value between 10 and 100;
   * this kind of motor seems to move if the duty cycle is between 70% and 100%
   */

  return map(input_speed, _speed_threshold, 100, 70, 100); // (threshold, 100) speed percentage
}
