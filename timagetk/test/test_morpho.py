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
    from timagetk.algorithms import morpho
    from timagetk.plugins import morphology
except ImportError:
    raise ImportError('Import Error')


def test_dilation():
    #--- dilation
    im = imread(data_path('filtering_src.inr'))
    im_ref = imread(data_path('morpho_dilation_default.inr'))
    output = morpho(im, param_str_2='-dilation')
    np.testing.assert_array_equal(output, im_ref)


def test_erosion():
    #--- erosion
    im = imread(data_path('filtering_src.inr'))
    im_ref = imread(data_path('morpho_erosion_default.inr'))
    output = morpho(im, param_str_2='-erosion')
    np.testing.assert_array_equal(output, im_ref)


def test_plugin_1():
    im = imread(data_path('filtering_src.inr'))
    im_ref = imread(data_path('morpho_dilation_default.inr'))
    output = morphology(im, method='dilation')
    np.testing.assert_array_equal(output, im_ref)


def test_plugin_2():
    im = imread(data_path('filtering_src.inr'))
    im_ref = imread(data_path('morpho_erosion_default.inr'))
    output = morphology(im, method='erosion')
    np.testing.assert_array_equal(output, im_ref)