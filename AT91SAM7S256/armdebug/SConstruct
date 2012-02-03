# -*- mode: python -*-
###############################################################
# This scons build script is used to check the armdebug project
# code for syntax errors. It does not build working executable
# code since it links to external routines.
###############################################################

import os
import os.path
import new
from glob import glob

###############################################################
# Utility functions.
###############################################################

# Similar to env.WhereIs, but always searches os.environ.
def find_on_path(filename):
    paths = os.environ.get('PATH')
    if not paths:
        return None
    for p in paths.split(':'):
        path = os.path.abspath(os.path.join(p, filename))
        if os.path.isfile(path):
            return p
    return None

# Run the given gcc binary, and parses its output to work out the gcc
# version.
def determine_gcc_version(gcc_binary):
    stdout = os.popen('%s --version' % gcc_binary)
    gcc_output = stdout.read().split()
    stdout.close()
    grab_next = False
    for token in gcc_output:
        if grab_next:
            return token
        elif token[-1] == ')':
            grab_next = True
    return None

# Check that a given cross-compiler tool exists. If it does, the path is
# added to the build environment, and the given environment variable is
# set to the tool name.
#
# This is used to check for the presence of a working cross-compiler
# toolchain, and to properly set up the environment to do it. See below
# in the configuration section for details.
def CheckTool(context, envname, toolname=None, hostprefix=None):
    toolname = toolname or envname.lower()
    if hostprefix is None:
        hostprefix = '%s-' % context.env['CROSS_COMPILE_HOST']
    toolname = '%s%s' % (hostprefix, toolname)
    context.Message("Checking for %s..." % toolname)
    toolpath = find_on_path(toolname)
    if not toolpath:
        context.Result('not found')
        return False
    else:
        context.Result('ok')
        context.env[envname] = toolname
        context.env.AppendENVPath('PATH', toolpath)
        return True

# Find the correct variant and version of libgcc.a in the cross-compiler
# toolchain.
def CheckLibGcc(context, gccname):
    context.Message("Locating a cross-compiled libgcc...")
    toolpath = find_on_path(gccname)
    if not toolpath:
        context.Result("%s not found" % toolname)
        return False
    gcc_version = determine_gcc_version(gccname)
    if not gcc_version:
        context.Result("Could not determine gcc version")
        return False
    gcc_install_dir = os.path.split(os.path.normpath(toolpath))[0]
    for libdir in ['interwork', 'thumb', '']:
        libgcc_path = os.path.join(gcc_install_dir, 'lib', 'gcc',
                               context.env['CROSS_COMPILE_HOST'],
                               gcc_version, libdir, 'libgcc.a')
        if os.path.isfile(libgcc_path):
            break
    if not os.path.isfile(libgcc_path):
        context.Result("libgcc.a not found")
        return False
    context.Result("ok - " + libgcc_path)
    context.env.Append(LIBGCC=libgcc_path)
    return True

def CheckDoxygen(context):
    context.Message("Looking for Doxygen...")
    doxypath = find_on_path('doxygen')
    if doxypath:
        context.Result("ok")
        context.env.AppendENVPath('PATH', doxypath)
        context.env['WITH_DOXYGEN'] = True
    else:
        context.Result("not found")
        context.env['WITH_DOXYGEN'] = False



###############################################################
# Options that can be provided on the commandline
###############################################################

opts = Variables('scons.options', ARGUMENTS)

opts.Add(PathVariable('gccprefix',
                    'Prefix of the cross-gcc to use (by default arm-none-eabi)',
                    'arm-none-eabi', PathVariable.PathAccept))

Help('''
Type: 'scons' to build object files.

 - To use another cross-gcc than arm-none-eabi-gcc:
     scons gccprefix=arm-softfloat-eabi

Options are saved persistent in the file 'scons.options'. That means
after you have called e.g. 'scons gccprefix=arm-softfloat-eabi' it's enough
to call only 'scons' to build both using the new gcc version again.
''')

###############################################################
# Construct and configure a cross-compiler environment
###############################################################
env = Environment(options = opts,
                  tools = ['gcc', 'as', 'gnulink', 'ar'],
                  toolpath = ['scons_tools'],
                  LIBGCC = [], CPPPATH = '#',
                  WITH_DOXYGEN = False)
opts.Update(env)
opts.Save('scons.options', env)

if not env.GetOption('clean'):
    conf = Configure(env, custom_tests = {'CheckTool': CheckTool,
                                          'CheckLibGcc': CheckLibGcc,
                                          'CheckDoxygen': CheckDoxygen})
    conf.env['CROSS_COMPILE_HOST'] = env['gccprefix']
    if not (conf.CheckTool('CC', 'gcc') and conf.CheckTool('AR') and
            conf.CheckTool('OBJCOPY') and conf.CheckTool('LINK', 'ld') and
            conf.CheckLibGcc(conf.env['CC'])):
        print "Missing or incomplete arm-elf toolchain, cannot continue!"
        Exit(1)
    env = conf.Finish()

mycflags = ['-mcpu=arm7tdmi', '-Os', '-Wextra', '-Wall', '-Werror',
                      '-Wno-div-by-zero', '-Wfloat-equal', '-Wshadow',
                      '-Wpointer-arith', '-Wbad-function-cast',
                      '-Wmissing-prototypes', '-ffreestanding',
                      '-fsigned-char', '-ffunction-sections', '-std=gnu99',
                      '-fdata-sections', '-fomit-frame-pointer', '-msoft-float']
myasflags = ['-Wall', '-Werror', '-Os'];
if str(env['LIBGCC']).find('interwork') != -1:
    mycflags.append('-mthumb-interwork')
    myasflags.append('-Wa,-mcpu=arm7tdmi,-mfpu=softfpa,-mthumb-interwork')
elif str(env['LIBGCC']).find('thumb') != -1:
    mycflags.append('-mthumb')
    myasflags.append('-Wa,-mcpu=arm7tdmi,-mfpu=softfpa,-mthumb')
else:
    myasflags.append('-Wa,-mcpu=arm7tdmi,-mfpu=softfpa')
mycflags.append('-g')
mycflags.append('-ggdb')
# Big Endian Output (disabled by default)
#mycflags.append('-D__BIG_ENDIAN__')
# Test build for NxOS (Comment out for NXT Firmware)
mycflags.append('-D__NXOS__')

myasflags.append('-g')
myasflags.append('-ggdb')
# Big Endian Output (disabled by default)
#mycflags.append('-D__BIG_ENDIAN__')
# Test build for NxOS (Comment out for NXT Firmware)
myasflags.append('-D__NXOS__')

env.Replace(CCFLAGS = mycflags, ASFLAGS = myasflags )

# Build the baseplate, and all selected application kernels.

numProcs = os.sysconf('SC_NPROCESSORS_ONLN')
SConscript(['SConscript'], 'numProcs env CheckTool')
