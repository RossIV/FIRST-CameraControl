#include <cmath>
#include <iostream>
#include "Camera.h"

using namespace std;
using boost::asio::ip::udp;

Camera::Camera(std::string name, std::string ip_address, uint8_t default_preset, boost::asio::io_service &io_service)
    : _name(name),
      _address(ip_address),
      _default_preset(default_preset),
      _io_service(io_service),
      _socket(_io_service)
{
    cout << "Connecting to camera " << name << " (" << ip_address << ")..." << endl;

    try {
        udp_connect(ip_address, "52381");
        cout << "Connected to camera " << name << "!" << endl;
    } catch(const boost::system::system_error &e) {
        if(e.code() == boost::asio::error::operation_aborted) {
            cerr << "ERROR: Connecting to camera " << _name << " timed out." << endl;
        }
    }
}

Camera::~Camera() {
    _socket.close();
}

void Camera::save_preset(uint8_t number) {
    send_command({0x81, 0x01, 0x04, 0x3F, 0x01, number, 0xFF});
}

void Camera::recall_preset(uint8_t number) {
    send_command({0x81, 0x01, 0x04, 0x3f, 0x02, number, 0xFF});
}

void Camera::rotate(double pan, double tilt) {
    uint8_t pan_power = static_cast<uint8_t>(pan_max() * abs(pan));
    pan_power = max(pan_power, static_cast<uint8_t>(1));

    uint8_t tilt_power = static_cast<uint8_t>(tilt_max() * abs(tilt));
    tilt_power = max(tilt_power, static_cast<uint8_t>(1));

    uint8_t pan_direction = 0x03;
    if(pan < 0)
        pan_direction = 0x02;
    else if(pan > 0)
        pan_direction = 0x01;

    uint8_t tilt_direction = 0x03;
    if(tilt < 0)
        tilt_direction = 0x02;
    else if(tilt > 0)
        tilt_direction = 0x01;

    send_command({0x81, 0x01, 0x06, 0x01, pan_power, tilt_power, pan_direction, tilt_direction, 0xFF});
}

void Camera::rotate(Camera::RotateType type, double speed_factor) {
    uint8_t pan_speed = static_cast<uint8_t>(pan_max() * speed_factor);
    uint8_t tilt_speed = static_cast<uint8_t>(tilt_max() * speed_factor);

    uint8_t pan_direction;
    uint8_t tilt_direction;

    switch(type) {
        case RotateType::STOP:
            pan_direction = 0x03;
            tilt_direction = 0x03;
            break;
        case RotateType::UP:
            pan_direction = 0x03;
            tilt_direction = 0x01;
            break;
        case RotateType::DOWN:
            pan_direction = 0x03;
            tilt_direction = 0x02;
            break;
        case RotateType::LEFT:
            pan_direction = 0x01;
            tilt_direction = 0x03;
            break;
        case RotateType::RIGHT:
            pan_direction = 0x02;
            tilt_direction = 0x03;
            break;
        case RotateType::UP_LEFT:
            pan_direction = 0x01;
            tilt_direction = 0x01;
            break;
        case RotateType::UP_RIGHT:
            pan_direction = 0x02;
            tilt_direction = 0x01;
            break;
        case RotateType::DOWN_LEFT:
            pan_direction = 0x01;
            tilt_direction = 0x02;
            break;
        case RotateType::DOWN_RIGHT:
            pan_direction = 0x02;
            tilt_direction = 0x02;
            break;
    }

    send_command({0x81, 0x01, 0x06, 0x01, pan_speed, tilt_speed, pan_direction, tilt_direction, 0xFF});
}

void Camera::zoom(double speed) {
    uint8_t power = static_cast<uint8_t>(zoom_max() * abs(speed));

    uint8_t direction = 0x00;
    if(speed < 0)
        direction = static_cast<uint8_t>(0x30) | power;
    else if(speed > 0)
        direction = static_cast<uint8_t>(0x20) | power;

    send_command({0x81, 0x01, 0x04, 0x07, direction, 0xFF});
}

void Camera::zoom(Camera::ZoomType type) {
    uint8_t direction;
    switch(type) {
        case ZoomType::STOP:
            direction = 0x00;
	    send_command({0x81, 0x01, 0x04, 0x07, 0x00, 0xff});
            break;
        case ZoomType::TELE:
            direction = 0x00;
	    send_command({0x81, 0x01, 0x04, 0x47, 0x04, 0x00, 0x00, 0x00, 0x00, 0xff});
            break;
        case ZoomType::WIDE:
	    direction = 0x00;
            send_command({0x81, 0x01, 0x04, 0x47, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff});
            break;
    }
}

void Camera::focus(Camera::FocusType type) {
    uint8_t direction;
    switch(type) {
        case FocusType::NEAR:
            direction = 0x03;
            break;
        case FocusType::FAR:
            direction = 0x02;
            break;
        case FocusType::STOP:
            direction = 0x00;
            break;
    };
    
    // Set to manual focus
    send_command({0x81, 0x01, 0x04, 0x38, 0x03, 0xff});
    //  Adjust focus
    send_command({0x81, 0x01, 0x04, 0x08, direction, 0xff});

}

void Camera::stop() {
    rotate();
    zoom();
}

void Camera::send_command(std::vector<uint8_t> payload) {
    uint8_t header[] = {0x01, 0x00, 0x00, payload.size(), 0x00, 0x00, 0x00, 0x00};
    uint8_t message[] = {header.data(), payload.data()}

    if(_socket.is_open()) {
        try {
            _socket.send(boost::asio::buffer(message, message.size()));
        } catch(const boost::exception_detail::clone_impl<boost::exception_detail::error_info_injector<boost::system::system_error> > &er) {
            cerr << "Error: Communication to camera " << _name << " failed.\nYou have likely lost connection. This camera will now be disabled." << endl;
            _socket.close();
        }
    }
}

void Camera::reconnect() {
    if (_socket.is_open()) {
        _socket.close();
    }

    try {
        udp_connect(_address, "52381");
    } catch (const boost::system::system_error &e) {
        if (e.code() == boost::asio::error::operation_aborted) {
            cerr << "ERROR: Connection to camera " << _name << " timed out." << endl;
        }
    } 
}

void Camera::udp_connect(const std::string &host, const std::string &service) {
    udp::resolver::query query{host, service};
    udp::resolver::iterator endpoint_iter = udp::resolver{_io_service}.resolve(query);

    boost::system::error_code ec = boost::asio::error::would_block;

    /*
     * TODO Boost's async connect seems to have a bug in it as this causes only the first camera to work.
     * May be related to https://svn.boost.org/trac/boost/ticket/8795
     */

//    boost::asio::async_connect(_socket, endpoint_iter, [&ec](const boost::system::error_code& error, const tcp::resolver::iterator&){ ec = error; });

//    timer.async_wait([this](const boost::system::error_code&){
//        _socket.close();
//    });

//    // Block until the asynchronous operation has completed.
//    while(ec == boost::asio::error::would_block) {
//        _io_service.run_one();
//    }

    _socket.connect(*endpoint_iter, ec);

    if (ec || !_socket.is_open())
        throw boost::system::system_error(
                ec ? ec : boost::asio::error::operation_aborted);
}
