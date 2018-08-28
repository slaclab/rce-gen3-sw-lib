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

import os
import re
import shutil
import subprocess
import sys
import tempfile

from architecture import Architecture, Build

class LocalEnv:
    def __init__(self, build, package, project,
                 workspace, sdkVer, devel_flag, c_only_flag):
        self._bld  = build
        self._pkg  = package
        self._prj  = project
        self._wks  = workspace
        self._ver  = sdkVer
        self._dvl  = devel_flag
        self._c    = c_only_flag
    @property
    def arch(self):      return self.build.arch
    @property
    def build(self):     return self._bld
    @property
    def package(self):   return self._pkg
    @property
    def project(self):   return self._prj
    @property
    def workspace(self): return self._wks
    @property
    def version(self):   return self._ver
    @property
    def developer(self): return self._dvl
    @property
    def c_only(self):    return self._c

def getPackageProject():
    # Since we're running in workspace/project/package, we can parse
    # this out of curdir
    pwd = os.path.realpath(os.path.curdir)

    # Drop trailing slash and tokenize
    pwd.rstrip('/')
    dirs = pwd.split('/')
    if len(dirs) < 2:
        raise RuntimeError( "Cannot set package and project name")
    package = dirs[-1]
    project = dirs[-2]
    return package, project
    
def convertTemplate(doxy_source, environment):
    file_template = os.path.join(environment.workspace,
                                 'make', 'tools', 'Doxyfile.template')
    head_template = os.path.join(environment.workspace,
                                 'make', 'tools', 'Doxyhead.template')
    nav_template  = os.path.join(environment.workspace,
                                 'make', 'tools', 'Doxynav.template')
    nav_lines     = []  # a transcription of nav_template with wildcards fixed
    
    tmp_file = tempfile.NamedTemporaryFile(delete=False)
    tmp_head = tempfile.NamedTemporaryFile(delete=False)

    # local temp vars for brevity
    bld = environment.build
    arch = environment.arch
    developer = environment.developer

    # Inputs to Doxygen
    input_str    = ' '.join(sorted(doxy_source))
    extract_pvt  = 'YES'           if developer else 'NO'
    internal_doc = 'YES'           if developer else 'NO'
    enabled_sec  = 'development'   if developer else ''
    doc_root     = bld.devDocDir   if developer else bld.usrDocDir
    output_dir   = os.path.join(doc_root, environment.package)
    c_only       = 'YES'           if environment.c_only else 'NO'


    # Read in the navigation template and make the substitutions
    with open(nav_template, 'r') as nt:
        for line in nt:
            nl = re.sub( '\$\$Version\$\$', environment.version, line )
            nl = re.sub( '\$\$Target\$\$',  arch.noDbgOpt, nl )
            nl = re.sub( '\$\$Package\$\$', environment.package, nl )
            nav_lines.append(nl)

    # Transcribe the header template, making substitutions
    with open(head_template, 'r') as ht:
        for line in ht:
            nl = re.sub( '\$\$Package\$\$', environment.package, line )
            if re.search( '\$\$NavBar\$\$', nl ):
                for n in nav_lines:
                    tmp_head.write(n)
            else:
                tmp_head.write(nl)
    tmp_head.close()

    with open(file_template, 'r') as dt:
        for line in dt:
            nl = re.sub( '\$\$DOXY_INPUT\$\$',        input_str,   line )
            nl = re.sub( '\$\$DOXY_INTERNAL_DOC\$\$', internal_doc, nl )
            nl = re.sub( '\$\$DOXY_EXTRACT_PVT\$\$',  extract_pvt, nl )
            nl = re.sub( '\$\$DOXY_ENABLED_SEC\$\$',  enabled_sec, nl )
            nl = re.sub( '\$\$DOXY_OUTPUT_DIR\$\$',   output_dir,  nl )
            nl = re.sub( '\$\$DOXY_C_ONLY\$\$',       c_only,      nl )
            nl = re.sub( '\$\$DOXY_HTML_HEADER\$\$',
                         os.path.join(output_dir, "Doxyhead.html"),  nl )
            tmp_file.write(nl)
    tmp_file.close()

    if not os.path.exists(output_dir):
        os.makedirs(output_dir)
    shutil.copyfile(tmp_file.name, os.path.join(output_dir, "Doxyfile"))
    shutil.copyfile(tmp_head.name, os.path.join(output_dir, "Doxyhead.html"))


    os.unlink(tmp_file.name)
    os.unlink(tmp_head.name)


