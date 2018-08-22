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
# ------------------------------------------------------------------------------

import unittest
import numpy as np

try:
    from timagetk.components import SpatialImage
except ImportError:
    raise ImportError('Unable to import SpatialImage')


class TestSpatialImage(unittest.TestCase):

    def test_get_methods(self):
        tmp_arr = np.ones((5, 5), dtype=np.uint8)
        vox = [1.0, 1.0]
        orig = [0, 0]
        img = SpatialImage(tmp_arr, origin=orig, voxelsize=vox)
        # metadata = {'dim': 2, 'extent': [5.0, 5.0], 'shape': (5, 5),
        #             'type': 'uint8',
        #             'voxelsize': [1.0, 1.0], 'origin': [0, 0], 'max': 1,
        #             'mean': 1.0,
        #             'min': 1}
        # 'min', 'max' & 'mean' are not computed by default anymore:
        metadata = {'dim': 2, 'extent': [5.0, 5.0], 'shape': (5, 5),
                    'type': 'uint8', 'voxelsize': [1.0, 1.0], 'origin': [0, 0]}

        self.assertEqual(img.get_type(), tmp_arr.dtype)
        self.assertEqual(img.get_voxelsize(), vox)
        self.assertEqual(img.get_shape(), tmp_arr.shape)
        self.assertEqual(img.get_origin(), orig)
        self.assertEqual(img.get_extent(), [tmp_arr.shape[0], tmp_arr.shape[1]])
        self.assertEqual(img.get_dim(), 2)
        self.assertDictEqual(img.get_metadata(), metadata)
        self.assertEqual(img.get_min(), 1)
        self.assertEqual(img.get_max(), 1)
        self.assertEqual(img.get_mean(), 1.0)

    def test_set_methods(self):
        tmp_arr = np.ones((5, 5), dtype=np.uint8)
        vox = [1.0, 1.0]
        orig = [0, 0]
        img = SpatialImage(tmp_arr, origin=orig, voxelsize=vox)
        new_vox = [0.5, 0.5]
        new_orig = [1, 1]
        new_ext = [tmp_arr.shape[0] * new_vox[0], tmp_arr.shape[1] * new_vox[1]]
        new_type = np.uint16
        new_met = img.get_metadata()
        new_met['name'] = 'img_test'

        img.set_voxelsize(new_vox)
        self.assertEqual(img.get_voxelsize(), new_vox)
        self.assertEqual(img.get_metadata()['voxelsize'], new_vox)
        self.assertEqual(img.get_extent(), new_ext)
        self.assertEqual(img.get_metadata()['extent'], new_ext)
        img = img.set_type(new_type)
        self.assertEqual(img.get_type(), 'uint16')
        self.assertEqual(img.get_metadata()['type'], 'uint16')
        img.set_origin(new_orig)
        self.assertEqual(img.get_origin(), new_orig)
        self.assertEqual(img.get_metadata()['origin'], new_orig)
        img.set_extent([10.0, 5.0])
        self.assertEqual(img.get_voxelsize(), [2.0, 1.0])
        self.assertEqual(img.get_metadata()['extent'], [10.0, 5.0])
        self.assertEqual(img.get_metadata()['voxelsize'], [2.0, 1.0])
        img.set_metadata(new_met)
        self.assertDictEqual(img.get_metadata(), new_met)
        img.set_pixel([2, 2], 10)
        self.assertEqual(img.get_pixel([2, 2]), 10)

        # --- numpy compatibility (for example transposition)
        new_arr = np.ones((10, 5), dtype=np.uint8)
        img = SpatialImage(new_arr, voxelsize=[0.5, 1])
        img = img.transpose()
        self.assertEqual(img.get_metadata()['shape'], (5, 10))
        self.assertEqual(img.get_metadata()['voxelsize'], [1.0, 0.5])
        self.assertEqual(img.get_metadata()['voxelsize'], img.get_voxelsize())
        self.assertEqual(img.get_metadata()['extent'], [5.0, 5.0])
        self.assertEqual(img.get_metadata()['extent'], img.get_extent())
