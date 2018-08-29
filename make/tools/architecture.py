#!/usr/bin/env python
# Defines a class which parses a standard DAT architecture string
# and creates an object which is used in generating link farms for
# include and/or source files

import os

COREINC_SUBDIR = os.path.join('include', 'core')


# Map short forms of board names to the build type in order
# to pick up the boot.bin.  The keys are also keys into 
# finding fpga.bit for each board
BOOT_ARCH_MAP  = { 'dpm'   : 'arm-eabi-rceCA9',
                   'dtm'   : 'arm-eabi-cmCA9',
                   'zed'   : 'arm-eabi-zedCA9'}

class Architecture:
    I_FAM = 0
    I_OS  = 1
    I_BRD = 2
    I_DBG = 3
    def __init__(self, architecture):
        #print "Architecture constructor: %s" % architecture
        self._part = architecture.split('-')
    def __str__(self):
        return "-".join(self._part)
    @property
    def family(self):  
        return self._part[self.I_FAM]
    @property
    def os(self):
        return self._part[self.I_OS]
    @property
    def board(self):
        return self._part[self.I_BRD]
    @property
    def dbgOpt(self):
        if len(self._part) >= self.I_DBG+1:
            return self._part[self.I_DBG]
        return None
    @property
    def noDbgOpt(self):
        return '-'.join(self._part[:self.I_DBG])
    @property
    def cpuModel(self):
        # model is problematic 
        mpu = self.board if "CA9" not in self.board else "CA9"
        return self.family+mpu
    @property
    def platform(self):
        if 'eabi' not in self.os:
            return self.cpuModel+'-'+self.os
        return self.cpuModel
    @property
    def generation(self):
        if 'CA9' in self.board: return 'gen3'
        return None
    @property
    def cpu(self):
        return self.cpuModel
    @property
    def gen(self):
        return self.generation
    # Given a single directory name, match against 
    # the architecture properties
    def match(self, name):
        ret = []
        if self.generation == name: ret.append('gen')
        if self.family     == name: ret.append('family')
        if self.os         == name: ret.append('os')
        if self.board      == name: ret.append('board')
        if self.cpuModel   == name: ret.append('cpu')
        if self.platform   == name: ret.append('platform')
        return ret


class Build:
    # buildRoot is a directory, arch is an Architecture object
    def __init__(self, buildRoot, arch):
        self._d  = os.path.normpath(buildRoot)
        self._a  = arch
    @property
    def root(self):     return self._d
    @property
    def arch(self):     return self._a
    @property
    def archDir(self):  return os.path.join(self.root, str(self.arch))
    @property
    def incDir(self):   return os.path.join(self.archDir, COREINC_SUBDIR)
    @property
    def stubDir(self):  return os.path.join(self.archDir, 'lib')
    @property
    def taskDir(self):  return os.path.join(self.archDir, 'mod')
    @property
    def svtDir(self):   return os.path.join(self.archDir, 'mod')
    @property
    def soDir(self):
        if (self.arch.match('rtems')):
            return os.path.join(self.archDir, 'mod')
        else:
            return os.path.join(self.archDir, 'lib')
    @property
    def binDir(self):   return os.path.join(self.archDir, 'bin')
    @property
    def depDir(self):   return os.path.join(self.archDir, 'dep')
    @property
    def usrDocDir(self):return os.path.join(self.archDir, 'doc_usr')
    @property
    def devDocDir(self):return os.path.join(self.archDir, 'doc_dev')
    

class SDK:
    def __init__(self, sdkRoot, architecture):
        self._d = sdkRoot
        self._a = architecture
    @property
    def root(self):     return self._d
    @property
    def arch(self):     return self._a
    @property
    def incDir(self):   return os.path.join(self.root, COREINC_SUBDIR)
    @property
    def sysDir(self):   
        return os.path.join(self.root, 'tgt')
    @property
    def libDir(self):   return os.path.join(self.root, 'lib')
    @property
    def soDir(self):    
        if self.arch.match('rtems'):
            return os.path.join(self.sysDir,'rtems')
        elif self.arch.match('linux'):
            return self.libDir
        else: return None
    @property
    def svtDir(self):    
        if self.arch.match('rtems'):
            return os.path.join(self.sysDir,'rtemsapp','config')
        elif self.arch.match('linux'):
            return self.libDir
        else: return None
    @property
    def binDir(self):
        return os.path.join(self.root, 'bin')

if __name__ == "__main__":

    import sys
    
    arches = [ "arm-eabi-rceCA9-dbg",
               "arm-linux-rceCA9-opt",
               "arm-eabi-cmCA9-dbg",
               "arm-eabi-zedCA9-opt",
               "i86-linux-32",
               "i86-linux-64"
               ]

    for arch in arches:
        a= Architecture(arch)
        print "Arch: %s" % a
        print "   brd:%s  cpu:%s  fam:%s  gen:%s  os:%s  plat:%s" % \
            (a.board, a.cpuModel, a.family, a.generation, a.os, a.platform)

