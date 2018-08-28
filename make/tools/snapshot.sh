#!/bin/bash
#
# Create a snapshot tag of the various trunks for the RCE core codebase
#
# The tag is created using a timestamp: 
#      $SVNROOT/core_tags/snapshot/YYYYMMDD_hhmm
#
# arguments == commit message

if [ $# -lt 1 ]; then
  echo "usage: ${BASH_SOURCE} 'commit message'"
  echo "   Note: commit message should be in quotes of some sort"
  exit -1
fi
commit_msg="$*"

if [ ${#SVNROOT} -le 1 ]; then
  echo "The SVNROOT variable must be set for this script to work"
  exit -1
fi


snap_dir=core_tags/snapshot

build_dir="workspace"
subdirs="bin bootstrap cm configuration driver platform ppi service test tool xilinx"

tag=$(date +"%Y%m%d_%H%M")

svn cp -m "${commit_msg}" $SVNROOT/${build_dir}/trunk ${SVNROOT}/${snap_dir}/${tag}

for i in ${subdirs}; do
  svn cp -m "${commit_msg}" ${SVNROOT}/${i}/trunk ${SVNROOT}/${snap_dir}/${tag}/${i}
done

echo "Snapshotted to ${snap_dir}/${tag}"