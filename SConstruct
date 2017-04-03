#!python
#
#       Copyright 2016 INRIA
#
#       File author(s): Sophie Ribes <sophie.ribes@inria.fr>
#			Guillaume Baty <guillaume.baty@inria.fr>
#
#       Distributed under the Cecill-C License.
#       See accompanying file LICENSE.txt or copy at
#           http://www.cecill.info/licences/Licence_CeCILL-C_V1-en.html
#
###############################################################################

import os
separator = os.sep
import platform
import glob

name = 'timagetk'
folder_name = 'build-scons'

# GCC compiler
if ('GCC' in platform.python_compiler()):
    CC_val = 'gcc'
    CCFLAGS_val = '-O2 -Wall -Wextra -ansi -fsigned-char -fsigned-bitfields -pthread'

env = Environment(CC = CC_val, CCFLAGS = CCFLAGS_val)
#print('CC:'), env['CC']
#print("CCCOM is:"), env['CCCOM']
#print("CCCOM is:"), env.subst('$CCCOM')

if (platform.system() == 'Darwin') or (platform.system() == 'Linux'):
    env.Append(LIBS=['pthread', 'z', 'm'])

build_dir =  os.getcwd() + separator + name + separator + folder_name
env['build'] = build_dir
#print env['build']

folders = ['bin', 'include', 'lib', 'src']
for folder in folders:
    tmp_dir = build_dir + separator + folder
    if not os.path.isdir(tmp_dir):
        os.mkdir(tmp_dir)
    if folder == 'bin':
        env['bin'] = tmp_dir
    elif folder == 'include':
        env['include'] = tmp_dir
    elif folder == 'lib':
        env['lib'] = tmp_dir
    elif folder == 'src':
        env['src'] = tmp_dir

#print env['bin']
#print env['include']
#print env['lib']
#print env['src']

src = os.getcwd() + separator + name + separator + folder_name + separator + 'src'
folders = ['libio', 'libbasic', 'libvt', 'blockmatching', 'vtexec', 'libdavid', 'libvp']

sconscript_list = []

for folder in folders:

    if (folder != 'vt-exec'):
        var = src + separator + folder + separator
        env[folder] = var
    elif (folder == 'vt-exec'):
        var = src + separator + 'vt-exec' + separator
        env['vtexec'] = var

    var = var + 'SConscript'
    sconscript_list.append(var)

#print sconscript_list
SConscript(sconscript_list, exports={"env": env})
