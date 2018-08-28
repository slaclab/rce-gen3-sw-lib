#!/bin/env python
#
# This script is used to burn an encoded MAC address into the RCE (Zynq)
# eFUSE USER register. The MAC address is allocated in the usual way.
#

import subprocess
import argparse
import re, os, sys
from tempfile import NamedTemporaryFile

import macaddr, update_macs
import getpass, cli, page

# This is the file which contains the Pool of MAC addresses we'll use
MAC_POOL_FILE = "/afs/slac/g/cci/hwdb/macfile"

# This is list of base addresses for each pool. Pool 0 is reserved
MAC_ADDR_POOL_BASES = ["Reserved",
                       "08:00:56:00:40:00"]

# These classes contain the different types of Mezannine boards
# supported. Each has the parameters which allow the script to
# identify and program the proper devices on the JTAG chain
class DPM:
  cmb_type = "DPM"
  device_ids =  ['Xilinx xc7z045',         
                 'Xilinx zynq7000_arm_dap',
                 'Xilinx xc7z045',         
                 'Xilinx zynq7000_arm_dap']
  device_pos = [2, 4]

class DTM:
    cmb_type = "DTM"
    device_ids = ['Xilinx xc7z030',
                  'Xilinx zynq7000_arm_dap']
    device_pos = [2]

CMBs = [DPM, DTM]


# These are the templates for the iMPACT scripts that are used to
# read/write the eFUSE and manage the JTAG chain.
CONFIG_XILINX_SCRIPT = \
"""
#!/bin/bash

# Set up Xilinx Environment
case ${HOSTTYPE} in
  x86_64*) bits="64"  ;;
  i386)    bits="32"  ;;
  i686)    bits="32"  ;;
  *)       bits="32"
esac

source /afs/slac/g/reseng/xilinx/ise_14.7/ISE_DS/settings${bits}.sh > /dev/null
"""

GET_CABLES_SCRIPT = \
CONFIG_XILINX_SCRIPT + \
"""
impact -batch  <<EOF 2>&1  | grep .
listusbcables
exit
EOF  
"""

GET_DEVICES_SCRIPT = \
CONFIG_XILINX_SCRIPT + \
"""
impact -batch  <<EOF 2>&1 
setmode -bs
setcable -port CABLE_PORT -baud 3000000
identify
exit
EOF  
"""

GET_EFUSE_SCRIPT = \
CONFIG_XILINX_SCRIPT + \
"""
impact -batch  <<EOF 2>&1 
setmode -bs
setcable -port CABLE_PORT -baud 3000000
identify
readefuse -user -position DEV_POS
exit
EOF  
"""

BURN_EFUSE_SCRIPT = \
CONFIG_XILINX_SCRIPT + \
"""
impact -batch  <<EOF 2>&1 
setmode -bs
setcable -port CABLE_PORT -baud 3000000
identify
programefuse -position DEV_POS -high_user -high_value HIGH_VALUE
programefuse -position DEV_POS -aeskey_and_low_user -low_value LOW_VALUE -no_key
exit
EOF  
"""

# This class contains utility functions for handling the different
# representations of the MAC addresses
class MAC_addr:
  def __init__(self, base, addr):
    self.addr   = addr.upper()
    self.base   = base.upper()
    if self.base not in MAC_ADDR_POOL_BASES:
      raise RuntimeError, "Base MAC %s not known"%self.base
    self.table  = MAC_ADDR_POOL_BASES.index(self.base)
    self.offset = self._to_int(self.addr) - self._to_int(self.base)

  @classmethod
  def from_efuse(cls, efuse):
    try:
      table  = (efuse & 0xffffc000)>>14
      offset = (efuse & 0x00003fff)
      base   = MAC_ADDR_POOL_BASES[table]
      addr   = cls._from_int(cls._to_int(base)+offset)
    except:
      print "Unable to convert eFUSE 0x%08x to MAC address"%efuse
      raise
      
    return cls(base, addr)

  @staticmethod
  def _to_int(addr):
    if re.match(5*r"[\dA-Fa-f]{2,2}:" + r"[\dA-Fa-f]{2,2}$", addr) is None:
      return None
    return long(addr.replace(":", ""), 16)

  @staticmethod
  def _from_int(mac):
    addr = 6*[None]
    for i in xrange(len(addr)):
      byte = mac & 0xff
      mac >>= 8
      addr[5 - i] = "%02x" % byte
    return ":".join(addr)
    
  def to_efuse(self):
    return (self.table<<14) | self.offset

