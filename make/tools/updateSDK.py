#!/bin/env python

import sys, os, shutil, glob, filecmp, stat

from architecture import *

# location of htis file (used for finding ancillary stuff)
WHERE_AM_I = os.path.dirname(os.path.realpath(__file__))

# When copying trees, filter out emacs autosaves and any dotfiles
IGNORE_PATTERNS = shutil.ignore_patterns('*~', '.*')

# GLOBs for pattern matching
RTEMS_TASK_GLOB = '*.exe'
RTEMS_SVT_GLOB  = '*.svt'
SHARED_GLOB     = '*.so'

# Various name lists needed when updateing
STUB_NAMES = ['svt', 'task']       # List of all stubs being shipped with RTEMS SDK
SDK_EXCLUDE = []
RTEMS_SRC_NAMES = [os.path.join('configuration', 'svtcfg', 'src', 'app.c'),
                   os.path.join('configuration', 'svtcfg', 'src', 'sys.c'),
                   os.path.join('configuration', 'svtcfg', 'src', 'sas.c'),
                   os.path.join('platform', 'startup', 'src', 'appinit.c'),
                   ]

FREEIPMI_LIB_STUB = '/afs/slac/g/cci/package/freeipmi/current-'
PTH_STUB          = '/afs/slac/g/cci/package/gnu_afs/pth/current-'
LINUX_STUB        = '/afs/slac/g/cci/volumes/vol1/xilinx/linux-xlnx'

# Various other global variable declarations for completeness
TGT_ARCH          = None    # The target architecture
TGT_BUILD         = None    # The build to read from
TGT_SDK           = None    # The base SDK to update
CM_ARCH           = None    # The target architecture for DTM linux
CM_BUILD          = None    # The build to read from for DTM linux code

SRC_RTEMS_INCLUDE = None    # The source of the RTEMS include files for an RTEMS SDK
SRC_FPGA_BIT      = None    # The source of the FPGA bit files

#----------------------------------------------------------------------#
# HELPER functions which either abstract a function or 
#        simplify code

def fatal(errorString, rc=-1):
    sys.stderr.write('Error: ' + errorString)
    sys.exit(rc)

def warning(warnString):
    sys.stderr.write('Warning: ' + warnString)

def exclude(src):
    if not SDK_EXCLUDE: return
    def match(pat):
        from fnmatch import fnmatch
        return fnmatch(src,pat)
    return True in map(match,SDK_EXCLUDE)
    if not (os.path.basename(src) in SDK_EXCLUDE):
        return False
    return True

# Abstract the copy function to allow exclusion
def doCopy(src, dest):
    if not exclude(src):
#        print 'cp %s --> %s' % (src,dest)
        shutil.copyfile(src, dest)
        shutil.copymode(src, dest)
        return True
    return False

# Abstract the link function to allow exclusion
def doLink(src, dest):
    if not exclude(src):
#        print 'link %s --> %s' % (src,dest)
        os.symlink(src, dest)

def isLinux():
    if TGT_ARCH.match('linux'): return True
    return False

def isRtems():
    if TGT_ARCH.match('rtems'): return True
    return False

def isARM():
    if TGT_ARCH.match('arm'): return True
    return False

def syncTree(src, dest, follow=True):
    # This is implemented as a three step process.
    # First develop a list of things to be copied
    # Second, create only the directories needed
    # Third, do the copy
    toCopy = set()
    toMkDir = set()
    # This builds 
    for root, dirs, files in os.walk(src, topdown=True, followlinks=follow):
        if '.svn' in root: continue          # Exclude .svn and subdirs
        cutRoot = root[len(src):].lstrip('/')
        for f in files:
            s = os.path.join(root,f)
            d = os.path.join(dest,cutRoot,f)
            if not os.path.isfile(d) or not filecmp.cmp(s,d,shallow=False):
                if not exclude(s):
                    toCopy.add( (s, os.path.join(cutRoot,f)) )
    # Only make directories that are needed.
    [toMkDir.add(os.path.dirname(f[1])) for f in toCopy]
    for d in toMkDir: 
        f = os.path.join(dest,d)
        if not os.path.isdir(f): os.makedirs(f)
    # now copy
    for f in toCopy: doCopy(f[0], os.path.join(dest,f[1]))
    

