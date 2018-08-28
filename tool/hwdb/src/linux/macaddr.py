#-*-Mode: Python;-*-
## @file
# @brief Operations related to MAC addresses: parsing, validation, allocation, etc.
#
# Externally a MAC address is a string of the form HH:HH:HH:HH:HH:HH where
# H is a lower/upper case hex digit. Internally a MAC address is a long int
# with the leftmost digits being the most significant, i.e.,
# aa:bb:cc:dd:ee:ff becomes 0xaabbccddeeff.
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
# \$Date: 2015-02-12 11:13:16 -0800 (Thu, 12 Feb 2015) $ by \$Author: jgt $.
#
# @par Revision number:
# \$Revision: 3998 $
#
# @par Location in repository:
# \$HeadURL: file:///afs/slac.stanford.edu/g/cci/repositories/ctkrepo/core_tags/prod/V2.4.2/tool/hwdb/src/linux/macaddr.py $
#
# @par Credits:
# SLAC

# Standard Python library
import fcntl, os, re, sys


## @brief Allocate a block of n addresses. Return an empty list
# if n addresses are not available. Otherwise return
# a list of the addresses in external form.
# @param[in] n The number of addresses to allocate.
# @param[in] macfile The name of the file to read for info on MAC allocation.
def allocate(n, macfile):
    pool = _Pool(macfile)
    allocated = pool.allocate(n)
    if allocated:
        pool.writeback()
    else:
        pool.release()
    return allocated

## @brief Return the base MAC address from this pool
# @param[in] macfile The name of the file to read for info on MAC allocation.
def get_base(macfile):
    pool = _Pool(macfile)
    return _toExternal(pool.first)

# Convert an external-form MAC address to internal
# form, producing None if the address is badly formed.
def _toInternal(addr):
    if _macRe.match(addr) is None:
        print >>sys.stderr, "macaddr.py: Invalid MAC address", addr
        sys.exit(1)
    return long(addr.replace(":", ""), 16)

_macRe = re.compile(5*r"[\dA-Fa-f]{2,2}:" + r"[\dA-Fa-f]{2,2}$")

# Convert an internal-form MAC address to external form.
def _toExternal(mac):
    addr = 6*[None]
    for i in xrange(len(addr)):
        byte = mac & 0xff
        mac >>= 8
        addr[5 - i] = "%02x" % byte
    return ":".join(addr)

# Class representing the pool of MAC addresses.
class _Pool(object):
    # Initialize from the field dictionary extracted from the pool page.
    def __init__(self, macfile):
        self.openAndLock(macfile)
        lines = self.file.readlines()
        if len(lines) != 3:
            print >>sys.stderr, "macaddr.py: MAC address file doesn't have exactly three lines."
            sys.exit(1)
        self.first    = _toInternal(lines[0])
        self.nextfree = _toInternal(lines[1])
        self.end      = _toInternal(lines[2]) + 1
    def allocate(self, n):
        if n > (self.end - self.nextfree):
            return list()
        allocated = list(range(self.nextfree, self.nextfree+n))
        self.nextfree += n
        return list(_toExternal(a) for a in allocated)
    def isValid(self, mac):
        return self.first <= mac < self.end
    def isAllocated(self, mac):
        return self.first <= mac < self.nextfree
    def openAndLock(self, macfile):
        self.file = open(macfile, "r+")
        fcntl.flock(self.file.fileno(), fcntl.LOCK_EX)
        self.file.seek(0)
    def writeback(self):
        self.file.seek(0)
        print >>self.file, _toExternal(self.first)
        print >>self.file, _toExternal(self.nextfree)
        print >>self.file, _toExternal(self.end - 1)
        self.file.flush()
        self.file.truncate()
        os.fsync(self.file.fileno())
        fcntl.flock(self.file.fileno(), fcntl.LOCK_UN)
        self.file.close()
    def release(self):
        fcntl.flock(self.file.fileno(), fcntl.LOCK_UN)
        self.file.close()
