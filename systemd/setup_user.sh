#!/bin/sh -x

USER=user
GECOS=Ubuntu
UGID=1000

DEFGROUPS="tty,sudo,adm,dialout,cdrom,plugdev,dip,video,input"

if id -u $USER >/dev/null 2>&1; then
    echo "I: $USER already exist !!!"
    exit 0
else
    echo "I: $USER not exist !!!"
fi

echo "I: creating default user $USER"
adduser --gecos $GECOS --disabled-login $USER --uid $UGID

echo "I: set user $USER password to 123456"
echo "$USER:123456" | chpasswd

mkdir -p /home/$USER/Music
mkdir -p /home/$USER/Pictures
mkdir -p /home/$USER/Videos
mkdir -p /home/$USER/Downloads
mkdir -p /home/$USER/Documents
chown -R $UGID:$UGID /home/$USER

usermod -a -G ${DEFGROUPS} ${USER}