#----------------------------------------------------------------------#
# CORE functions which define the logic of an SDK

# Snarf the include directory and the RTEMS includes (if needed)
def updateIncludes():
    DEST_SDK = TGT_SDK# if isRtems() else TGT_ARCH_SDK
    syncTree(TGT_BUILD.incDir, DEST_SDK.incDir)
    if isRtems() and SRC_RTEMS_INCLUDE is not None:
        syncTree(SRC_RTEMS_INCLUDE, os.path.abspath(os.path.join(DEST_SDK.incDir,'..','rtems')),follow=False)
    if isLinux():
        pass
                
# Now the shared libraries
def updateShared():
    if not os.path.isdir(TGT_SDK.soDir): os.makedirs(TGT_SDK.soDir)
    for f in glob.glob(os.path.join(TGT_BUILD.soDir, SHARED_GLOB)):
        doCopy(f, os.path.join(TGT_SDK.soDir, os.path.basename(f)))
    if isLinux() and isARM():
        # For ARM linux, we also want the DTM libraries
        CM_SHARED_GLOB='libcm*.so'
        for f in glob.glob(os.path.join(CM_BUILD.soDir,CM_SHARED_GLOB)):
            doCopy(f, os.path.join(TGT_SDK.soDir, os.path.basename(f)))
        

# Update the lib<stub>stub.a files for RTEMS
def updateStubs():
    # meaningless for non-rtems for now
    if not isRtems():  return
    if not os.path.isdir(TGT_SDK.libDir): os.makedirs(TGT_SDK.libDir)
    for stub in STUB_NAMES:
        sName = 'lib%sstub.a' % stub
        doCopy(os.path.join(TGT_BUILD.stubDir, sName), os.path.join(TGT_SDK.libDir,sName))

# Update all task files for RTEMS
def updateTasks():
    # meaningless for non-rtems for now
    if not isRtems():  return
    destDir = TGT_SDK.soDir
    if not os.path.isdir(destDir): os.makedirs(destDir)
    for f in glob.glob(os.path.join(TGT_BUILD.taskDir, RTEMS_TASK_GLOB)):
        doCopy(f, os.path.join(destDir,os.path.basename(f)))

# Update all svt files for RTEMS
def updateSvts():
    # meaningless for non-rtems for now
    if not isRtems():  return
    destDir = TGT_SDK.svtDir
    if not os.path.isdir(destDir): os.makedirs(destDir)
    for f in glob.glob(os.path.join(TGT_BUILD.taskDir, RTEMS_SVT_GLOB)):
        doCopy(f, os.path.join(destDir,os.path.basename(f)))
        
def updateBinaries():
    # Meaningless for non-linux for now
    if not isLinux(): return
    for f in os.listdir(TGT_BUILD.binDir):
        src = os.path.join(TGT_BUILD.binDir,f)
        dest= os.path.join(TGT_SDK.binDir,f)
        if doCopy(src, dest):
            os.chmod(dest, stat.S_IRUSR|stat.S_IWUSR|stat.S_IRGRP|stat.S_IROTH|stat.S_IXUSR|stat.S_IXGRP|stat.S_IXOTH)
    if isARM():
        # FMD is copied from CM
        for b in ['fmd', 'bsi_dump']:
            doCopy(os.path.join(CM_BUILD.binDir,b),os.path.join(TGT_SDK.binDir,b))
        

