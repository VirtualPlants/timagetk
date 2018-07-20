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

import os
import unittest
import numpy as np
try:
    from timagetk.components import imread, imsave
    from timagetk.util import data_path
except ImportError:
    raise ImportError('Import Error')


class TestIO(unittest.TestCase):


    def test_read_tiff_3D(self):
        """
        Read an INR image, save it as TIFF, read it and make sure they are equal
        """
        # Load INR:
        sp_img_inr = imread(data_path('time_0_cut.inr'))
        # Write TIFF from SpatialImage:
        img_path_tiff = data_path('time_0_cut.tiff')
        imsave(img_path_tiff, sp_img_inr)
        # Load TIFF:
        sp_img_tiff = imread(img_path_tiff)
        self.assertDictEqual(sp_img_inr.get_metadata(), sp_img_tiff.get_metadata())
        np.testing.assert_array_equal(sp_img_inr, sp_img_tiff)


    def test_read_mha_3D(self):
        """
        Read an INR image, save it as MHA, read it and make sure they are equal
        """
        # Load INR:
        sp_img_inr = imread(data_path('time_0_cut.inr'))
        # Write MHA from SpatialImage:
        img_path_mha = data_path('time_0_cut.mha')
        imsave(img_path_mha, sp_img_inr)
        # Load MHA:
        sp_img_mha = imread(img_path_mha)
        # Dictionary are different since metadata are handled differently:
        # TODO: check those differences and create a better test
        # self.assertDictEqual(sp_img_inr.get_metadata(), sp_img_mha.get_metadata())
        np.testing.assert_array_equal(sp_img_inr, sp_img_mha)


    def test_save_3D(self):
        """
        Test read & write functions do not change the objects.
        """
        # Test read/write INR:
        sp_img_inr = imread(data_path('time_0_cut.inr'))
        img_path_inr = data_path('time_0_cut_cp.inr')
        imsave(img_path_inr, sp_img_inr)
        sp_img_inr_cp = imread(img_path_inr)
        self.assertDictEqual(sp_img_inr.get_metadata(), sp_img_inr_cp.get_metadata())
        np.testing.assert_array_equal(sp_img_inr, sp_img_inr_cp)
        # Test read/write TIFF:
        sp_img_tiff = imread(data_path('time_0_cut.tiff'))
        img_path_tiff = data_path('time_0_cut_cp.tiff')
        imsave(img_path_tiff, sp_img_tiff)
        sp_img_tiff_cp = imread(img_path_tiff)
        self.assertDictEqual(sp_img_tiff.get_metadata(), sp_img_tiff_cp.get_metadata())
        np.testing.assert_array_equal(sp_img_tiff, sp_img_tiff_cp)
        # Test read/write MHA:
        sp_img_mha = imread(data_path('time_0_cut.mha'))
        img_path_mha = data_path('time_0_cut_cp.mha')
        imsave(img_path_mha, sp_img_mha)
        sp_img_mha_cp = imread(img_path_mha)

        # Dictionary should be the same, except for 'filename':
        md = sp_img_mha.get_metadata()
        md_cp = sp_img_mha_cp.get_metadata()
        md.pop('filename')
        md_cp.pop('filename')
        self.assertDictEqual(md, md_cp)
        np.testing.assert_array_equal(sp_img_mha, sp_img_mha_cp)

        # Remove TIFF and MHA files:
        os.remove(data_path('time_0_cut.tiff'))
        os.remove(data_path('time_0_cut.mha'))
        # Remove "copy" files:
        os.remove(img_path_inr)
        os.remove(img_path_tiff)
        os.remove(img_path_mha)
