#!/bin/bash

(( EUID != 0 )) && exec sudo -- "$0" "$@"

apt-get update
sudo apt-get install -y cmake libsdl2-dev libboost-all-dev libyaml-cpp-dev

mkdir -p build
cd build
cmake ..

