#!/bin/bash

# Set environment variables
export DEBIAN_FRONTEND=noninteractive
export TZ=Europe/Paris


# Install debug tools and mandatory libraries
apt-get update
apt-get upgrade --yes
apt-get install --yes \
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

# move to trf dircetory
cd ~/containerization/fed/ci-scripts
mkdir -p ~/containerization/logs
touch ~/containerization/logs/trf.log

# Copy entrypoint script
cp trfgen_entrypoint.sh /tmp/

# Set entrypoint and default command
chmod +x /tmp/trfgen_entrypoint.sh
nohup /tmp/trfgen_entrypoint.sh > ~containerization/logs/trf.log &
