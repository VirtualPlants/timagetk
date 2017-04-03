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

import unittest
import numpy as np
try:
    from timagetk.components import imread, imsave
    from timagetk.util import data_path
except ImportError:
    raise ImportError('Import Error')


class TestIO(unittest.TestCase):


    def test_read_3D(self):
        sp_img_inr = imread(data_path('time_0_cut.inr'))
        img_path_tiff = data_path('time_0_cut.tiff')
        imsave(img_path_tiff, sp_img_inr)
        sp_img_tiff = imread(img_path_tiff)
        self.assertDictEqual(sp_img_inr.get_metadata(), sp_img_tiff.get_metadata())
        np.testing.assert_array_equal(sp_img_inr, sp_img_tiff)


    def test_save_3D(self):
        sp_img_inr = imread(data_path('time_0_cut.inr'))
        img_path_inr = data_path('time_0_cut_cp.inr')
        imsave(img_path_inr, sp_img_inr)
        sp_img_inr_cp = imread(img_path_inr)
        self.assertDictEqual(sp_img_inr.get_metadata(), sp_img_inr_cp.get_metadata())
        sp_img_tiff = imread(data_path('time_0_cut.tiff'))
        img_path_tiff = data_path('time_0_cut_cp.tiff')
        imsave(img_path_tiff, sp_img_tiff)
        sp_img_tiff_cp = imread(img_path_tiff)
        self.assertDictEqual(sp_img_tiff.get_metadata(), sp_img_tiff_cp.get_metadata())
        np.testing.assert_array_equal(sp_img_tiff, sp_img_tiff_cp)