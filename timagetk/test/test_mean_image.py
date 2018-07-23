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
    from timagetk.util import data_path
    from timagetk.components import imread
    from timagetk.algorithms import mean_images
    from timagetk.plugins import averaging
except ImportError as e:
    raise ImportError('Import Error: {}'.format(e))

image_1 = imread(data_path('filtering_src.inr'))
image_2 = imread(data_path('transformed_img.inr'))
mean_image = imread(data_path('mean_image.inr'))


class TestMeanImage(unittest.TestCase):

    def test_mean_image(self):
        out = mean_images([image_1, image_2])
        np.testing.assert_array_equal(out, mean_image)

    def test_plugin(self):
        out_1 = averaging([image_1, image_2], method='mean_averaging')
        out_2 = mean_images([image_1, image_2])
        np.testing.assert_array_equal(out_1, out_2)
