#ifndef FIRST_CAMERACONTROL_CONTROLLER_H
#define FIRST_CAMERACONTROL_CONTROLLER_H

#include "Joystick.h"
#include "Camera.h"

class Controller {
public:
    Controller(std::shared_ptr<Joystick> joystick = nullptr, std::shared_ptr<Camera> camera = nullptr);

    void update();

    void change_joystick(std::shared_ptr<Joystick> joystick);

    void chage_camera(std::shared_ptr<Camera> camera);

private:
    std::shared_ptr<Joystick> _joystick;

    std::shared_ptr<Camera> _camera;

    double _prev_zoom = 0;

    void axis_control();

    void button_control();

};


#endif //FIRST_CAMERACONTROL_CONTROLLER_H
