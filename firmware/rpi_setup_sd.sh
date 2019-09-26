#!/bin/sh

RPIBOOT=/media/sean/boot/
RPIROOT=/media/sean/rootfs/
RPIHOSTNAME=aeon
RPISSHKEY=${HOME}/.ssh/rpi_ecdsa.pub
RPIWIFISSID=
RPIWIFIPASS=

# Bail if error happens
set -o errexit
set -o verbose

# Should set this but...
sudo tee -a ${RPIBOOT}/config.txt <<EOF
dtoverlay=i2c-rtc,ds1307
dtoverlay=gpio-poweroff,gpio_pin=24
dtoverlay=gpio-shutdown,gpio_pin=23,gpio_pull=2,active_low=1
EOF
sudo vim ${RPIBOOT}/config.txt

# Enable SSH support on first boot
sudo touch ${RPIBOOT}/ssh

# Set hostname
echo "${RPIHOSTNAME}" | sudo tee ${RPIROOT}/etc/hostname

# Enable login via SSH key by adding our pubkey to ~/.authorized_keys
sudo mkdir -m 700 ${RPIROOT}/home/pi/.ssh/
sudo chown 1000:1000 ${RPIROOT}/home/pi/.ssh/
cat ${RPISSHKEY} | sudo tee -a ${RPIROOT}/home/pi/.ssh/authorized_keys

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

# Copy clock program over
#sudo install -m 0755 -o root -g root -t ${RPIROOT}/usr/local/bin/ rpi_ht16k33/clock.py
sudo install -m 0755 -o root -g root -t ${RPIROOT}/home/pi/bin/ rpi_ht16k33/clock.py

# And add to rc.local
#sudo sed -ie "s/exit 0/sudo -u pi \/usr\/local\/bin\/clock.py/" ${RPIROOT}/etc/rc.local
#sudo tee -a ${RPIROOT}/etc/rc.local <<EOF
#
#exit 0
#EOF

# Enable i2c /dev entry
sudo tee -a ${RPIROOT}/etc/modules <<EOF
i2c-dev
EOF

sync
sync
sync
