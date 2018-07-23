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
    from timagetk.algorithms import linearfilter
    from timagetk.algorithms import regionalext
    from timagetk.algorithms import connexe
    from timagetk.algorithms import watershed
    from timagetk.plugins import linear_filtering
    from timagetk.plugins import h_transform
    from timagetk.plugins import region_labeling
    from timagetk.plugins import segmentation
except ImportError as e:
    raise ImportError('Import Error: {}'.format(e))


class TestSegmentation(unittest.TestCase):

    def test_segmentation(self):
        im = imread(data_path('segmentation_src.inr'))
        im_ref = imread(data_path('segmentation_seeded_watershed.inr'))
        # - Smoothing:
        smooth_params = "-smoothing -sigma 2.0"
        smooth_img = linearfilter(im, param_str_2=smooth_params)
        # - Local minima:
        regext_params = "-minima -connectivity 26 -h 5"
        regext_img = regionalext(smooth_img, param_str_2=regext_params)
        # - Connexe components labelling:
        conn_params = "-low-threshold 1 -high-threshold 3 -labels -connectivity 26"
        conn_img = connexe(regext_img, param_str_2=conn_params)
        # - Watershed:
        wat_img = watershed(smooth_img, conn_img)
        # Uncomment clean (rm -vf ...) in gen_image.sh script and uncomment next lines to test step results
        # np.testing.assert_array_equal(smooth_img, imread('segmentation_smooth.inr'))
        # np.testing.assert_array_equal(regext_img, imread('segmentation_regext.inr'))
        # np.testing.assert_array_equal(conn_img, imread('segmentation_connexe.inr'))
        np.testing.assert_array_equal(wat_img, im_ref)

    def test_plugin(self):
        im = imread(data_path('segmentation_src.inr'))
        im_ref = imread(data_path('segmentation_seeded_watershed.inr'))
        # - Smoothing:
        smooth_img = linear_filtering(im, std_dev=2.0,
                                      method='gaussian_smoothing')
        # - Local minima:
        regext_img = h_transform(smooth_img, h=5, method='h_transform_min')
        # - Connexe components labelling:
        conn_img = region_labeling(regext_img, low_threshold=1,
                                   high_threshold=3,
                                   method='connected_components')
        # - Watershed:
        wat_img = segmentation(smooth_img, conn_img, control='first',
                               method='seeded_watershed')
        np.testing.assert_array_equal(wat_img, im_ref)
