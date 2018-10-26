#!/bin/sh

phpize
./configure --enable-phd --with-php-config=$(which php-config)
make
sudo make install