def runDoxygen(environment):

    developer  = environment.developer
    bld        = environment.build
    doc_root   = bld.devDocDir   if developer else bld.usrDocDir
    output_dir = os.path.join(doc_root, environment.package)
    doxy_file  = os.path.join(output_dir, "Doxyfile")
    
    c1 = subprocess.Popen( ['/bin/cat', doxy_file], stdout=subprocess.PIPE )
    
    c2 = subprocess.Popen( ['/usr/local/bin/doxygen','-'],
                               stdin=c1.stdout,
                               stdout=subprocess.PIPE,
                               stderr=subprocess.PIPE,
                               close_fds=True,
                               universal_newlines=True )
        
    rawout, rawerr = c2.communicate()
    status         = c2.returncode

    with open(os.path.join(output_dir, 'doxygen.stdout'), 'w') as dout:
        for line in rawout.splitlines():
            dout.write(line)
            dout.write('\n')

    with open(os.path.join(output_dir, 'doxygen.stderr'), 'w') as derr:
        for line in rawerr.splitlines():
            derr.write(line)
            derr.write('\n')

    #print "Doxygen completed with status %d" % status


def parseDepFile(depFilePath, environment):
    # There are three classes of dependencies:
    inc_farm   = set()  # includes from the include farm
    inc_ws     = set()  # includes from the workspace directly
    dep_rel    = set()  # relative dependencies (to CWD)
    try:
        with open(depFilePath, 'r') as fi:
            for line in fi:
                # Lose the whitespace
                line = line.strip(' \\\n')
                # Remove any colons
                line = re.sub(':', '', line)
                
                # Split lines to individual deps and categorize 
                lst = re.split( r'(?<!\\)(?:\\\\)*\s+', line)
                for ele in lst:
                    # Filter out the dependency file and the object file
                    if ele.endswith('.d') or ele.endswith('.o'): continue

                    # Chop off the known parts of the paths
                    ele = os.path.normpath(ele)
                    if ele.startswith(environment.build.incDir):
                        inc_farm.add( ele[ len(environment.build.incDir)+1: ] )
                    elif ele.startswith(environment.workspace):
                        inc_ws.add( ele[len(environment.workspace)+1:] )
                    elif ele.startswith( '/' ):
                        print "!!!!! Unhandleable dependency: %s" % ele
                        continue
                    else:  # src/ file 
                        dep_rel.add(ele)
                
    except Exception, e:
        print "!!!!! Failed parsing dependency file %s" % depFilePath

    # include farm files for this package only
    while len(inc_farm):
        inc_sp = inc_farm.pop().split('/')
        if inc_sp[0] != environment.package:  continue

        if inc_sp[1] in ('gen', 'family', 'os', 'board', 'cpu', 'platform'):
            # undo the indirection in the include farm
            # This construct allows calling a fn only known at runtime
            inc_sp[1] = Architecture.__dict__[ inc_sp[1] ].fget(environment.arch)
            dep_rel.add( os.path.join(*inc_sp[1:]) )
        elif len(inc_sp[1:]) == 1:
            # Add include to straight include files
            dep_rel.add( os.path.join('include', *inc_sp[1:]) )
        else:
            dep_rel.add( os.path.join(*inc_sp[1:]) )

    # include files relative to the workspace.  Only care about
    # those in this package
    while len(inc_ws):
        inc_sp = inc_ws.pop().split('/')
        if inc_sp[1] != environment.package: continue

        raise NotImplementedError("Waiting for a test case -- JHP.")
        
#####+++++
#    print "+++++ Parsed dependency file %s" % depFilePath
#    for df in sorted(dep_rel):
#        print "  ", df
#    print "----- Parsed dependency file %s" % depFilePath
#####-----

    return dep_rel


