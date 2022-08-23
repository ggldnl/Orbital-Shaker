#ifndef DRV8833_H
#define DRV8833_H

class DRV8833 {

  public:

    DRV8833 (int IN_1, int IN_2, int ENABLE);

    DRV8833 (int IN_1, int IN_2);
 
    /**
     * Move the motor forward at a certain speed (at least it tries to).
     * Values out of the (threshold, 100) range will be clamped to the range bounds.
     * 
     * forward:
     * 1   pwm   forward, slow decay
     * pwm   0   forward, fast decay
     * 
     */
    void forward (int speed);

    /**
    * @brief Move the motor backward at a certain speed (at least it tries to).
    * Values out of the (threshold, 100) range will be clamped to the range bounds
    *
    * pwm   1   backward, slow decay
    * 0   pwm   backward, fast decay
    *
    * @param speed_percent (threshold, 100)
    */
    void backward (int speed);

    /**
     * @brief Change the motor speed keeping the same direction
     */
    void set_speed (int new_speed_percent);

    /**
    * @brief Stop the motor
    */
    void halt (void);

    /**
     * @brief invert spinning direction
     */
     void invert_direction (void);

    /**
    * @brief Setup the hardware (pins as output, pwm creation and so on)
    */
    void hardware_setup (void);

    /* --------------------------------- setters -------------------------------- */

    /**
     * @brief set slow decay mode
     */
    void set_slow_decay();

    /**
     * @brief set fast decay mode
     */
    void set_fast_decay();

  private:

    /* ------------------------------- pin mapping ------------------------------ */

    int _IN_1, _IN_2, _EEP;

    /* ------------------------------ internal data ----------------------------- */

    /**
     * https://www.allaboutcircuits.com/technical-articles/difference-slow-decay-mode-fast-decay-mode-h-bridge-dc-motor-applications/
     */
    enum Decay { SLOW_DECAY, FAST_DECAY};

    /*
     * current direction in which the motor is spinning
     */
    enum Direction { BACKWARD, FORWARD};
    
    Decay _decay_mode = Decay::SLOW_DECAY;
    Direction _direction = Direction::FORWARD;
    int _input_speed = 0; // store the value asked by the user
    int _normalized_speed = 0;

    /*
     * Valid range -> (10, 100)
     */
    static const int _speed_threshold = 10;
  
    /**
    * @brief Normalizes the input speed in range (0, 100) to fit the motors need
    * 
    * @param speed input in range (0, 100)
    * @return normalized speed (0, 100) -> (100, 80);
    */
    int normalize_speed (int speed);

};

#endif
