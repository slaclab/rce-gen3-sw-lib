#!/bin/bash
#
# Fetch the includes and libraries from the development branch
#
# Unlike install-sdk.sh, this does nothing about the cross-dev tools
# assuming that if you're developing, you already have them.
#
# arguments:  sdk_dir
#
#

if [ $# -lt 1 ]; then
    echo "Usage: ${BASH_SOURCE} <sdk_dir>"
    exit -1
fi
sdkroot=$1; shift

cd ${sdkroot}

thisURL=$(cd ${sdkroot} > /dev/null 2>&1 && git config remote.origin.url)
arch=arm-rtems-rceCA9

# retrieve the includes and libs
echo "Fetching libraries and include files for rtems..."

for dir in include lib tgt; do
  if test -d ${sdkroot}/${dir}; then
    echo "Error:  ${dir} directory exists.  Are you installing an SDK on top of itself?"
    echo "  You may want to consider using update-sdk.sh instead."
    exit -1
  fi

  git clone -q --branch development ${thisURL}/../${dir} ${sdkroot}/${dir}
done
echo "...done"

# Now build
echo "Building the templates app.svt, sys.svt, appinit.so"
cd ${sdkroot}/examples/templates
./build.sh

  
