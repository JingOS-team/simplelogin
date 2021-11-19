#!/bin/bash

command="id 1000"
var=$($command)
var1=${var%% *}
var2=${var1%%)*}
var3=${var2#*(}
echo $var3

if [[ $var == *"uid"* ]]; then
    echo "user is set !"
    simplelogin --user $var3 --session /usr/share/wayland-sessions/plasma-mobile.desktop

else
    echo "user is not set!"
    simplelogin --user root --session /usr/share/wayland-sessions/plasma-mobile.desktop
fi
