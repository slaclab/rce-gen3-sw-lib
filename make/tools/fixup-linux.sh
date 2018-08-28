#!/bin/bash
#
# Usage fixup-linux.sh <destination>
# 
#   destination  --> SDK location
#
# * Fetches the includes and libraries from their respective 
#   repositories (Note: development branch)
#
# * Compiles the host tools


if [ $# -lt 1 ]; then
  echo "Usage: $BASH_SOURCE <SDK_dir>"
  exit -1
fi
destination=$1; shift

sdkroot=${destination}
thisURL=$(cd ${sdkroot} > /dev/null 2>&1 && git config remote.origin.url)

if test -d ${sdkroot}/lib; then
  echo "Error: lib directory exists.  Are you installing an SDK on top of itself?"
  echo "  You may want to consider using update-sdk.sh instead."
  exit -1
fi
if test -d ${sdkroot}/include; then
  echo "Error: include directory exists.  Are you installing an SDK on top of itself?"
  echo "  You may want to consider using update-sdk.sh instead."
  exit -1
fi

# retrieve the includes and libs
echo -n "Fetching libraries and include files..."

# programming note:  This checks out the whole history
git clone -q --branch development ${thisURL}/include ${sdkroot}/include
git clone -q --branch development ${thisURL}/lib ${sdkroot}/lib

echo "done"

## Now build
#source ${sdkroot}/tools/envs-sdk.sh
#echo -n "Building the host tools for ${arch}..."
#${sdkroot}/tools/atca/build.sh ${arch}
#echo "done"
