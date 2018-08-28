#-*-Mode: Python;-*-
## @file
# @brief Various checking functions for the hardware database.
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
# \$HeadURL: file:///afs/slac.stanford.edu/g/cci/repositories/ctkrepo/core_tags/prod/V2.4.2/tool/hwdb/src/linux/check.py $
#
# @par Credits:
# SLAC

# Standard Python library.
import re

# HWDB software.
import cli, page

## @brief Check the CRC of a device serial number.
# @param[in] serial The serial number as a hex string. No leading "0x".
#
# Dallas/Maxim 8-bit CRC.  Polynomial x**8 + x**5 + x**4 + 1.
# Dropping the hight-order coefficient and encoding the polynomial
# such that the LSB corresponds to x**7 yields 0x8c.  The serial
# number argument is a 16-byte hex string without a leading "0x". The
# order of the bytes is the reverse of the order their I2C addresses
# have, so that the CRC is first and the family code is last.
# The function returns a pair: the actual CRC and the calculated
# one, or None if the form of the serial no. is invalid.
def serialCrc(serial):
    if len(serial) < 16:
        return None
    # Convert from string to int.
    try:
        iserial = int(serial, 16)
    except:
        return None
    # Calculate the CRC on the lowest-order
    # seven bytes.
    crc = 0
    for i in xrange(0, 7):
        byte = iserial & 0xff
        iserial >>= 8
        crc ^= byte
        for j in xrange(0, 8):
            if crc & 0x1:
                crc = (crc >> 1) ^ 0x8c
            else:
                crc >>= 1
    # The CRC byte is all that's left of the serial number,
    return iserial, crc

## @brief Given a device class and a Ryan hardware page title, make sure
# that the latter is valid for the former. Returns True or False.
# @param[in] devClass The device class string, i.e., "COB", "DPM", etc.
# @param[in] hwPageTitle The title of the hardware description page.
def validHwPage(devClass, hwPageTitle):
    return devClass == page.typeToClass.get(hwPageTitle, None)

## @brief Perform some checks on a device page. Returns True or False.
# @param[in] cid The Confluence page ID, as a string.
#
# Checks performed:
# - Does the page exist?
# - Is the title of the correct form?
# - Is the Type field present and correct?
#   - Correct form is "HWDB <pageId>"
# - Is the Serial field present and correct?
#   - Valid form and CRC.
def validDevicePage(cid):
    # Page exists.
    pages = cli.getPageList(cid=cid)
    if not pages:
        print "No such page."
        return False
    # Title is valid.
    print "Title is %r." % pages[0]["Title"]
    tt = pages[0]["Title"].split(None, 2)
    if len(tt) != 2:
        print "The title doesn't have exactly two words."
        return False
    pageType, cid2 = tt
    if pageType != "HWDB":
        print "The title's first word is not HWDB."
        return False
    if cid2 != cid:
        print "The second word of the title is not the page ID."
        return False
    fields = page.extractFields(cli.getPageSource(cid=cid))
    if "type" not in fields:
        print "There's no Type field."
        return False
    devtype = fields["type"]
    if devtype not in page.typeToClass:
        print "The type value %r is not valid for RCE Gen 3." % devtype
        return False
    if "serial" not in fields:
        print "There's no Serial field."
        return False
    serial = fields["serial"]
    crcs = serialCrc(serial)
    if crcs is None:
        print "The serial number %r is not a 16-digit hex number." % serial
        return False
    if crcs[0] != crcs[1]:
        print "The serial number %r fails the CRC test." % serial
        return False
    return True

def testcrc():
    serials = (
        "fooey",             # Bad form
        "pfui000000000000",  # Bad form
        "ff00000000000000",  # Bad CRC    
        "040000007eaf8470",
        "da0000007ed19970",
        "690000007ed46770",
        "b10000007e9c9b70",
        "ea0000007ef8f270",
        "690000007e910c70"
    )
    for s in serials:
        print "Serial no.", s,
        result = serialCrc(s)
        if result is None:
            print "Bad form"
        elif result[0] == result[1]:
            print result, "OK"
        else:
            print result, "BAD CRC"
