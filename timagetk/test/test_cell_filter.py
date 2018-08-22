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
    from timagetk.algorithms import cell_filter
except ImportError as e:
    raise ImportError('Import Error: {}'.format(e))


class TestCellFilter(unittest.TestCase):

    def test_erosion(self):
        sp_img_ref = imread(data_path('eroded_segmentation.inr'))
        sp_img = imread(data_path('segmentation_seeded_watershed.inr'))
        output = cell_filter(sp_img, param_str_2='-erosion -R 1')
        np.testing.assert_array_equal(output, sp_img_ref)