# This class represents the JTAG cable by port and ESN
class JTAG_cable:
  def __init__(self, port, esn):
    self.port = port
    self.esn  = esn

  def get_devices(self):
  
    tf = NamedTemporaryFile()
    tf.write(re.sub("CABLE_PORT", self.port, GET_DEVICES_SCRIPT))
    tf.flush()
    os.fsync(tf)

    try:
      out = subprocess.check_output(["/bin/bash", tf.name])
    except subprocess.CalledProcessError, e:
      print e
      print "iMPACT returned an error. Log to follow:\n"
      print 80*"*"
      print e.output
      print 80*"*"
      print "\n"
      return []
    
    tf.close()
    
    out = out.strip().split('\n')

    devices = []
    d = 0
    for o in out:
        dev = re.search("(?:Manufacturer\'s ID = )(\w*) (\w*),",o)
        if not dev:
          continue
        else:
          dev = dev.group(1)+" "+dev.group(2)
          
        pos = re.search("([0-9])",o)
        if pos:
            pos = int(pos.group(0))
  
        if pos == d:
            devices.append(dev)
            d = d+1
        else:
            print "ERROR - Device positions not monotonically increasing"
            return None
        
    return devices

# A utility function which gets the list of Xilinx dongles attached
def get_cables():
  tf = NamedTemporaryFile()
  tf.write(GET_CABLES_SCRIPT)
  tf.flush()
  os.fsync(tf)

  try:
    out = subprocess.check_output(["/bin/bash", tf.name])
  except subprocess.CalledProcessError, e:
    print e
    print "iMPACT returned an error. Log to follow:\n"
    print 80*"*"
    print e.output
    print 80*"*"
    print "\n"
    return []
  
  tf.close()

  cables= out.strip().split('\n') 

  jtag_cables = []
  for cable in cables:
    if not re.match("port=usb[0-9]{2}, esn=[0-9A-Fa-f]{14}", cable):
      continue
    (port,esn) = cable.split(',')
    jtag_cables.append(JTAG_cable(port.split('=')[1], esn.split('=')[1]))

  return jtag_cables

# A convenience wrapper that queries the user for which cable to connect to
def select_cable(cable_req=None):
  cables = get_cables()
  cable = None

  if not cables:
    print "No cables found"
    return None
  
  if cable_req:
    for c in cables:
      if cable_req == c.esn:
        print "Found JTAG cable %s"%c.esn
        cable = c
    if not cable:
      print "Could not find JTAG cable %s"%args.esn
      return None
  else:
    print "Found the following JTAG cables:"
    for c in cables:
      print "%i : %s"%(cables.index(c),c.esn)

    try:
      idx = int(raw_input("Select one: "))
    except KeyboardInterrupt:
      raise
    except:
      idx = None
    while idx not in range(len(cables)):
      try:
        idx = int(raw_input("Try again: "))
      except KeyboardInterrupt:
        raise
      except:
        idx = None
    cable = cables[idx]
  return cable

# This class does the heavy lifting. When passed a CMB type, it can
# read/write/compare eFUSE contents. It will also allocate new MAC
# addresses and burn them into the eFUSEs.
class CMB_efuse:
  def __init__(self, cable, cmb_type=None):
    self.cable = cable

    devices = self.cable.get_devices()
    for cmb in CMBs:
      if cmb_type in [cmb.cmb_type, None] and devices == cmb.device_ids:
        print "Found %s on JTAG cable %s"%(cmb.cmb_type, cable.esn)
        self.cmb_type = cmb.cmb_type
        self.device_ids = cmb.device_ids
        self.device_pos = cmb.device_pos
        break
    else:
      err_string = "JTAG chain attached to %s not consistent with " \
                   %(self.cable.esn)
      if cmb_type:
        err_string += "device %s"%(cmb_type,)
      else:
        err_string += "any known device"
      raise RuntimeError, err_string 
 
  def read_efuse(self, dev):
  
    tf = NamedTemporaryFile()
    script = re.sub("CABLE_PORT", self.cable.port, GET_EFUSE_SCRIPT)
    script = re.sub("DEV_POS", str(self.device_pos[dev]), script)
    tf.write(script)
    tf.flush()
    os.fsync(tf)
    
    try:
      out = subprocess.check_output(["/bin/bash", tf.name])
    except subprocess.CalledProcessError, e:
      print e
      print "iMPACT returned an error. Log to follow:\n"
      print 80*"*"
      print e.output
      print 80*"*"
      print "\n"
      return None
    
    tf.close()
    
    out = out.strip().split('\n')
    
    efuse = 0
    byte = 0
    for o in out:
        b = re.search("\[0x([\dA-Fa-f]{2})\]", o)
        if b and byte<4:
            efuse |= int(b.group(1),16)<<(8*(3-byte))
            byte += 1
        elif b and byte>=4:
            print "ERROR - cannot parse IMPACT output for eFuse"
            return None
        
    return efuse

  def burn_efuse(self, pos, value):
    
    high_value = "%06x"%((value & 0xffffff00)>>8)
    low_value  = "%02x"% (value & 0x000000ff)
  
    tf = NamedTemporaryFile()
    script = re.sub("CABLE_PORT", self.cable.port, BURN_EFUSE_SCRIPT)
    script = re.sub("HIGH_VALUE", high_value, script)
    script = re.sub("LOW_VALUE", low_value, script)
    script = re.sub("DEV_POS", str(pos), script)
    tf.write(script)
    tf.flush()
    os.fsync(tf)

    try:
      out = subprocess.check_output(["/bin/bash", tf.name])
    except subprocess.CalledProcessError, e:
      print e
      print "iMPACT returned an error. Log to follow:\n"
      print 80*"*"
      print e.output
      print 80*"*"
      print "\n"
      return
    
  def burn_macs(self):

      for pos in range(len(self.device_pos)):
        efuse = self.read_efuse(pos)
        if efuse:
          print "eFUSE readback NON-ZERO (0x%08x) on device position %i, not proceeding with burn"%(efuse, self.device_pos[pos])
          return False


      print "About to allocate and burn %i MAC addresses into %s eFUSE registers"\
            %(len(self.device_pos),self.cmb_type)
      sure = raw_input("DO YOU WANT TO PROCEED? (y/N) ")
      if "Y" != sure.upper():
        print "OK - you are off the hook. Not proceeding."
        return False
      
      base = macaddr.get_base(MAC_POOL_FILE)
      addr = macaddr.allocate(len(self.device_pos), MAC_POOL_FILE)

      if len(addr) != len(self.device_pos):
        return False

      macs = [MAC_addr(base, a) for a in addr]
      efuse = [m.to_efuse() for m in macs]
      
      for pos in range(len(self.device_pos)):
        print "Base: %s, MAC %s, eFUSE 0x%08x"\
              %(macs[pos].base, macs[pos].addr, efuse[pos])
        self.burn_efuse(self.device_pos[pos], efuse[pos])


      readback = [self.read_efuse(dev) for dev in range(len(self.device_pos))]

      if readback != efuse:
        print "eFUSE readback mismatch: wrote %s, read %s"\
              %(["0x%08x"%val for val in efuse],
                ["0x%08x"%val for val in readback])
        return []

      print "eFUSE readback matched. Congratulations!"
      return macs

  def dump_macs(self):
    macs = []
    print 80*"="
    print "| %-37s  %37s |"%(self.cmb_type, "ESN: "+self.cable.esn)
    print 80*"="
    print "| %3s | %10s | %7s | %6s |  %17s | %17s |"\
          %("Dev", "eFUSE", "Table", "Offset", "Base MAC", "Device MAC")
    print 80*"-"
    for pos in range(len(self.device_pos)):
      efuse = self.read_efuse(pos)
      print "|  %1i  | 0x%08x |"%(pos, efuse),
      try:  
        mac = MAC_addr.from_efuse(efuse)
        macs.append(mac)
      except:
        pass
      else:
        print "0x%05x | 0x%04x |  %s | %s |"\
              %(mac.table, mac.offset, mac.base, mac.addr) 
    print 80*"="
    return macs
     
      
