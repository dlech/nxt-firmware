# This scons build script is used by NxOS

from glob import glob

Import('env')


for source in glob('Debugger/*.[cS]'):
    obj = env.Object(source.split('.')[0], source)
    env.Append(NXOS_DEBUG=obj)

if env['WITH_DOXYGEN']:
    env.Doxygen('Doxyfile')
