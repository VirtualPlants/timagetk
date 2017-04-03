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

from ctypes import Structure, c_int, c_double, POINTER
__all__ = ['BAL_MATRIX']


class BAL_MATRIX(Structure):
    """
    Definition of BAL_MATRIX structure derived from the Structure class which is defined in the ctypes module
    """
    _fields_ = [("l", c_int),       # Number of row
                ("c", c_int),       # Number of column
                ("m", POINTER(c_double))  # Array
                ]