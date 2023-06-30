#!/bin/bash

#Application path location of applicaiton

Dependencies=`pwd`/Dependencies

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

#source ${ToolDAQapp}/ToolDAQ/root/bin/thisroot.sh

export LD_LIBRARY_PATH=`pwd`/lib:${Dependencies}/zeromq-4.0.7/lib:${Dependencies}/boost_1_66_0/install/lib:${Dependencies}/pqxx/install/lib:${Dependencies}/YOCTO/Binaries/linux/$arch_in/:$LD_LIBRARY_PATH

export SEGFAULT_SIGNALS="all"

#source ~/.profile
