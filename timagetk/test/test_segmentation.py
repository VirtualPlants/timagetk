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

import numpy as np
try:
    from timagetk.util import data_path
    from timagetk.components import imread
    from timagetk.algorithms import linearfilter, regionalext, connexe, watershed
    from timagetk.plugins import linear_filtering, h_transform, region_labeling, segmentation
except ImportError:
    raise ImportError('Import Error')


def test_segmentation():
    im = imread(data_path('segmentation_src.inr'))
    im_ref = imread(data_path('segmentation_seeded_watershed.inr'))
    smooth_img = linearfilter(im, param_str_2="-smoothing -sigma 2.0")
    regext_img = regionalext(smooth_img, param_str_2="-minima -connectivity 26 -h 5")
    conn_img = connexe(regext_img, param_str_2="-low-threshold 1 -high-threshold 3 -labels -connectivity 26")
    wat_img = watershed(smooth_img, conn_img)
    # Uncomment clean (rm -vf ...) in gen_image.sh script and uncomment next lines to test step results
    #np.testing.assert_array_equal(smooth_img, data('segmentation_smooth.inr'))
    #np.testing.assert_array_equal(regext_img, data('segmentation_regext.inr'))
    #np.testing.assert_array_equal(conn_img, data('segmentation_connexe.inr'))
    np.testing.assert_array_equal(wat_img, im_ref)

def test_plugin():
    im = imread(data_path('segmentation_src.inr'))
    im_ref = imread(data_path('segmentation_seeded_watershed.inr'))
    smooth_img = linear_filtering(im, std_dev=2.0, method='gaussian_smoothing')
    regext_img = h_transform(smooth_img, h=5, method='h_transform_min')
    conn_img = region_labeling(regext_img, low_threshold=1, high_threshold=3, method='connected_components')
    wat_img = segmentation(smooth_img, conn_img, control='first', method='seeded_watershed')
    np.testing.assert_array_equal(wat_img, im_ref)