def updateBootLoader():
    for board in BOOT_ARCH_MAP:
        #bootarch = Architecture(BOOT_ARCH_MAP[board]+
        #                     ('-'+TGT_BUILD.arch.dbgOpt) if TGT_BUILD.arch.dbgOpt else '')
        if board == "dtm":
          #temporarily hardcode dbg
          bootarch = Architecture(BOOT_ARCH_MAP[board]+'-dbg')
        else:
          bootarch = Architecture(BOOT_ARCH_MAP[board]+'-opt')
          
        bootbld = Build(TGT_BUILD.root,bootarch)                  
        destDir = os.path.join(TGT_SDK.sysDir, 'boot', board)
        if not os.path.isdir(destDir): os.makedirs(destDir)
        if os.path.isdir(bootbld.binDir):
            doCopy(os.path.join(bootbld.binDir, 'boot.bin'),
                   os.path.join(destDir, 'boot.bin'))
        else:
            warning('Bootloader build directory %s is not present: not copying boot.bin\n' %
                    bootbld.binDir)
        if board == "dtm":
          if SRC_FPGA_BIT is not None and os.path.isdir(os.path.join(SRC_FPGA_BIT, board)):
              doCopy(os.path.join(SRC_FPGA_BIT, board,'BOOT','fpga.bit'),
                     os.path.join(destDir, 'fpga.bit.dtm'))
        elif board != "zed": 
          if isLinux():
            if SRC_FPGA_BIT is not None and os.path.isdir(os.path.join(SRC_FPGA_BIT, board)):
                doCopy(os.path.join(SRC_FPGA_BIT, board,'BOOT','fpga.bit.10GAxi'),
                       os.path.join(destDir, 'fpga.bit.10GAxi'))
          elif isRtems():                
            if SRC_FPGA_BIT is not None and os.path.isdir(os.path.join(SRC_FPGA_BIT, board)):
                doCopy(os.path.join(SRC_FPGA_BIT, board,'BOOT','fpga.bit.10GPpi'),
                       os.path.join(destDir, 'fpga.bit.10GPpi'))

def updateScripts():
    # ARM linux 
    if isLinux():
        if isARM():
            # reboot_foo and repair_vfat
            for f in glob.glob(os.path.join(opts.work, 'platform', 'rced', 'scripts', '*')):
                doCopy(f, os.path.join(TGT_SDK.binDir, os.path.basename(f)))
        else:
            # ipmi_foo scripts for i86  Only want some of these
            tails = ['activate', 'cmd', 'deactivate', 'get_addr', 'id', 'read', 'write']
            for t in tails:
                f = os.path.join(opts.work, 'bin', 'ipmi_' + t)
                doCopy(f, os.path.join(TGT_SDK.binDir, os.path.basename(f)))
            
    elif isRtems():
        # rtems-foo scripts from bin/
        for f in glob.glob(os.path.join(opts.work, 'bin', 'rtems-*')):
            doCopy(f, os.path.join(TGT_SDK.binDir, os.path.basename(f)))
        
            

def updateExtras():
    if isRtems():
        updateBootLoader()
        # shareable.ld  B.binDir --> S.libDir
        doCopy(os.path.join(TGT_BUILD.binDir, 'shareable.ld'),
               os.path.join(TGT_SDK.libDir,'shareable.ld'))
        # urtems.elf    B.binDir --> S.sysDir/rtems
        doCopy(os.path.join(TGT_BUILD.binDir, 'urtems.elf'),
               os.path.join(TGT_SDK.sysDir, 'rtems', 'urtems.elf'))
        # rtems.so   B.binDir --> S.libDir
        doCopy(os.path.join(TGT_BUILD.binDir, 'rtems.so'),
               os.path.join(TGT_SDK.libDir, 'rtems.so'))
        # appinit.so   B.binSoDir --> S.svtDir
        shutil.copyfile(os.path.join(TGT_BUILD.soDir, 'appinit.so'),
               os.path.join(TGT_SDK.svtDir, 'appinit.so'))
        # Now the source files from the workspace
        for src in RTEMS_SRC_NAMES:
            doCopy(os.path.join(opts.work, src),
                   os.path.join(TGT_SDK.root, 'examples', 'templates', os.path.basename(src)))

    elif isLinux():
        # System Services
        if isARM():
            updateBootLoader()
            kern_files=['uImage', 'devicetree.dtb', 'devicetree_ramdisk.dtb','devicetree.dtb.10GAxi']
            for f in kern_files:
                doCopy(os.path.join(LINUX_STUB, 'arch', 'arm', 'boot', f),
                       os.path.join(TGT_SDK.sysDir, 'linux', 'kernel',f))
            cm_d_files=['dhcpd.dtm.conf', 'dtm_dhcp.service', 'fmd.service', 'dtm_dhcp.sh']
            for f in cm_d_files:
                doCopy(os.path.join(opts.work, 'cm', 'cm_daemon', f),
                       os.path.join(TGT_SDK.sysDir, 'linux', f))
        # PTH
        libpth = os.path.join(PTH_STUB + TGT_ARCH.cpuModel, 'lib', 'libpth.so.20')
        doCopy(libpth, os.path.join(TGT_SDK.libDir, 'libpth.so.20'))
        pth_h  = os.path.join(PTH_STUB + TGT_ARCH.cpuModel, 'include', 'pth.h')
        doCopy(pth_h,  os.path.join(TGT_SDK.incDir, 'pth.h'))
               
        # i86 specific libraries
        if not isARM():
            libfreeipmi = os.path.join(FREEIPMI_LIB_STUB + TGT_ARCH.cpuModel, 'lib', 'libfreeipmi.so.16')
            doCopy(libfreeipmi, os.path.join(TGT_SDK.libDir, 'libfreeipmi.so.16'))


