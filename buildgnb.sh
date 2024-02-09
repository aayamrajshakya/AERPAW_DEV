#!/bin/bash


sudo apt update -y
sudo apt install -y libboost-all-dev libusb-1.0-0-dev doxygen \
python3-docutils python3-mako python3-numpy python3-requests python3-ruamel.yaml python3-setuptools cmake build-essential git

# Clone UHD repository
git clone https://github.com/EttusResearch/uhd.git ~/uhd || { echo "Failed to clone UHD repository"; exit 1; }

# Checkout the desired version
cd ~/uhd || { echo "UHD directory not found"; exit 1; }
git checkout v4.5.0.0 || { echo "Failed to checkout UHD version"; exit 1; }

# Build and install UHD
cd ~/uhd/host || { echo "UHD host directory not found"; exit 1; }
mkdir -p build || { echo "Failed to create build directory"; exit 1; }
cd build || { echo "Failed to change to build directory"; exit 1; }
cmake ../ || { echo "CMake configuration failed"; exit 1; }
make -j $(nproc) || { echo "Build failed"; exit 1; }
sudo make install || { echo "Installation failed"; exit 1; }
sudo ldconfig

# Download UHD images
sudo uhd_images_downloader || { echo "Failed to download UHD images"; exit 1; }

# Clone OpenAirInterface repository
cd ~/
git clone https://gitlab.eurecom.fr/oai/openairinterface5g.git ~/openairinterface5g || { echo "Failed to clone OpenAirInterface repository"; exit 1; }

# Checkout the desired branch
cd ~/openairinterface5g || { echo "OpenAirInterface directory not found"; exit 1; }
git checkout develop || { echo "Failed to checkout OpenAirInterface branch"; exit 1; }

# Build OpenAirInterface
cd ~/openairinterface5g/cmake_targets || { echo "OpenAirInterface cmake_targets directory not found"; exit 1; }
./build_oai -I || { echo "OpenAirInterface build failed"; exit 1; }

# Install additional dependencies
sudo apt install -y libforms-dev libforms-bin || { echo "Failed to install additional dependencies"; exit 1; }

# Navigate to OpenAirInterface directory
cd ~/openairinterface5g || { echo "OpenAirInterface directory not found"; exit 1; }

# Source the environment setup script
source oaienv || { echo "Failed to source oaienv script"; exit 1; }

# Navigate to cmake_targets directory
cd ~/openairinterface5g/cmake_targets || { echo "OpenAirInterface cmake_targets directory not found"; exit 1; }

# Build OpenAirInterface with desired options
./build_oai -w USRP --ninja --nrUE --gNB --build-lib "nrscope" -C || { echo "OpenAirInterface build failed"; exit 1; }

