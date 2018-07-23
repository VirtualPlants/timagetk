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
    from timagetk.plugins import linear_filtering
except ImportError as e:
    raise ImportError('Import Error: {}'.format(e))


class TestFiltering(unittest.TestCase):

    def test_linear_filter(self):
        """
        Testing 'linearfilter' algorithm.
        """
        # --- apply default linear filter
        sp_img_ref = imread(data_path('filtering_linearfilter_default.inr'))
        sp_img = imread(data_path('filtering_src.inr'))
        filtered = linearfilter(sp_img)
        np.testing.assert_array_equal(filtered, sp_img_ref)

    def test_gaussian_filtering(self):
        """
        Testing Gaussian filter algorithm.
        """
        sp_img_ref = imread(data_path('filtering_linearfilter_sigma_3.inr.gz'))
        sp_img = imread(data_path('filtering_src.inr'))
        # Equivalent to VT library command:
        # $ linearfilter filtering_src.inr.gz filtering_linearfilter_sigma_3.inr.gz -smoothing -sigma 3
        filtered = linearfilter(sp_img, param_str_2='-smoothing -sigma 3.0')
        np.testing.assert_array_equal(filtered, sp_img_ref)


    def test_gaussian_filtering_xyz(self):
        """
        Testing direction specific Gaussian filter algorithm.
        """
        sp_img_ref = imread(data_path('filtering_linearfilter_sigma332.inr.gz'))
        sp_img = imread(data_path('filtering_src.inr'))
        # Equivalent to VT library command:
        # $ linearfilter filtering_src.inr.gz filtering_linearfilter_sigma332.inr.gz -smoothing -sigma 3 3 2
        filtered = linearfilter(sp_img, param_str_2='-smoothing -sigma 3.0 3.0 2.0')
        np.testing.assert_array_equal(filtered, sp_img_ref)

    def test_plugin_1(self):
        sp_img_ref = imread(data_path('filtering_linearfilter_default.inr'))
        sp_img = imread(data_path('filtering_src.inr'))
        filtered = linear_filtering(sp_img, method='gaussian_smoothing')
        np.testing.assert_array_equal(filtered, sp_img_ref)

    def test_plugin_2(self):
        sp_img_ref = imread(data_path('filtering_linearfilter_sigma_3.inr.gz'))
        sp_img = imread(data_path('filtering_src.inr'))
        filtered = linear_filtering(sp_img, std_dev=3.0,
                                    method='gaussian_smoothing')
        np.testing.assert_array_equal(filtered, sp_img_ref)

    def test_plugin_3(self):
        sp_img_ref = imread(data_path('filtering_linearfilter_sigma_332.inr.gz'))
        sp_img = imread(data_path('filtering_src.inr'))
        filtered = linear_filtering(sp_img, std_dev=[3.0, 3.0, 2.0],
                                    method='gaussian_smoothing')
        np.testing.assert_array_equal(filtered, sp_img_ref)
