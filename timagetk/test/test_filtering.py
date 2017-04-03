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
    from timagetk.algorithms import linearfilter
    from timagetk.plugins import linear_filtering
except ImportError:
    raise ImportError('Import Error')


def test_linear_filter():
    #--- apply default linear filter
    sp_img_ref = imread(data_path('filtering_linearfilter_default.inr'))
    sp_img = imread(data_path('filtering_src.inr'))
    filtered = linearfilter(sp_img)
    np.testing.assert_array_equal(filtered, sp_img_ref)


def test_gaussian_filtering():
    """
    Gaussian filter :
    $ linearfilter filtering_src.inr.gz filtering_linearfilter_sigma.inr.gz -smoothing -sigma 3 -o 1
    """
    #--- apply linear filter (sigma==3)
    sp_img_ref = imread(data_path('filtering_linearfilter_sigma_3.inr'))
    sp_img = imread(data_path('filtering_src.inr'))
    filtered = linearfilter(sp_img, '-smoothing -sigma 3.0')
    np.testing.assert_array_equal(filtered, sp_img_ref)


def test_plugin_1():
    sp_img_ref = imread(data_path('filtering_linearfilter_default.inr'))
    sp_img = imread(data_path('filtering_src.inr'))
    filtered = linear_filtering(sp_img, method='gaussian_smoothing')
    np.testing.assert_array_equal(filtered, sp_img_ref)


def test_plugin_2():
    sp_img_ref = imread(data_path('filtering_linearfilter_sigma_3.inr'))
    sp_img = imread(data_path('filtering_src.inr'))
    filtered = linear_filtering(sp_img, std_dev=3.0, method='gaussian_smoothing')
    np.testing.assert_array_equal(filtered, sp_img_ref)