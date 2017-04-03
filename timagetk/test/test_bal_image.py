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
    from timagetk.components import SpatialImage, imread
    from timagetk.util import data_path
    from timagetk.wrapping.bal_image import BalImage, spatial_image_to_bal_image
except ImportError:
    raise ImportError('Import Error')

image = imread(data_path('filtering_src.inr'))

class TestBalImage(unittest.TestCase):


    def check_image(self, sp_image_ref, bal_image):
        """
        sp_image_ref: SpatialImage instance
        bal_image: bal_image instance
        """
        sp_image = bal_image.to_spatial_image()
        np.testing.assert_array_almost_equal(sp_image_ref, sp_image, decimal=6)
        ix, iy, iz = sp_image_ref.get_voxelsize()
        nx, ny, nz = sp_image.get_voxelsize()
        np.testing.assert_almost_equal(ix, nx, decimal=6)
        np.testing.assert_almost_equal(iy, ny, decimal=6)
        np.testing.assert_almost_equal(iz, nz, decimal=6)


    def test_equality(self):
        shape, dtype = (2, 3, 4), np.uint16
        sp_ref = SpatialImage(np.zeros(shape, dtype))
        bal_ref = BalImage(sp_ref)
        # Same matrix
        bal_image_1 = BalImage(sp_ref)
        assert bal_ref == bal_image_1
        assert (bal_ref != bal_image_1) is False
        # Different shape
        zero = SpatialImage(np.zeros((4, 3, 2), dtype))
        bal_image_2 = BalImage(zero)
        assert bal_ref != bal_image_2
        # Different values
        one = SpatialImage(np.ones(shape, dtype))
        bal_image_3 = BalImage(one)
        assert bal_ref != bal_image_3
        # Different resolution
        zero = SpatialImage(np.zeros(shape, dtype), voxelsize=[0.5, 0.5, 0.5])
        bal_image_4 = BalImage(zero)
        assert bal_ref != bal_image_4
        # Different dtype
        zero_uint8 = SpatialImage(np.zeros(shape, dtype=np.uint8))
        bal_image_5 = BalImage(zero_uint8)
        assert bal_ref.to_spatial_image().dtype == np.uint16
        assert bal_image_5.to_spatial_image().dtype == np.uint8
        assert bal_ref != bal_image_5


    def test_build_from_spatial_image(self):
        bal_image = BalImage(image)
        bal_image.c_display()
        self.check_image(image, bal_image)


    def test_build_from_c_bal_image(self):
        c_bal_image = spatial_image_to_bal_image(image)
        bal_image = BalImage(c_bal_image=c_bal_image)
        bal_image.c_display()
        self.check_image(image, bal_image)


    def test_build_from_empty(self):
        shape = (2, 3, 4)
        bal_image = BalImage(shape=shape)
        bal_image.c_display()
        ref = SpatialImage(np.zeros((shape),dtype=np.uint8))
        self.check_image(ref, bal_image)