#!/bin/sh

RPIBOOT=/media/sean/boot/
RPIROOT=/media/sean/rootfs/
RPIHOSTNAME=aeon
RPISSHKEY=${HOME}/.ssh/rpi_ecdsa.pub
RPIWIFISSID=
RPIWIFIPASS=

# Bail if error happens
set -o errexit

# Should set this but...
sudo vim ${RPIBOOT}/config.txt

# Enable SSH support on first boot
sudo touch ${RPIBOOT}/ssh

# Set hostname
echo "${RPIHOSTNAME}" > ${RPIROOT}/etc/hostname

# Enable login via SSH key by adding our pubkey to ~/.authorized_keys
sudo mkdir -m 700 ${RPIROOT}/home/pi/.ssh/
sudo chown 1000:1000 ${RPIROOT}/home/pi/.ssh/
cat ${RPISSHKEY} | sudo tee -a ${RPIROOT}/home/pi/authorized_keys

# Disable password login for pi user
sudo tee -a ${RPIROOT}/etc/ssh/sshd_config <<EOF
Match User pi
PasswordAuthentication no
EOF

# Setup WPA so wifi works
sudo tee -a ${RPIROOT}/etc/wpa_supplicant/wpa_supplicant.conf <<EOF
country=US
network={
  ssid="${RPIWIFISSID}"
  psk="${RPIWIFIPASS}"
}
EOF
