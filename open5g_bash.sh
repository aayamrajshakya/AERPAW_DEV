#!/bin/bash

set -e

# Update the system
apt-get update
DEBIAN_FRONTEND=noninteractive apt-get upgrade --yes

# Install necessary packages
DEBIAN_FRONTEND=noninteractive apt-get install --yes \
  psmisc \
  git \
  cmake

# Update the system again
apt-get update
apt-get upgrade --yes

# Configure Git
git config --global https.postBuffer 123289600
git config --global http.sslverify false

# Get the current username
current_user=$(whoami)

# Use tilde directly in cd


if [ -d "~/containerization" ]; then
    echo "Folder exists. Skipping."
else
    echo "Folder does not exist. Cloning repository."
    cd ~
    git clone https://github.com/aayamrajshakya/containerization.git 

fi
folder_path=~/containerization

cd "$folder_path"

ls

#:<<'COMMENT'
# cd into AMF directory
cd amf/build/scripts
# Installing all the needed libraries/packages tfolder_patho build and run AMF
./build_amf --install-deps --force

# Building AMF
./build_amf --clean --Verbose --build-type Release --jobs \
  && ldconfig \

# We install some debug tools for the moment in addition of mandatory libraries
apt-get update
DEBIAN_FRONTEND=noninteractive apt-get upgrade --yes
DEBIAN_FRONTEND=noninteractive apt-get install --yes \
    tzdata \
    gcc-multilib \
    psmisc \
    net-tools \
    tcpdump \
    iputils-ping \
    bc \
    libssh-4 \
    openssl \
    libbrotli1 \
    netbase \
    libgssapi-krb5-2 \
    libldap-2.?-? \
    libconfig++9v5 \
    libsctp1 \
    librtmp1 \
    libpsl5 \
    libmysqlclient2? \
    libsqlite3-0 \
    libcurl4-gnutls-dev \
    libboost-thread1.7?.0 \
    libboost-chrono1.7?.0 \
  && rm -rf /var/lib/apt/lists/*


ldconfig

# Set environment variables
export TZ=Europe/Paris
export IS_DOCKERFILE=1

# Update and upgrade packages
apt-get update
DEBIAN_FRONTEND=noninteractive apt-get upgrade --yes

# Install required packages
DEBIAN_FRONTEND=noninteractive apt-get install --yes \
  psmisc \
  git

# Clean up
rm -rf /var/lib/apt/lists/*

# Configure GIT
git config --global https.pos tBuffer 123289600
git config --global http.sslverify false

#cd into ausf directory
cd ../../..
cd ausf/build/scripts
# Installing all the needed libraries/packages to build and run AUSF
apt-get update
apt-get upgrade -y
apt-get install -y libidn2-0-dev
./build_ausf --install-deps --force
ldconfig

#Building AUSF
./build_ausf --clean --Verbose --build-type Release --jobs && ldconfig

# We install some de
# Set environment variables
export DEBIAN_FRONTEND=noninteractive
export TZ=Europe/Paris

# Update and upgrade packages
apt-get update
DEBIAN_FRONTEND=noninteractive apt-get upgrade --yes

# Install debug tools and mandatory packages
DEBIAN_FRONTEND=noninteractive apt-get install --yes \
  tzdata \
  psmisc \
  net-tools \
  tcpdump \
  iputils-ping \
  bc \
  netbase \
  libconfig++9v5 \
  libnettle? \
  libhogweed? \
  libldap-2.?-? \
  libkrb5-3 \
  libgssapi-krb5-2 \
  libcurl?-gnutls \
  librtmp1 \
  libpsl5 \
  libboost-thread1.7?.0 

# Clean up
rm -rf /var/lib/apt/lists/*

ldconfig

# Set environment variables
export DEBIAN_FRONTEND=noninteractive
export TZ=Europe/Paris

DEBIAN_FRONTEND=noninteractive apt-get update --yes

# Install debug tools and mandatory libraries
DEBIAN_FRONTEND=noninteractive apt-get install --yes \
  psmisc \
  net-tools \
  ethtool \
  tshark \
  tzdata \
  iperf \
  iproute2 \
  iputils-ping \
  iptables \
  traceroute \
  tcpdump \
  iperf3

# Clean up
rm -rf /var/lib/apt/lists/*


###### BUILD IMS IMAGE #########################

export DEBIAN_FRONTEND=noninteractive

# Update and upgrade packages
apt-get update

DEBIAN_FRONTEND=noninteractive apt-get upgrade --yes

# Install required packages
DEBIAN_FRONTEND=noninteractive apt-get install --yes \
  psmisc \
  git \
  asterisk

# Clean up
rm -rf /var/lib/apt/lists/*

###################### BUILD NRF IMAGE #########################

# Set environment variables
export DEBIAN_FRONTEND=noninteractive
export TZ=Europe/Paris
export IS_DOCKERFILE=1

DEBIAN_FRONTEND=noninteractive apt-get upgrade --yes

# Install required packages
DEBIAN_FRONTEND=noninteractive apt-get install --yes \
  psmisc \
  git

# Clean up
rm -rf /var/lib/apt/lists/*

# Git configuration commands
git config --global https.postBuffer 123289600
git config --global http.sslverify false

#cd into NRF directory
cd ../../..
cd nrf/build/scripts

# Installing all the needed libraries/packages to build and run NRF
./build_nrf --install-deps --force
ldconfig

#Building NRF
./build_nrf --clean --Verbose --build-type Release --jobs
ldconfig

# Set environment variables
export DEBIAN_FRONTEND=noninteractive
export TZ=Europe/Paris

# Update and upgrade packages
apt-get update
DEBIAN_FRONTEND=noninteractive apt-get upgrade --yes

# Install debug tools and mandatory packages
DEBIAN_FRONTEND=noninteractive apt-get install --yes \
  tzdata \
  psmisc \
  net-tools \
  tcpdump \
  iputils-ping \
  bc \
  libldap-2.?-? \
  libkrb5-3 \
  libgssapi-krb5-2 \
  libcurl4-gnutls-dev \
  librtmp1 \
  libpsl5 \
  libboost-thread1.7?.0 \
  libboost-chrono1.7?.0

# Clean up
rm -rf /var/lib/apt/lists/*

ldconfig

###################### BUILD SMF IMAGE #########################

# Set environment variables
export DEBIAN_FRONTEND=noninteractive
export TZ=Europe/Paris
export IS_DOCKERFILE=1

# Update and upgrade packages
apt-get update
DEBIAN_FRONTEND=noninteractive apt-get upgrade --yes

# Install required packages
DEBIAN_FRONTEND=noninteractive apt-get install --yes \
  psmisc \
  git

# Clean up
rm -rf /var/lib/apt/lists/*

# Git configuration commands
git config --global https.postBuffer 123289600
git config --global http.sslverify false

#cd into SMF directory
cd ../../..
cd smf/build/scripts

# Installing all the needed libraries/packages to build and run NRF
./build_smf --install-deps --force

# Building SMF
./build_smf --clean --Verbose --build-type Release --jobs
ldconfig

# Set environment variables
export DEBIAN_FRONTEND=noninteractive
export TZ=Europe/Paris

# Update and upgrade packages
apt-get update
DEBIAN_FRONTEND=noninteractive apt-get upgrade --yes

# Install debug tools and mandatory packages
DEBIAN_FRONTEND=noninteractive apt-get install --yes \
  tzdata \
  psmisc \
  net-tools \
  tcpdump \
  iputils-ping \
  bc \
  libssh-4 \
  openssl \
  libbrotli1 \
  netbase \
  libgssapi-krb5-2 \
  libldap-2.?-? \
  libsctp1 \
  librtmp1 \
  libpsl5 \
  libcurl?-gnutls \
  libboost-thread1.7?.0 \
  libboost-chrono1.7?.0

# Clean up
rm -rf /var/lib/apt/lists/*

ldconfig

###################### BUILD UDM IMAGE #########################
# Set environment variables
export DEBIAN_FRONTEND=noninteractive
export TZ=Europe/Paris
export IS_DOCKERFILE=1

# Update and upgrade packages
apt-get update
DEBIAN_FRONTEND=noninteractive apt-get upgrade --yes

# Install required packages
DEBIAN_FRONTEND=noninteractive apt-get install --yes \
  psmisc \
  git

# Clean up
rm -rf /var/lib/apt/lists/*

# Git configuration commands
git config --global https.postBuffer 123289600
git config --global https.maxRequestBuffer 123289600
git config --global core.compression 0
git config --global http.sslverify false

#cd into UDM directory
cd ../../..
cd udm/build/scripts
# Installing all the needed libraries/packages to build and run UDM
./build_udm --install-deps --force 
ldconfig

# Building UDM
./build_udm --clean --Verbose --build-type Release --jobs

# Set environment variables
export DEBIAN_FRONTEND=noninteractive
export TZ=Europe/Paris

# Update and upgrade packages
apt-get update
DEBIAN_FRONTEND=noninteractive apt-get upgrade --yes

# Install debug tools and mandatory packages
DEBIAN_FRONTEND=noninteractive apt-get install --yes \
  tzdata \
  psmisc \
  net-tools \
  tcpdump \
  iputils-ping \
  bc \
  netbase \
  libconfig++9v5 \
  libnettle? \
  libhogweed? \
  libldap-2.?-? \
  libkrb5-3 \
  libgssapi-krb5-2 \
  libcurl?-gnutls \
  librtmp1 \
  libpsl5 \
  libboost-thread1.7?.0

# Clean up
rm -rf /var/lib/apt/lists/*

ldconfig

#---------------------------------------------------------------------
# BASE IMAGE UDR
#---------------------------------------------------------------------

# Set environment variables
export DEBIAN_FRONTEND=noninteractive
export TZ=Europe/Paris
export IS_DOCKERFILE=1

# Update and upgrade packages
apt-get update
DEBIAN_FRONTEND=noninteractive apt-get upgrade --yes

# Install required packages
DEBIAN_FRONTEND=noninteractive apt-get install --yes \
  psmisc \
  git

# Clean up
rm -rf /var/lib/apt/lists/*

# Git configuration commands
git config --global https.postBuffer 123289600
git config --global http.sslverify false

#wait for it
cd /
if [ -d "/wait-for-it" ]; then
    echo "Directory exists. Skipping."
else
    echo "Directory does not exist. Performing some action."
    git clone https://github.com/vishnubob/wait-for-it.git
fi




#cd into udr directory
cd "$folder_path"/udr/build/scripts

# Installing all the needed libraries/packages to build and run UDR
apt-get update
apt-get install -y libmongoc-dev libbson-dev
./build_udr --install-deps --force
ldconfig

# Building UDR
./build_udr --clean --Verbose --build-type Release --jobs
ldconfig
# Set environment variables
export DEBIAN_FRONTEND=noninteractive
export TZ=Europe/Paris

# Update and upgrade packages
apt-get update
DEBIAN_FRONTEND=noninteractive apt-get upgrade --yes

# Install debug tools and mandatory packages
DEBIAN_FRONTEND=noninteractive apt-get install --yes \
  tzdata \
  psmisc \
  net-tools \
  tcpdump \
  iputils-ping \
  bc \
  libmysqlclient2? \
  netbase \
  libconfig++9v5 \
  libnettle? \
  libhogweed? \
  libldap-2.?-? \
  libkrb5-3 \
  libgssapi-krb5-2 \
  libcurl?-gnutls \
  librtmp1 \
  libpsl5 \
  libboost-thread1.7?.0

# Clean up
rm -rf /var/lib/apt/lists/*

ldconfig

###################### INSTALL AND BUILD UPF #########################
# Set environment variables
export DEBIAN_FRONTEND=noninteractive
export TZ=Europe/Paris
export IS_DOCKERFILE=1

# Update and upgrade packages
apt-get update
DEBIAN_FRONTEND=noninteractive apt-get upgrade --yes

# Install required packages
DEBIAN_FRONTEND=noninteractive apt-get install --yes \
  psmisc \
  git

# Clean up
rm -rf /var/lib/apt/lists/*

# Git configuration commands
git config --global https.postBuffer 123289600
git config --global http.sslverify false

cd ~/containerization/upf/build/scripts

apt-get update
./build_upf --install-deps --force
./build_upf --clean --Verbose --build-type Release --jobs && ldconfig


# Set environment variables
export DEBIAN_FRONTEND=noninteractive
export TZ=Europe/Paris

# Update and upgrade packages
apt-get update
DEBIAN_FRONTEND=noninteractive apt-get upgrade --yes

# Install debug tools and mandatory libraries

DEBIAN_FRONTEND=noninteractive apt-get install --yes \
      tzdata \
      psmisc \
      net-tools \
      iproute2 \
      ethtool \
      arping \
      sudo \
      iptables \
      netbase \
      libasan? \
      libgssapi-krb5-2 \
      libldap-2.?-? \
      libgoogle-glog0v5 \
      libdouble-conversion3 \
      libconfig++9v5 \
      librtmp1 \
      libpsl5 \
      libnghttp2-14 \
      libcurl?-gnutls \
      libboost-thread1.7?.0

# Clean up
rm -rf /var/lib/apt/lists/*

cd /usr/local/lib

ldconfig

# use these labels for CI purpose

#support-multi-sgwu-instances="true"
#support-nrf-fdqn="true"




##################### INSTALL AND BUILD MYSQL ##################

sudo apt-get update -y
sudo apt-get install -y mysql-server
    
    
# Start MySQL service
sudo service mysql start
      
  
# Log in to MySQL and set root password
sudo mysql -u root -e "FLUSH PRIVILEGES;"
sudo mysql -u root -e "ALTER USER 'root'@'localhost' IDENTIFIED BY 'linux';"
  
# Log in to MySQL to create a database
sudo mysql -u root -e "create database oai_db;"

# Import the provided SQL script
cd ~/containerization/fed/docker-compose/database
sudo mysql -u root oai_db < oai_db2.sql

cd ~/containerization
git clone https://github.com/joshuamoorexyz/oai_cn_configs

