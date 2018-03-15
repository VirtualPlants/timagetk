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

import numpy as np
from ctypes import c_ubyte, c_byte, c_short, c_ushort, c_uint, c_int, c_ulong, c_long, c_float, c_double
try:
    from timagetk.wrapping.vt_typedefs import ImageType
except ImportError:
    raise ImportError('Import Error')

__all__ = ['np_type_to_vt_type',
           'vt_type_to_np_type',
           'vt_type_to_c_type',
           'vt_type',
           'np_type',
           'morpheme_dtype_arg']


def np_type_to_vt_type(dtype):
    """
    used to get information from numpy array
    """
    if dtype == np.uint8:
        _type = ImageType.UCHAR
    elif dtype == np.int8:
        _type = ImageType.SCHAR
    elif dtype == np.int16:
        _type = ImageType.SSHORT
    elif dtype == np.uint16:
        _type = ImageType.USHORT
    elif dtype == np.uint32:
        _type = ImageType.UINT
    elif dtype == np.int32:
        _type = ImageType.SINT
    elif dtype == np.uint64:
        _type = ImageType.ULINT
    elif dtype == np.int64:
        _type = ImageType.SLINT
    elif dtype == np.float32:
        _type = ImageType.FLOAT
    elif dtype == np.float64:
        _type = ImageType.DOUBLE
#     elif dtype == np.float128 :
#         _type = ImageType.LONG_DO
    else:
        _type = ImageType.TYPE_UNKNOWN
    return _type


def vt_type_to_np_type(vt_type):
    if vt_type == ImageType.UCHAR:
        return np.uint8
    elif vt_type == ImageType.SCHAR:
        return np.int8
    elif vt_type == ImageType.SSHORT:
        return np.int16
    elif vt_type == ImageType.USHORT:
        return np.uint16
    elif vt_type == ImageType.UINT:
        return np.uint32
    elif vt_type == ImageType.SINT:
        return np.int32
    elif vt_type == ImageType.ULINT:
        return np.uint64
    elif vt_type == ImageType.SLINT:
        return np.int64
    elif vt_type == ImageType.FLOAT:
        return np.float32
    elif vt_type == ImageType.DOUBLE:
        return np.float64
    # elif vt_type == VOXELTYPE. :
    #    self._data.dtype == np.float128
    else:
        print "TYPE_UNKNOWN"
        return -1


def vt_type_to_c_type(vt_type):
    """
    used to get information from numpy array
    """
    if vt_type == ImageType.UCHAR:
        return c_ubyte
    elif vt_type == ImageType.SCHAR:
        return c_byte
    elif vt_type == ImageType.SSHORT:
        return c_short
    elif vt_type == ImageType.USHORT:
        return c_ushort
    elif vt_type == ImageType.UINT:
        return c_uint
    elif vt_type == ImageType.SINT:
        return c_int
    elif vt_type == ImageType.ULINT:
        # return c_int
        return c_ulong
    elif vt_type == ImageType.SLINT:
        # return c_int
        return c_long
    elif vt_type == ImageType.FLOAT:
        return c_float
    elif vt_type == ImageType.DOUBLE:
        return c_double
    # elif vt_type == VOXELTYPE. :
    #    self._data.dtype == np.float128
    else:
        print "TYPE_UNKNOWN"
        return -1


VT_TO_ARG = {
    ImageType.SCHAR: "-o 1 -s",
    ImageType.UCHAR: "-o 1",
    ImageType.SSHORT: "-o 2 -s",
    ImageType.USHORT: "-o 2",
    ImageType.SINT: "-o 4 -s",
    ImageType.ULINT: "-o 4",
    ImageType.FLOAT: "-r",
}


def vt_type(np_type=None, vt_type=None):
    if np_type:
        return np_type_to_vt_type(np_type)
    elif vt_type:
        return vt_type
    else:
        return None


def np_type(np_type=None, vt_type=None):
    if np_type:
        return np_type
    elif vt_type:
        return vt_type_to_np_type(vt_type)
    else:
        return None


def morpheme_dtype_arg(image):
    vt_type = np_type_to_vt_type(image.dtype)
    if vt_type in VT_TO_ARG:
        return VT_TO_ARG[vt_type]
