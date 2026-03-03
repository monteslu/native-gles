#!/bin/bash
set -e
echo "Installing EGL and GLES development libraries..."
sudo apt install -y libegl-dev libgles-dev
echo "Done."