def gatherDependencies(srcList, environment):
    dd = os.path.join(environment.build.depDir, environment.project, environment.package)
    deps = set()

    # Construct list of dependency files related to the input sources
    depFiles = set()
    for sf in srcList:
        # Change extension to .d and add to our set if the dependency file exists
        sf_d = re.sub( '\..*$', '.d', sf )
        df = os.path.join(dd, sf_d)
        if os.path.exists(df):
            depFiles.add(os.path.join(dd, sf_d))
        else:
            # Case for "doc only" source files which don't generate a .d
            deps.add(sf)            

    # parse out package dependencies from depFiles.
    for df in depFiles: deps |= parseDepFile(df, environment)

#####+++++
#    print "+++++ dependency input files"
#    for nam in depFiles:
#        print "  ", nam
#    print "----- dependency input files"
#####-----
#####+++++
#    print "+++++ dependencies"
#    for df in sorted(deps):
#        print "  ", df
#    print "----- dependencies"
#####-----

    return deps

def pruneDependencies(deps, unwanted, environment):
    # This operation breaks down to simple set arithmetic
    unw_s = set(unwanted)

    deps -= unw_s
    return deps

    

def setup(args):
    # Find the workspace and build root directory.
    if not os.path.exists(args.workspace): 
        raise RuntimeError ("Workspace area %s does not exist." % args.workspace )
    workspace = args.workspace
    
    bldDir = os.path.join(workspace, 'build')
    if not os.path.exists(bldDir):
        raise RuntimeError ("Workspace build area %s does not exist." % bldDir )
    
    # Architecture is used for deconvolving file paths
    arch = Architecture(args.arch)
    # Build provides accessors for the doc dirs
    build = Build(bldDir, arch)

    pack, proj = getPackageProject()

    # By design, there must be a 'package.doc' in the package directory
    # (which is also the CWD)
    if not os.path.exists('package.doc'):
        raise RuntimeError("File package.doc does not exist in %s", os.path.realpath(os.path.curdir))

    localEnv = LocalEnv(build, pack, proj, workspace, args.sdkVer, args.developer, args.c)
    return localEnv
    

def commandLine():
    import argparse
    cmdLine = argparse.ArgumentParser(description='Create doxygen output for exactly one package')

    required = cmdLine.add_argument_group('Required arguments')
    
    required.add_argument('--arch', required=True, 
                        help='Target Architecture (e.g. arm-rtems-rceCA9-opt)')

    source   = cmdLine.add_argument_group('Arguments controlling what gets documented')
    source.add_argument('--devSource', nargs='*', default=[],
                         help="Developer-only source files to document")

    source.add_argument('--source', nargs='+', default=[], required=True,
                        help='Source files to document')

    source.add_argument('--exclude',
                        nargs='*', default=[],
                        help='Source files to *not* document.')

    cmdLine.add_argument('--developer', action='store_true', default=False,
                        help='Build developer and private documentation.')

    source.add_argument('--devExclude',
                        nargs='*', default=[],
                        help='Developer source files to *not* document.')

    source.add_argument('-c', action='store_true', default=False,
                        help='Package contains C code only')

    cmdLine.add_argument('--sdkVer', default="VX.Y.Z")


    # Need a sensible default for the workspace:
    workspace = os.environ['RELEASE_DIR'] if 'RELEASE_DIR' in os.environ.keys() else "/INVALID"
    cmdLine.add_argument('--workspace', default=workspace)
                        

    return cmdLine.parse_args()
    

if __name__ == '__main__':

    args = commandLine()
    environment = None
    
    try:
        environment = setup(args)
    except RuntimeError, e:
        print "!!!! Doxygen generation setup failed with error: %s" % e
        print "!!!! CWD=%s" % os.path.realpath(os.path.curdir)
        
        sys.exit(-1)

    # Files which get input to Doxygen
    doxy_source  = set(['package.doc'])
    devel_source = set()

    doxy_source |= gatherDependencies(args.source, environment)
    doxy_source  = pruneDependencies(doxy_source, args.exclude, environment)

    if args.developer:
        devel_source |= gatherDependencies(args.devSource, environment)
        devel_source  = pruneDependencies(devel_source, args.devExclude, environment)
    doxy_source |= devel_source

#####+++++
#    print "+++++ doxy_source"
#    for df in sorted(doxy_source):
#        print "  ", df
#    print "----- doxy_source"
#####-----

    try:
        convertTemplate(doxy_source, environment)
        runDoxygen(environment)
    except Exception, e:
        print "!!!! Doxygen generation failed with error: %s" % e
        sys.exit(-1)
