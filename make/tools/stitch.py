#-*-Mode: Python;-*-
# @file
# @brief 
# @verbatim
#                               Copyright 2014
#                                    by
#                       The Board of Trustees of the
#                    Leland Stanford Junior University.
#                           All rights reserved.
# @endverbatim


from architecture import Architecture, Build

import glob
import os
import re
import shutil
import tempfile

DEFAULT_ARCHITECTURES = ['arm-rtems-rceCA9-opt',
                         'arm-linux-rceCA9-opt',
                         'i86-linux-32-opt',
                         'i86-linux-64-opt']


class Environment:
    DOC_DIR_ROOT = "Doxygen-%s"
    def __init__(self, workspace, devel, sdk_ver):
        self.workspace  = workspace
        self.devel      = devel
        self.version    = sdk_ver
        self.builds     = []
    @property
    def docDir(self):
        return os.path.join(self.workspace, self.DOC_DIR_ROOT % self.version)

# Create the document root directory and the architecture subdirs
def createDocDir(environment):
    if os.path.exists(environment.docDir):
        print "Warning: Documentation root directory %s already exists." % environment.docDir
        print "         Backing up to %s " % (environment.docDir + ".bak")
        os.rename(environment.docDir, environment.docDir + ".bak")

    os.makedirs(environment.docDir)
    for build in environment.builds:
        os.makedirs(os.path.join(environment.docDir, build.arch.noDbgOpt))

# Pull the docs into the root
def copyBuild(environment):
    for build in env.builds:
        
        src = ""
        dst = os.path.join(environment.docDir, build.arch.noDbgOpt)
        if env.devel:
            src = build.devDocDir
        else:
            src = build.usrDocDir

        if not os.path.exists(src): continue
        
        for dd in os.listdir(src):
            shutil.copytree(os.path.join(src, dd),
                            os.path.join(dst, dd))

# Gather the list of built docs from each arch
def gatherTree(environment):
    docLists = {}
    packages  = set()

    for build in environment.builds:
        docLists[build.arch.noDbgOpt] = os.listdir(os.path.join(env.docDir, build.arch.noDbgOpt))
        packages.update(docLists[build.arch.noDbgOpt])

        
    return docLists, sorted(list(packages))

# From the lists of documents built, create the stitching html
def stitchDocs(environment, docLists, packages):
    stitch_template = os.path.join(environment.workspace,
                                   'make', 'tools', 'Doxypkg.template')

    tmp_stitch = tempfile.NamedTemporaryFile(delete=False)

    with open(stitch_template, 'r') as st:
        for line in st:
            nl = re.sub( '\$\$Version\$\$', environment.version, line )
            # Add the table header
            if re.search( '\$\$STITCH_TABLE_HEADER\$\$', nl ):
                tmp_stitch.write( '<tr>\n' )
                tmp_stitch.write( '  <td width="20%" valign="top"><font face="Helvetica" size="2"><b>Package</b></font></td>\n' )
                
                for k in sorted(docLists.keys()):
                    tmp_stitch.write( '  <td width="20%" valign="top" align="center">')
                    tmp_stitch.write( '<font face="Helvetica" size="2"><b>%s</b></font></td>\n' % k )
                tmp_stitch.write( '</tr>\n' )
                nl = ""

            # Add the rows.  Use the contents of packages since that's a superset of each build
            if re.search( '\$\$STITCH_TABLE_ROWS\$\$', nl ):
                for p in packages:
                    tmp_stitch.write( '<tr>\n' )
                    tmp_stitch.write( '  <td width="20%" valign="top"><font face="Helvetica" size="2">' )
                    tmp_stitch.write( '<b>%s<b></font></td>\n' % p )
                    for k in sorted(docLists.keys()):
                        v = docLists[k]
                        tmp_stitch.write( '  <td width="20%" valign="top" align="center">' )
                        if p in v:
                            tmp_stitch.write( '<a href="%s/%s/html/"><font face="Helvetica" size="2">>>></font></a>' %
                                              (k, p) )
                        else:
                            tmp_stitch.write( '<font face="Helvetica" size="2">---</font>' )
                        tmp_stitch.write( '</td>\n' )                                              
                
                    tmp_stitch.write( '</tr>\n' )
                nl = ""
            if nl:
                tmp_stitch.write(nl)
    tmp_stitch.close()
    shutil.copyfile(tmp_stitch.name, os.path.join(environment.docDir, 'Doxypkg.html'))

    os.unlink(tmp_stitch.name)
    
def finalize(environment):
    files = set(['tabs.css', 'navtree.css', 'doxygen.css', 'jquery.js', 'navtree.js', 'resize.js'])
    src = ''
    # Find a directory with the css and js
    for ad in os.listdir(environment.docDir):
        if os.path.isdir(os.path.join(environment.docDir, ad)):
            for pkg in os.listdir(os.path.join(environment.docDir, ad)):
                if files.issubset(set(os.listdir(os.path.join(environment.docDir, ad, pkg, 'html')))):
                    src = os.path.join(environment.docDir, ad, pkg, 'html')
                if src: break
        if src: break

    if not src:
        raise RuntimeError("Could not find all files: %s" % files )
    for f in files:
        shutil.copyfile(os.path.join(src,f), os.path.join(env.docDir, f))
    for f in glob.glob(os.path.join(src, '*.png')):
        if '_' in f: continue
        shutil.copyfile(f, os.path.join(env.docDir, os.path.basename(f)))

def setup(args):
    # Find the workspace and build root directory.
    if not os.path.exists(args.workspace): 
        raise RuntimeError ("Workspace area %s does not exist." % args.workspace )
    workspace = args.workspace
    
    bldDir = os.path.join(workspace, 'build')
    if not os.path.exists(bldDir):
        raise RuntimeError ("Workspace build area %s does not exist." % bldDir )

    env = Environment(workspace, args.developer, args.sdkVer)
    for archname in args.arch:
        arch = Architecture(archname)
        env.builds.append(Build(bldDir, arch))

#####+++++
#    print "+++++ build directories (setup)"
#    for b in env.builds:
#        print "  ", b.archDir
#    print "----- build directories (setup)"
#####+++++

    return env


def commandLine():
    import argparse
    cmdLine = argparse.ArgumentParser(description='test stitching')

    required = cmdLine.add_argument_group('Required Arguments')

    cmdLine.add_argument('--arch', action='append',
                          help='Architectures to stitch',
                          default=DEFAULT_ARCHITECTURES)
    
    required.add_argument('--sdkVer', required=True)

    cmdLine.add_argument('--developer', action='store_true', default=False,
                        help='Build developer and private documentation.')

    # Need a sensible default for the workspace:
    workspace = os.environ['RELEASE_DIR'] if 'RELEASE_DIR' in os.environ.keys() else "/INVALID"
    cmdLine.add_argument('--workspace', default=workspace)
                        
    return cmdLine.parse_args()



if __name__ == "__main__":

    args = commandLine()

    env = setup(args)

    createDocDir(env)

    copyBuild(env)

    docLists, packages = gatherTree(env)

    stitchDocs(env, docLists, packages)

    finalize(env)
