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

#--- Feb. 2016
import unittest
import numpy as np
try:
    from timagetk.util import data_path
    from timagetk.wrapping.bal_trsf import BalTransformation
    from timagetk.components import imread
    from timagetk.algorithms import create_trsf, blockmatching
    from timagetk.plugins import registration
except ImportError:
    raise ImportError('Import Error')

REAL_DATA = True

class TestBlockmatching(unittest.TestCase):


    def test_blockmatching_basic(self):
        img_src = imread(data_path('filtering_src.inr'))
        trsf_out, res = blockmatching(img_src, img_src)
        identity_trsf = create_trsf(param_str_2='-identity')
        np.testing.assert_array_almost_equal(trsf_out.mat.to_np_array(),
                                             identity_trsf.mat.to_np_array(),
                                             decimal=8)
        np.testing.assert_array_almost_equal(res, img_src, decimal=8)
        identity_trsf.free()
        trsf_out.free()
        return

    if REAL_DATA is True:

        def test_blockmatching_rigid(self):
            #--- rigid registration
            floating_img = imread(data_path('time_0_cut.inr'))
            reference_img = imread(data_path('time_1_cut.inr'))
            rigid_trsf = data_path("rigid_0_1.trsf")

            trsf_out, res = blockmatching(floating_img, reference_img)
            trsf_inp = BalTransformation()
            trsf_inp.read(rigid_trsf)
            np.testing.assert_array_almost_equal(trsf_out.mat.to_np_array(), trsf_inp.mat.to_np_array(), decimal=6)
            trsf_out.free()
            trsf_inp.free()
            return

        def test_plugin_rigid(self):
            #--- rigid registration
            floating_img = imread(data_path('time_0_cut.inr'))
            reference_img = imread(data_path('time_1_cut.inr'))
            rigid_trsf = data_path("rigid_0_1.trsf")

            trsf_out, res = registration(floating_img, reference_img, method='rigid_registration')
            trsf_inp = BalTransformation()
            trsf_inp.read(rigid_trsf)
            np.testing.assert_array_almost_equal(trsf_out.mat.to_np_array(), trsf_inp.mat.to_np_array(), decimal=6)
            trsf_out.free()
            trsf_inp.free()
            return

        def test_blockmatching_deformable(self):
            #--- deformable registration
            floating_img = imread(data_path('time_0_cut.inr'))
            reference_img = imread(data_path('time_1_cut.inr'))
            rigid_trsf = data_path("rigid_0_1.trsf")
            deformable_trsf = data_path("deformable_0_1.trsf")
            init_result_transformation = BalTransformation()
            init_result_transformation.read(rigid_trsf)
            param_str_2 = '-trsf-type vectorfield'
            trsf_out, res = blockmatching(floating_img, reference_img,
                                          init_result_transformation=init_result_transformation,
                                          left_transformation=None,
                                          param_str_2=param_str_2)
            trsf_inp = BalTransformation()
            trsf_inp.read(deformable_trsf)
            np.testing.assert_array_almost_equal(trsf_out.mat.to_np_array(), trsf_inp.mat.to_np_array(), decimal=6)
            trsf_out.free()
            trsf_inp.free()
            return

        def test_plugin_deformable(self):
            #--- deformable registration
            floating_img = imread(data_path('time_0_cut.inr'))
            reference_img = imread(data_path('time_1_cut.inr'))
            deformable_trsf = data_path("deformable_0_1.trsf")
            trsf_inp = BalTransformation()
            trsf_inp.read(deformable_trsf)
            trsf_out, res = registration(floating_img, reference_img, method='deformable_registration')
            np.testing.assert_array_almost_equal(trsf_out.mat.to_np_array(), trsf_inp.mat.to_np_array(), decimal=6)
            trsf_out.free()
            trsf_inp.free()
            return