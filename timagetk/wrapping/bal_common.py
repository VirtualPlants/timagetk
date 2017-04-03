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

from ctypes import pointer
__all__ = ['bal_mystr_c_ptr',
           'bal_mystr_c_struct'
           ]


def bal_mystr_c_ptr(c_or_bal_mystr, BAL_MYSTR, BalMyStr):
    """
    Return a pointer to instance of BAL_MYSTR or BalMyStr.
    BAL_MYSTR can be either : BAL_IMAGE, BAL_MATRIX
    BalMyStr can be either : BalImage, BalMatrix
    """
    if c_or_bal_mystr is None:
        return
    elif isinstance(c_or_bal_mystr, BAL_MYSTR):
        return pointer(c_or_bal_mystr)
    elif isinstance(c_or_bal_mystr, BalMyStr):
        return c_or_bal_mystr.c_ptr
    else:
        return NotImplementedError


def bal_mystr_c_struct(c_or_bal_mystr, BAL_MYSTR, BalMyStr):
    """
    Return an instance of BAL_MYSTR or BalMyStr.
    BAL_MYSTR can be either : BAL_IMAGE, BAL_MATRIX
    BalMyStr can be either : BalImage, BalMatrix
    """
    if c_or_bal_mystr is None:
        return
    elif isinstance(c_or_bal_mystr, BAL_MYSTR):
        return c_or_bal_mystr
    elif isinstance(c_or_bal_mystr, BalMyStr):
        return c_or_bal_mystr.c_struct
    else:
        return NotImplementedError
