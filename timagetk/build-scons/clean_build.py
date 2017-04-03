# -*- coding: utf-8 -*-
# -*- python -*-
#
#
#       Copyright 2016 INRIA
#
#       File author(s):
#           Guillaume Baty <guillaume.baty@inria.fr>
#           Sophie Ribes <sophie.ribes@inria.fr>
#
#       See accompanying file LICENSE.txt
#------------------------------------------------------------------------------

import os, shutil, glob
separator = os.sep

#--- clean scons files --- WARNING

src_folder = os.getcwd() + separator + 'src'
bin_folder = os.getcwd() + separator + 'bin'
lib_folder = os.getcwd() + separator + 'lib'

if os.path.isdir(bin_folder):
    shutil.rmtree(bin_folder)
if os.path.isdir(lib_folder):
    shutil.rmtree(lib_folder)

folders = ['libio', 'libbasic', 'libvt', 'blockmatching', 'vtexec', 'libdavid', 'libvp']

for index, folder in enumerate(folders):
    folders[index] = src_folder + separator + folder

for folder in folders:
    tmp_os = folder + separator + '*.os'
    files_os = glob.glob(tmp_os)
    for tmp_file_os in files_os:
        os.remove(tmp_file_os)
    tmp_o = folder + separator + '*.o'
    files_o = glob.glob(tmp_o)
    for tmp_file_o in files_o:
        os.remove(tmp_file_o)

os.mkdir(lib_folder)
os.mkdir(bin_folder)