def update():
    updateIncludes()
    updateStubs()
    updateTasks()
    updateSvts()
    updateShared()
    updateBinaries()
    updateExtras()
    updateScripts()

# Perform sanity checks and set reasonable defaults when needed
def setup(opts):
    global TGT_ARCH, TGT_BUILD, CM_ARCH, CM_BUILD
    global SRC_RTEMS_INCLUDE, SRC_FPGA_BIT

    # Without specifying the SDK, how can you go on?
    if not opts.sdk:
        fatal('--sdk option is required\n')

    if not opts.arch:
        fatal('--arch option is required\n')
    TGT_ARCH = Architecture(opts.arch)

    if not (os.path.isdir(opts.work) and
            os.path.isdir(os.path.join(opts.work, 'make'))):
        fatal('Selected workspace directory (%s) is not a workspace', opts.work);

    build=opts.build
    if not build:
        build=os.path.join(opts.work, 'build')

    if not os.path.isdir(build):
        fatal('build directory %s does not exist.  Cannot find built products\n' % opts.build)
    TGT_BUILD = Build(build, TGT_ARCH)

    exclude = None
    if not opts.exclude:
        exclude = os.path.join(WHERE_AM_I,'..', 'sdk', 'exclude.%s' % TGT_ARCH.noDbgOpt)
    else:
        exclude = opts.exclude
    with open(exclude) as f:
        for line in f:
            if not line.startswith('#'):
                SDK_EXCLUDE.append(os.path.join('*/',line.strip()))

    # OS specific sanity checks and setup
    if isRtems():
        SRC_RTEMS_INCLUDE  = opts.rtemsInclude
        if SRC_RTEMS_INCLUDE is None:
            if 'RTEMS_INCLUDE' in os.environ:
                SRC_RTEMS_INCLUDE = os.environ['RTEMS_INCLUDE']

    elif isLinux():
        if isARM():
            CM_ARCH=Architecture('-'.join([TGT_ARCH.family,TGT_ARCH.os,'cmCA9',TGT_ARCH.dbgOpt]))
            CM_BUILD=Build(build, CM_ARCH)
    
    if isRtems() or (isLinux() and isARM()):
      SRC_FPGA_BIT = opts.fpgaSrc
      if SRC_FPGA_BIT is None:
          warning('fpga.bit file location is not set.  Not copying fpga.bit to SDK\n')

    


if __name__ == '__main__':
    import optparse
    import time

    begin = time.time()

    parser = optparse.OptionParser(usage='usage: %prog [options]')
    parser.add_option('--sdk', 
                      help='Location of SDK git directory')
    parser.add_option('--work', default='./',
                      help='Location of workspace (for RTEMS template code)')
    parser.add_option('--build', 
                      help='Location of build directory')
    parser.add_option('--arch', default=None, 
                      help='Target Architecture (e.g. arm-rtems-rceCA9-opt)')
    parser.add_option('--exclude', default=None,
                      help='File of items to exclude')
    parser.add_option('--rtemsInclude', default=None,
                      help='Location of RTEMS Include files')
    parser.add_option('--fpgaSrc', default=None,
                      help='Location of FPGA bit files')
  

    opts,args = parser.parse_args()

    setup(opts)

    TGT_SDK   = SDK(opts.sdk, TGT_ARCH)
    
    update()


