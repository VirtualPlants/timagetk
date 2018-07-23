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

# WARNING : currently, it seems that tests cannot be run in parallel.
# Need to check and fix it!


import unittest
import numpy as np

try:
    from timagetk.util import data_path
    from timagetk.components import imread
    from timagetk.algorithms import compose_trsf, inv_trsf, apply_trsf, \
        create_trsf
    from timagetk.wrapping.bal_trsf import BalTransformation
except ImportError as e:
    raise ImportError('Import Error: {}'.format(e))

# --- Images
img_src = imread(data_path("filtering_src.inr"))
arbitrary_transformed_ref = imread(data_path("transformed_img.inr"))
deformable_transformed_ref = imread(data_path("deformed_img.inr"))
# --- Transformations
trsf_arbitrary_path = data_path("arbitrary_trsf.trsf")
trsf_deformable_path = data_path("deformable_trsf.trsf")
trsf_composed_path = data_path("composed_trsf.trsf")
inv_trsf_ref_path = data_path("arbitrary_trsf_inv.trsf")

r = np.arange(12)
mat_ref = np.array(np.reshape(r, (3, 4)), np.float64)


class TestTrsf(unittest.TestCase):
    def test_create_trsf(self):
        # --- create identity transformation
        identity = create_trsf(param_str_2='-identity',
                               trsf_type=BalTransformation.AFFINE_3D,
                               trsf_unit=BalTransformation.REAL_UNIT)
        identity = create_trsf()
        mat = identity.mat.to_np_array()
        np.testing.assert_equal(mat, np.identity(4, dtype=mat.dtype))
        identity.free()

    def test_inv_trsf(self):
        # --- inverse transformation
        trsf_inp, trsf_ref = BalTransformation(), BalTransformation()
        trsf_inp.read(trsf_arbitrary_path)  # --- trsf_1
        trsf_ref.read(inv_trsf_ref_path)  # --- inv(trsf_1)
        trsf_out = inv_trsf(trsf_inp)
        ref, out = trsf_ref.mat.to_np_array(), trsf_out.mat.to_np_array()
        np.testing.assert_array_almost_equal(ref, out, decimal=8)
        trsf_inp.free()
        trsf_ref.free()
        trsf_out.free()


    def test_compose_trsf(self):
        # --- compose transformation
        trsf_inp, trsf_composed = BalTransformation(), BalTransformation()
        trsf_inp.read(trsf_arbitrary_path)
        trsf_composed.read(trsf_composed_path)
        trsf_out = compose_trsf([trsf_inp, trsf_inp, trsf_inp])
        np.testing.assert_almost_equal(trsf_out.mat.to_np_array(),
                                       trsf_composed.mat.to_np_array())


    def test_apply_trsf_arbitrary(self):
        # --- apply transformation
        trsf_inp = BalTransformation()
        trsf_inp.read(trsf_arbitrary_path)
        img_res = apply_trsf(img_src, trsf_inp)
        np.testing.assert_equal(arbitrary_transformed_ref, img_res)


    def test_apply_trsf_deformable(self):
        # --- apply transformation
        trsf_inp = BalTransformation()
        trsf_inp.read(trsf_deformable_path)
        img_res = apply_trsf(img_src, trsf_inp)
        np.testing.assert_equal(deformable_transformed_ref, img_res)
