#!/bin/sh

# Enable i2c
sudo raspi-config nonint do_i2c 0

# Enable HW serial but disable login over it (horrible undocumented option)
sudo raspi-config nonint do_serial 2

# Update system
sudo apt update && sudo apt -y full-upgrade && sudo apt -y autoremove

# Install required packages
sudo apt -y install build-essential git gpsd gpsd-clients libi2c-dev ntp pps-tools python3 python3-pil python3-pip python3-dev python3-virtualenv screen vim

# Instally python libraries
pip3 install --user adafruit-circuitpython-ht16k33 adafruit-circuitpython-veml7700

sudo tee -a /boot/config.txt <<EOF
dtoverlay=gpio-shutdown,gpio_pin=26
dtoverlay=i2c-rtc,ds3231
EOF

sudo tee -a /etc/ntp.conf <<EOF
# Enable statistics collection
statsdir /var/log/ntpstats/

# Allow LAN clients to query
restrict 192.168.42.0 mask 255.255.255.0

# PPS reference clock (primary sync source)
server 127.127.22.0 #minpoll 4 maxpoll 4
fudge 127.127.22.0 refid PPS
fudge 127.127.22.0 flag3 1 #enable kernel PLL/FLL clock discipline

# Shared mem interface to gpsd. This is the secondary source as we have no way
# of knowing what the millisecond offset is for the NMEA messages from the GPS
# unit. The "time1" parameter is a ballpark offset to use.
server 127.127.28.0 prefer #minpoll 4 maxpoll 4
fudge 127.127.28.0 time1 0.235
fudge 127.127.28.0 refid SHM
fudge 127.127.28.0 stratum 10
EOF

# Enable gpsd and configure it
sudo tee /etc/default/gpsd <<EOF
# Devices gpsd should collect to at boot time.
# They need to be read/writeable, either by user gpsd or the group dialout.
DEVICES="/dev/ttyAMA0"

# Other options you want to pass to gpsd
GPSD_OPTIONS=""

# Automatically hot add/remove USB GPS devices via gpsdctl
USBAUTO="true"
EOF

sudo systemctl enable gpsd.service

# Remove fake clock crap
sudo apt -y remove fake-hwclock
sudo update-rc.d -f fake-hwclock remove

sudo tee /lib/udev/hwclock-set <<EOF
#!/bin/sh
# Reset the System Clock to UTC if the hardware clock from which it
# was copied by the kernel was in localtime.

dev=\$1

#if [ -e /run/systemd/system ] ; then
#    exit 0
#fi

/sbin/hwclock --rtc=\$dev --systz
/sbin/hwclock --rtc=\$dev --hctosys
EOF
