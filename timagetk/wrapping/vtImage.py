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

#--- see vt_image.h
from ctypes import Structure, c_char, c_uint, c_void_p, c_char_p
try:
    from timagetk.wrapping.vt_typedefs import vt_4vsize, vt_fpt, vt_ipt, CpuType
except ImportError:
    raise ImportError('Import Error')

__all__ = ['_VT_IMAGE']

STRINGLENGTH = 256
MY_CPU = CpuType.LITTLEENDIAN


class _VT_IMAGE(Structure):
    """ vt_image structure, see : vt_image.h """
    _fields_ = [("name", c_char * STRINGLENGTH),
                ("type", c_uint),
                ("dim", vt_4vsize),  # dimension of image
                ("siz", vt_fpt),        # voxel size
                ("off", vt_fpt),        # translation or offset
                ("rot", vt_fpt),        # rotation
                ("ctr", vt_ipt),
                ("cpu", c_uint),
                ("array", c_void_p),
                ("buf", c_void_p),
                # User defined strings array. The user can use any internal
                # purpose string.
                ("user", c_char_p),
                # Each string is written at then end of
                # header after a '#' character.
                ("nuser", c_uint)       # Number of user defined strings
                ]