#!/usr/bin/env python
# -*- coding: utf-8 -*-

import sys, os, platform, subprocess
separator = os.sep
from setuptools import setup, find_packages
import timagetk

#--- still under dev.
# from setuptools.extension import Extension
#try:
#    from Cython.Build import cythonize
#except ImportError:
#    use_cython = False
#else:
#    use_cython = True
#---

top_folder = os.getcwd()
name = 'timagetk'
author = 'Sophie Ribes'
author_email = 'sophie.ribes@inria.fr'
short_description = 'Tissue Image Toolkit (timagetk) is a \
Python language package dedicated to image processing of multicellular architectures, \
and is intended for biologists, modelers and computer scientists.'
with open(os.path.join(top_folder, 'README.rst')) as f:
    long_description = f.read()

if sys.argv[-1] == 'setup.py':
    print("Type: 'python setup.py develop --user'")

if sys.version_info[:2] < (2, 7):
    print("Install Python 2.7 or later (%d.%d detected)." %
          sys.version_info[:2])
    sys.exit(-1)

supp_platforms = ['Darwin', 'Linux']
if (platform.system() not in supp_platforms):
    print("Supported platforms"), supp_platforms
    sys.exit(-1)

has_scons = True
if has_scons:
    build_prefix = os.path.join(top_folder, str(name), 'build-scons')
    scons_scripts = ['SConstruct']
    src_dirs = os.path.join(build_prefix, 'src')
    lib_dirs = os.path.join(build_prefix, 'lib')
    inc_dirs = os.path.join(build_prefix, 'include')
    bin_dirs = os.path.join(build_prefix, 'bin')
    subprocess.call('scons', shell=True)
    timagetk_path = ''.join(['timagetk_path=', top_folder, '\n'])
    home_list = os.listdir(os.getenv("HOME"))
    if '.bashrc' or '.profile' in home_list:
        if platform.system()=='Linux':
            export_val = 'export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:${timagetk_path}/timagetk/build-scons/lib\n'
            fic_path = ''.join([os.getenv("HOME"), separator, '.bashrc'])
        elif platform.system()=='Darwin':
            export_val = 'export DYLD_LIBRARY_PATH=$DYLD_LIBRARY_PATH:${timagetk_path}/timagetk/build-scons/lib\n'
            fic_path = ''.join([os.getenv("HOME"),separator, '.profile'])
        with open(fic_path,'r') as fic:
            all_lines = [line for line in fic]
            fic.close()
        if timagetk_path not in all_lines:
            with open(fic_path,'a') as fic:
                fic.write('\n')
                fic.write('\n')
                fic.write(timagetk_path)
                fic.close()
        if export_val not in all_lines:
            with open(fic_path,'a') as fic:
                fic.write(export_val)
                fic.close()
        src_cmd = ''.join(['source ', fic_path])
        subprocess.call(src_cmd, shell=True)

#--- still under dev.
#if use_cython:
#    geom_module_opt = os.path.join(top_folder, str(name), 'algorithms', 'geometrical_features.pyx')
#    extensions = [
#        Extension(
#                  'geometrical_features',
#                  [geom_module_opt]),
#                 ]
#    ext_modules_opt = cythonize(extensions)
#else:
#    geom_module_opt = os.path.join(top_folder, str(name), 'algorithms', 'geometrical_features.c')
#    extensions = [
#        Extension(
#                  'timagetk.algorithms.geometrical_features',
#                  [geom_module_opt]),
#                 ]
#    ext_modules_opt = extensions
#---
setup(
    name=name,
    version=timagetk.__version__,
    packages=find_packages(),
    author=author,
    author_email=author_email,
    description=short_description,
    long_description=long_description,
    #install_requires=[],
    include_package_data=True,
    #ext_modules=ext_modules_opt,
    url='https://github.com/VirtualPlants/timagetk',
    classifiers=[
        "Programming Language :: Python",
        "Development Status :: 5 - Production/Stable",
        "Intended Audience :: Science/Research",
        "License :: Free To Use But Restricted",
        "Natural Language :: English",
        "Operating System :: OS Independent",
        "Programming Language :: Python :: 2.7",
        "Topic :: Scientific/Engineering",
    ],
    entry_points = {
        'openalea.image': ['openalea.image/image_plugin = timagetk.plugins.plugin'],
                    },
    license="Free To Use But Restricted",
    platforms=['LINUX','MAC']
)
