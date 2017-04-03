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
    from timagetk.algorithms import regionalext
    from timagetk.plugins import h_transform
except ImportError:
    raise ImportError('Import Error')


def test_regionalmax():
    im = imread(data_path('filtering_src.inr'))
    im_ref = imread(data_path('regionalmax_default.inr'))
    output = regionalext(im, param_str_2="-maxima -connectivity 26 -h 3")
    np.testing.assert_array_equal(output, im_ref)


def test_regionalmin():
    im = imread(data_path('filtering_src.inr'))
    im_ref = imread(data_path('regionalmin_default.inr'))
    output = regionalext(im, param_str_2="-minima -connectivity 26 -h 3")
    np.testing.assert_array_equal(output, im_ref)


def test_pluginmax():
    im = imread(data_path('filtering_src.inr'))
    im_ref = imread(data_path('regionalmax_default.inr'))
    output = h_transform(im, h=3, method='h_transform_max')
    np.testing.assert_array_equal(output, im_ref)


def test_pluginmin():
    im = imread(data_path('filtering_src.inr'))
    im_ref = imread(data_path('regionalmin_default.inr'))
    output = h_transform(im, h=3, method='h_transform_min')
    np.testing.assert_array_equal(output, im_ref)