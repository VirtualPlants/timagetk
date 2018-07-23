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
import numpy as np
try:
    from timagetk.wrapping.clib import libblockmatching, c_stdout
    from timagetk.wrapping.bal_common import bal_mystr_c_ptr, bal_mystr_c_struct
    from timagetk.wrapping.balMatrix import BAL_MATRIX
except ImportError as e:
    raise ImportError('Import Error: {}'.format(e))

__all__ = ['bal_matrix_c_ptr',
           'bal_matrix_c_struct',
           'free_bal_matrix',
           'np_array_to_bal_matrix_fields',
           'init_c_bal_matrix',
           'allocate_c_bal_matrix',
           'new_c_bal_matrix',
           'bal_matrix_to_np_array',
           'np_array_to_bal_matrix',
           'BalMatrix'
           ]


def bal_matrix_c_ptr(c_or_bal_matrix):
    """
    Return a pointer to instance of BAL_MATRIX.

    Parameters
    ----------
    c_or_bal_matrix : instance of BAL_MATRIX

    Returns
    ----------
    output : c_ptr, pointer

    Example
    -------
    c_ptr = bal_matrix_c_ptr(c_or_bal_matrix)
    """
    return bal_mystr_c_ptr(c_or_bal_matrix, BAL_MATRIX, BalMatrix)


def bal_matrix_c_struct(c_or_bal_matrix):
    """
    Return an instance of BAL_MATRIX
    """
    return bal_mystr_c_struct(c_or_bal_matrix, BAL_MATRIX, BalMatrix)


def free_bal_matrix(c_or_bal_matrix):
    """
    Memory deallocation.

    Parameters
    ----------
    c_or_bal_matrix : instance of BAL_MATRIX

    Example
    -------
    free_bal_matrix(c_or_bal_matrix)
    """
    c_ptr = bal_matrix_c_ptr(c_or_bal_matrix)
    if c_ptr:
        libblockmatching._free_mat(c_ptr)  # free structure
        del c_or_bal_matrix


def np_array_to_bal_matrix_fields(np_array):
    """
    BAL_MATRIX fields from numpy array

    Parameters
    ----------
    np_array : numpy array

    Returns
    ----------
    output : BAL_MATRIX fields, type dict

    Example
    -------
    kwargs = np_array_to_bal_matrix_fields(np_array)
    """
    l, c = np_array.shape
    kwargs = dict(c=c, l=l)
    return kwargs


def init_c_bal_matrix(c_bal_matrix, **kwargs):
    """
    Initialization of a c_bal_matrix.

    Parameters
    ----------
    c_bal_matrix : c_bal_matrix
    kwargs: type dict
        - l: number of lines
        - c: number of columns

    Example
    -------
    init_c_bal_matrix(c_bal_matrix, kwargs)
    """
    c_bal_matrix.l, c_bal_matrix.c = kwargs.get('l'), kwargs.get('c')


def allocate_c_bal_matrix(c_bal_matrix, np_array, **kwargs):
    """
    Memory allocation of a c_bal_matrix.

    Parameters
    ----------
    c_bal_matrix : c_bal_matrix
    np_array : numpy array

    Example
    -------
    allocate_c_bal_matrix(c_bal_matrix, np_array)
    """

    l, c = c_bal_matrix.l, c_bal_matrix.c
    libblockmatching._alloc_mat(pointer(c_bal_matrix), l, c)

    # Fill matrix
    for i in range(l):
        for j in range(c):
            c_bal_matrix.m[j + i * c] = np_array[i, j]


def new_c_bal_matrix(**kwargs):
    """
    Create an instance of BAL_MATRIX and fill it with kwargs.
    """
    c_bal_matrix = BAL_MATRIX()
    init_c_bal_matrix(c_bal_matrix, **kwargs)
    return c_bal_matrix


def bal_matrix_to_np_array(c_or_bal_matrix, **kwargs):
    """
    From BAL_MATRIX to numpy array.

    Parameters
    ----------
    c_or_bal_matrix : c_or_bal_matrix
    """
    m = bal_matrix_c_struct(c_or_bal_matrix)
    l = m.l
    c = m.c
    np_array = np.zeros((l, c), dtype=np.float64)
    # Fill matrix
    for i in range(l):
        for j in range(c):
            np_array[i, j] = m.m[j + i * c]
    return np_array


def np_array_to_bal_matrix(np_array, **kwargs):
    """
    From numpy array to c_bal_matrix

    """
    np_array_kwargs = np_array_to_bal_matrix_fields(np_array)

    _kwargs = {}
    _kwargs.update(kwargs)
    _kwargs.update(np_array_kwargs)

    c_bal_matrix = new_c_bal_matrix(**_kwargs)
    allocate_c_bal_matrix(c_bal_matrix, np_array)

    return c_bal_matrix


class BalMatrix(object):

    def __init__(self, np_array=None, c_bal_matrix=None, **kwargs):

        if np_array is not None:
            self._np_array = np_array
            self._c_bal_matrix = np_array_to_bal_matrix(self._np_array, **kwargs)
        elif c_bal_matrix is not None:
            self._c_bal_matrix = c_bal_matrix
            self._np_array = self.to_np_array()
        else:
            self._c_bal_matrix = new_c_bal_matrix(**kwargs)
            self._np_array = self.to_np_array()

    @property
    def c_ptr(self):
        return pointer(self._c_bal_matrix)

    @property
    def c_struct(self):
        return self._c_bal_matrix

    def to_np_array(self, **kwargs):
        return bal_matrix_to_np_array(self._c_bal_matrix, **kwargs)

    def free(self):
        if self._c_bal_matrix:
            free_bal_matrix(self._c_bal_matrix)
        self._c_bal_matrix = None

    def c_display(self, name=''):
        libblockmatching._print_mat(c_stdout, self.c_ptr, name)

    def __del__(self):
        self.free()

    def __eq__(self, other):
        if isinstance(other, self.__class__):
            other = other.to_np_array()
        elif isinstance(other, np.ndarray):
            pass
        else:
            return False
        return np.array_equal(self.to_np_array(), other)

    def __ne__(self, other):
        return not self.__eq__(other)


"""
/* substract two matrix */
BAL_MATRIX sub_mat(BAL_MATRIX m1, BAL_MATRIX m2);

/* transpose matrix */
BAL_MATRIX transpose(BAL_MATRIX m);

/* det matrix */
double det(BAL_MATRIX mat);

/* inverse matrix */
BAL_MATRIX inv_mat(BAL_MATRIX m);
"""


def c_bal_matrix_transpose(c_bal_matrix):
    libblockmatching.transpose.argtypes = [BAL_MATRIX]
    libblockmatching.transpose.restype = BAL_MATRIX
    c_bal_matrix_out = libblockmatching.transpose(c_bal_matrix)
    return c_bal_matrix_out


def np_array_transpose(np_array):
    c_bal_matrix = np_array_to_bal_matrix(np_array)
    c_bal_matrix_out = c_bal_matrix_transpose(c_bal_matrix)
    return bal_matrix_to_np_array(c_bal_matrix_out)
