#include <csignal>
#include <iostream>
#include <chrono>
#include <thread>
#include "Joystick.h"
#include "Camera.h"
#include "Controller.h"

using namespace std;

volatile bool running = true;

void signal_handler(int) {
    running = false;
}

void set_SIGINT_handler() {
    struct sigaction sigIntHandler;
    sigIntHandler.sa_handler = signal_handler;
    sigemptyset(&sigIntHandler.sa_mask);
    sigIntHandler.sa_flags = 0;
    sigaction(SIGINT, &sigIntHandler, nullptr);
}

int main(){

    set_SIGINT_handler();

    Joystick::Init();

    Configuration config{"config.yaml"};

    auto camera_details = config.cameras();

    cout << "Found " << camera_details.size() << " cameras in config file." << endl;

    boost::asio::io_service io_service;

    auto joystick_count = static_cast<size_t>(Joystick::NumberOfConnectJoysticks());

    cout << "Found " << joystick_count << " joysticks." << endl;

    auto connection_count =  min(joystick_count, camera_details.size());

    cout << "Conecting to " << connection_count << " cameras." << endl;

    if(connection_count == 0) {
        return 0;
    }

    std::vector<Controller> controllers;

    for(size_t i = 0; i < joystick_count && i < camera_details.size(); ++i) {
        cout << "Assigning joystick " << i << " to camera " << camera_details[i].name << "(" << camera_details[i].ip_address << ")." << endl;
        controllers.emplace_back(Joystick(i), Camera(camera_details[i], io_service));
    }

    auto t = chrono::system_clock::now();

    while(running){
        for_each(controllers.begin(), controllers.end(), bind(&Controller::update, placeholders::_1));
        t += 10ms;
        this_thread::sleep_until(t);
    }

    return 0;
}