if __name__ == "__main__":

  desc = \
  """
  This program is used to allocate one or more MAC addresses
  from a pool, convert them, and write them into the eFUSE
  registers in the Zynqs on a COB Mezzanine Board.
  \n
  WARNING - Apparently, Xilinx has not seen fit to support eFUSE
            burning on 64-bit machines, so you'll have to find
            a 32-bit machine to run this script.
  """

  
  parser = argparse.ArgumentParser(description=desc,
                                   formatter_class=\
                                   argparse.RawDescriptionHelpFormatter,)
  cmb_group = parser.add_mutually_exclusive_group()
  for cmb in CMBs:
    cmb_group.add_argument("--"+cmb.cmb_type.lower(),
                       help="Program a "+cmb.cmb_type.upper(),
                       action="store_true")
  parser.add_argument("--esn", help="Xilinx dongle ESN", nargs=1)

  parser.add_argument("--hwdb", action="store_true", 
                      help="Update HWDB with MAC addresses")
  
  action_group = parser.add_mutually_exclusive_group(required=True)
  action_group.add_argument("--burn", action="store_true",
                            help="Allocate MAC addresses and burn eFUSEs")
  action_group.add_argument("--dump", action="store_true",
                            help="Dump the eFUSE contents")
                      

  args = parser.parse_args()

  if(args.esn):
    esn = args.esn[0]
  else:
    esn = None

  cable = select_cable(esn)
  if not cable:
    sys.exit(1)

  try:
    for cmb in CMBs:
      if eval("args."+cmb.cmb_type.lower()):
        cmb_efuse = CMB_efuse(cable, cmb.cmb_type)
        break
    else:
      print "No CMB type specified, discovering..."
      cmb_efuse = CMB_efuse(cable, None)
  except RuntimeError as err:
    print >> sys.stderr, str(err)+"\nExiting without doing anything."
    sys.exit(1)

  macs = []
  if args.burn:
    macs = cmb_efuse.burn_macs()
    cmb_efuse.dump_macs()
  else:
    print "BURN option not selected"

  if args.dump:
    macs = cmb_efuse.dump_macs()

  if args.hwdb and macs:
    print "Updating Hardware Database with MACs"
    
    while True:
      try:
        pwd = getpass.getpass("Confluence password for %s: " % getpass.getuser())
        rc = cli.login(getpass.getuser(), pwd, page.SPACE_KEY)
        break
      except cli.AuthFailure:
        pass

    cid = page.askForPageId()
    
    maclist = [m.addr for m in macs]
    try:
      update_macs.add_macs(cid, idx=0, macs=maclist)
    except ValueError, e:
      print "Couldn't update HWDB:", e
