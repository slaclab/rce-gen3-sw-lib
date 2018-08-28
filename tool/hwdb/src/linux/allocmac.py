#!/bin/env python
#-*-Mode: Python;-*-
## @file
# @brief Allocate a free MAC address and print it to stdout.
# @verbatim
#                               Copyright 2014
#                                    by
#                       The Board of Trustees of the
#                    Leland Stanford Junior University.
#                           All rights reserved.
# @endverbatim
#
# @par Facility:
# DAT
#
# @author
# Stephen Tether <tether@slac.stanford.edu>
#
# @par Date created:
# 2014/03/28
#
# @par Last commit:
# \$Date: 2014-04-09 22:43:44 -0700 (Wed, 09 Apr 2014) $ by \$Author: tether $.
#
# @par Revision number:
# \$Revision: 3144 $
#
# @par Location in repository:
# \$HeadURL: file:///afs/slac.stanford.edu/g/cci/repositories/ctkrepo/core_tags/prod/V2.4.2/tool/hwdb/src/linux/allocmac.py $
#
# @par Credits:
# SLAC

# Python standard library
import getpass, optparse, sys

# HWDB software.
import macaddr

DEFAULT_MACFILE = "/afs/slac/g/cci/hwdb/macfile"

def main(macfile):
    allocated = macaddr.allocate(1, macfile)
    if not allocated:
        print >>sys.stderr, "allocmac.py: Out of MAC addresses!"
        return 2
    print allocated[0]
    return 0

if __name__ == "__main__":
    p = optparse.OptionParser("allocmac.py --macfile MACFILE")
    p.add_option("--macfile",  metavar="MACFILE",
                 help="Optional. The file containing information on MAC address allocation.",
                 default=DEFAULT_MACFILE)
    opts, args = p.parse_args()
    sys.exit(main(opts.macfile))
