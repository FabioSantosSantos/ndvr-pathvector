#!/bin/bash

./waf configure --debug
PKG_CONFIG_PATH=/usr/local/lib/pkgconfig:$PKG_CONFIG_PATH ./waf configure --debug
./waf
sudo ./waf install
