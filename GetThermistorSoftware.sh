#!/bin/bash

arch=$(dpkg --print-architecture)

# Compare the value of arch using the if statement
if [[ $arch == "armhf" ]]; then
  echo "Architecture is armhf"
  arch_in="armhf"
elif [[ $arch == "amd64" ]]; then
  echo "Architecture is amd64"
  arch_in="64bits"
else
  echo "Architecture is neither armhf nor amd64"
fi

home=`pwd`

mkdir -p Dependencies
cd Dependencies #into Dependencies

git clone https://github.com/yoctopuce/yoctolib_cpp.git YOCTO
cd YOCTO #into YOCTO

./build.sh
cd Binaries/linux/$arch_in
ln -s libyocto.so.1.0.1 libyocto.so

cd $pwd
