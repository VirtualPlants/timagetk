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

#--- see vt_typedefs.h
from ctypes import Structure, c_int, c_size_t, c_float


class DimType:
    """ DimType enumeration """
    VT_1D, OneD = 1, 1
    VT_2D, TwoD = 2, 2
    VT_3D, ThreeD = 3, 3
    VT_4D = 4


class CpuType:
    """ Definition of CpuType enumeration """
    CPU_UNKNOWN = 0
    LITTLEENDIAN = 1
    BIGENDIAN = 2


class ImageType:
    """ ImageType enumeration """
    TYPE_UNKNOWN = 0  # unknown type
    UCHAR, SCHAR = 1, 2  # unsigned, signed char
    USHORT, SSHORT = 3, 4  # unsigned, signed short int
    UINT, SINT = 5, 6  # unsigned, signed int
    ULINT, SLINT = 7, 8  # unsigned, signed long int
    FLOAT, DOUBLE = 9, 10  # float, double


class BufferType:
    """ BufferType enumeration """
    TYPE_UNKNOWN = 0    # unknown type
    UCHAR, SCHAR = 1, 2  # unsigned, signed char
    USHORT, SSHORT = 3, 4  # unsigned, signed short int
    UINT, SINT = 5, 6  # unsigned, signed int
    ULINT, SLINT = 7, 8  # unsigned, signed long int
    FLOAT, DOUBLE = 9, 10  # float, double


class typeBoolean:
    """ TypeBoolean enumeration """
    False, True = 0, 1


class vt_ipt(Structure):
    """ vt_ipt structure: point with 3 intergers (x, y, z) """
    _fields_ = [("x", c_int), ("y", c_int), ("z", c_int)]


class vt_4ipt(Structure):
    """ vt_4ipt structure: point with 4 integers (x,y,z,i) """
    _fields_ = [("x", c_int), ("y", c_int), ("z", c_int), ("i", c_int)]


class vt_4tpt(Structure):
    """ vt_4tpt structure: point with 4 integers (x,y,z,t) """
    _fields_ = [("x", c_int), ("y", c_int), ("z", c_int), ("t", c_int)]


class vt_4vpt(Structure):
    """ vt_4vpt structure: point with 4 integers (v,x,y,z) """
    _fields_ = [("v", c_int), ("x", c_int), ("y", c_int), ("z", c_int)]


class vt_4vsize(Structure):
    """ vt_4size structure: point with 4 size_t (v,x,y,z) """
    _fields_ = [("v", c_size_t), ("x", c_size_t),
                ("y", c_size_t), ("z", c_size_t)]


class vt_fpt(Structure):
    """ vt_fpt structure: point with 3 floats (x,y,z) """
    _fields_ = [("x", c_float), ("y", c_float), ("z", c_float)]


class vt_4fpt(Structure):
    """ vt_4fpt structure: point with 4 floats (x,y,z,i) """
    _fields_ = [("x", c_float), ("y", c_float), ("z", c_float), ("i", c_float)]


#STRINGLENGTH = 256
#
#s8 = c_char
#u8 = c_ubyte
#s16 = c_short
#u16 = c_ushort
#i32 = c_int
#s32 = c_int
#u32 = c_uint
#s64 = c_long
#u64 = c_ulong
#r32 = c_float
#r64 = c_double
