# -*- coding: utf-8 -*-
# -*- python -*-
#
#
#       Copyright 2016 INRIA
#
#       File author(s):
#           Guillaume Baty <guillaume.baty@inria.fr>
#           Sophie Ribes <sophie.ribes@inria.fr>
#           Jonathan Legrand <jonathan.legrand@ens-lyon.fr>
#
#       See accompanying file LICENSE.txt
#------------------------------------------------------------------------------

import unittest
import numpy as np
try:
    from timagetk.wrapping.bal_matrix import BalMatrix, np_array_transpose
except ImportError as e:
    raise ImportError('Import Error: {}'.format(e))

r = np.arange(12)
mat_ref = np.array(np.reshape(r, (3, 4)), np.float64)

class TestBalMatrix(unittest.TestCase):

    def test_matrix_ptr_conversion(self):

        bal_matrix = BalMatrix(mat_ref)
        new_np = bal_matrix.to_np_array()
        np.testing.assert_array_equal(mat_ref, new_np)


    def test_matrix_equality(self):
        mat_2 = np.array(np.reshape(r, (3, 4)), np.float64)
        mat_2[0] = 10

        bal_matrix_1 = BalMatrix(mat_ref)
        bal_matrix_2 = BalMatrix(mat_ref)
        bal_matrix_3 = BalMatrix(mat_2)
        assert bal_matrix_1 == bal_matrix_2
        assert bal_matrix_1 != bal_matrix_3


    def test_matrix_conversion(self):
        np.testing.assert_array_equal(mat_ref.transpose(), np_array_transpose(mat_ref))


    def test_print_matrix(self):
        bal_matrix = BalMatrix(mat_ref)
        bal_matrix.c_display()
