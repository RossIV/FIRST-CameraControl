#ifndef FIRST_CAMERACONTROL_CONTROLLER_H
#define FIRST_CAMERACONTROL_CONTROLLER_H

#include "Joystick.h"
#include "Camera.h"

class Controller {
public:
    Controller(Joystick&& joystick, Camera&& camera);

    void update();

    Joystick& joystick();

    Camera& camera();

private:
    Joystick _joystick;

    Camera _camera;

    Joystick::HatDirection _prev_hat = Joystick::HatDirection::CENTERED;
    double _prev_pan = 0;
    double _prev_tilt = 0;
    double _prev_speed = 0;

    std::array<bool,12> _prev_button;

    void axis_control();

    void button_control();

    void set_prev_buttons();

};


#endif //FIRST_CAMERACONTROL_CONTROLLER_H
