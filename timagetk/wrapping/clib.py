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

import platform
from ctypes import cdll, c_void_p, c_char_p, POINTER
from ctypes.util import find_library
try:
    from timagetk.wrapping.balTrsf import BAL_TRSF
except ImportError:
    raise ImportError('Import Error')


if (platform.system() == 'Linux'):
    try:
        libbasic = cdll.LoadLibrary('libbasic.so')
        libblockmatching = cdll.LoadLibrary('libblockmatching.so')
        libvt = cdll.LoadLibrary('libvt.so')
        libvp = cdll.LoadLibrary('libvp.so')
        libvtexec = cdll.LoadLibrary('libvtexec.so')
        libio = cdll.LoadLibrary('libio.so')
        libdavid = cdll.LoadLibrary('libdavid.so')
    except ImportError:
        import sys
        print('Error: unable to load shared libraries')
        sys.exit(-1)
elif (platform.system() == 'Darwin'):
    try:
        libbasic = cdll.LoadLibrary('libbasic.dylib')
        libblockmatching = cdll.LoadLibrary('libblockmatching.dylib')
        libvt = cdll.LoadLibrary('libvt.dylib')
        libvp = cdll.LoadLibrary('libvp.dylib')
        libvtexec = cdll.LoadLibrary('libvtexec.dylib')
        libio = cdll.LoadLibrary('libio.dylib')
        libdavid = cdll.LoadLibrary('libdavid.dylib')
    except ImportError:
        import sys
        print('Error: unable to load shared libraries')
        sys.exit(-1)
else:
    supp_platforms = ['Linux', 'Darwin']
    print('Supported platforms: '), supp_platforms
    sys.exit(-1)

try:
    libc = cdll.LoadLibrary(find_library('c'))
except ImportError:
    import sys
    print('Error: unable to find libc')
    sys.exit(-1)

if (platform.system() == 'Linux'):
    c_stdout = c_void_p.in_dll(libc, "stdout")
elif (platform.system() == "Darwin"):
    c_stdout = c_void_p.in_dll(libc, '__stdoutp')
else:
    supp_platforms = ['Linux', 'Darwin']
    print('Supported platforms: '), supp_platforms
    sys.exit(-1)

libblockmatching.API_blockmatching.restype = POINTER(BAL_TRSF)


def return_value(values, rcode):
    if rcode == -1:
        return None

    # return result image
    return values


def add_doc(py_func, c_func):
    c_func.restype = c_char_p
    py_doc = py_func.__doc__
    doc = """
%s

parameter_str:

%s
""" % (py_doc, c_func(1))
    py_func.__doc__ = doc
