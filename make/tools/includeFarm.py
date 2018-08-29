#!/usr/bin/env python
# Create a bunch of soft links of files into an include farm
#
# Directory structure
# include/
#   package0/
#     foo.h, bar.hh  link to originals
#     os/
#     cpu/
#     platform/
#   package1/
#
# First argument is the location of the include farm (absolute)
# Subsequent arguments are the locations of the files to link
# (relative to $RELEASE_DIR)
#
#
#

import sys, shutil, os, glob, errno, subprocess

from architecture import Architecture

INC_GLOBS = [os.path.join('include', '*.h'), 
             os.path.join('include', '*.hh')
             ]

MAKE=['make', '--no-print-directory', '-s', '-f', '-']

RELEASE_DIR=os.environ['RELEASE_DIR']
TGT_ARCH=None


# Call make via subprocess and return any output.
# Output is split on word boundary into a list of strings
def make(target, fstring, incdir=None):
    env = os.environ
    if "MAKEFLAGS" in env: env.pop("MAKEFLAGS")
    #print 'jhpdebug:', env.keys()
    args = ["-C", RELEASE_DIR ,target, 'RELEASE_DIR=%s'%RELEASE_DIR, 'tgt_arch=%s'%TGT_ARCH]
    if incdir:
        args = args+['-I', incdir]
    tmp = subprocess.Popen(MAKE + args, env=env,
                           stdout=subprocess.PIPE, stdin=subprocess.PIPE)
    out,err = tmp.communicate(input=fstring)
    return out.split()

# inline makefile fragments which leverage the
# workspace/project/package/constituents method of building
# into a system which can report the contents of a build 
# in order to harvest for the SDK.  All makefiles contain
# path references starting at the build workspace

# Provides a list of projects, codetypes and platforms
MAKE_TOPLEVEL="""
include projects.mk
include compilation.mk
projects: 
\t@echo $(projects)
"""

# Takes one argument: the project
MAKE_PROJECT="""
include make/sw/flags.mk
include %s/packages.mk
packages:
\t@echo $(packages)
"""

def getProjects():
    return make('projects', MAKE_TOPLEVEL)

def getPackages(project):
    return make('packages', MAKE_PROJECT % (project), project)



# create a soft link, preventing failures if a soft link already exists
def mkLink(src, dest):
    if not os.path.isdir(os.path.dirname(dest)):
        os.makedirs(os.path.dirname(dest))
    try:
        os.symlink(src, dest)
    except OSError, e:
        if e.errno == errno.EEXIST: return
        print e, src, dest
        return
    #print "Created link %s -> %s" % (dest, src)

# Create a soft link only if the source directory exists
def mkLinkIfNeeded(src, dest):
    if os.path.exists(src):
        mkLink(src, dest)

def linkIncludeSubdirs(packageLocation, farmLocation, architecture):
    # Create the link farm, depending on what is actually 
    # in the package.
    
    packageName = os.path.split(packageLocation.strip(os.sep))[1]
    destDir = os.path.join(farmLocation, packageName)

    mkLinkIfNeeded(os.path.join(packageLocation, 'impl'), 
                   os.path.join(destDir, 'impl'))

    for d in os.listdir(packageLocation):
        
        for match in architecture.match(d):
            mkLinkIfNeeded(os.path.join(packageLocation, d), 
                           os.path.join(destDir, match))



def linkPublicIncludes(packageLocation, destination):
    packageName = os.path.split(packageLocation.strip(os.sep))[1]
    pubIncs = set()
    if not os.path.isdir(os.path.join(destination, packageName)):
        os.makedirs(os.path.join(destination, packageName))
    for g in INC_GLOBS:
        pubIncs.update(glob.glob(os.path.join(packageLocation,g)))
    for inc in pubIncs:
        mkLink(inc, os.path.join(destination, packageName, os.path.basename(inc)))

if __name__ == "__main__":
    import optparse
    #print "jhpDebug: ", sys.argv, os.environ['PWD']
    
    parser = optparse.OptionParser()
    parser.add_option('-a', '--arch', dest="architecture", help='Architecture')
    parser.add_option('-d', '--dest', dest="destination", help='Link farm destination')

    opts,args = parser.parse_args()
    arch = Architecture(opts.architecture)
    TGT_ARCH=opts.architecture

    for prj in getProjects():
        for pkg in getPackages(prj):
            pLoc = os.path.join(RELEASE_DIR,prj,pkg)
            linkPublicIncludes(pLoc, opts.destination)
            linkIncludeSubdirs(pLoc, opts.destination, arch)

    #sys.exit(-1)


