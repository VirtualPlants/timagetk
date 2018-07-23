# -*- coding: utf-8 -*-
# -*- python -*-
#
#
#       Copyright 2016 INRIA
#
#       File author(s):
#           Guillaume Baty <guillaume.baty@inria.fr>
#           Sophie Ribes <sophie.ribes@inria.fr>
#           Gregoire Malandain <gregoire.malandain@inria.fr>
#
#       See accompanying file LICENSE.txt
#------------------------------------------------------------------------------

#--- Aug. 2016
from ctypes import pointer
import numpy as np
try:
    from timagetk.wrapping.clib import add_doc, libblockmatching
    from timagetk.wrapping.balImage import BAL_IMAGE
    from timagetk.wrapping.bal_image import BalImage, init_c_bal_image
    from timagetk.wrapping.bal_image import allocate_c_bal_image, spatial_image_to_bal_image_fields
    from timagetk.wrapping.bal_trsf import BalTransformation
    from timagetk.components import SpatialImage
except ImportError as e:
    raise ImportError('Import Error: {}'.format(e))

__all__ = ['BLOCKMATCHING_DEFAULT', 'blockmatching']
BLOCKMATCHING_DEFAULT = '-trsf-type rigid'

# previous API - FRED
# Three plugins (rigid, affine and deformable registration) were implemented
# def blockmatching(floating_image,
#                  reference_image,
#                  transformation_type = 'rigid',
#                  init_result_transformation=None,
#                  left_transformation=None,
#                  param_str=''):


def blockmatching(floating_image, reference_image,
                  init_result_transformation=None, left_transformation=None,
                  param_str_1=BLOCKMATCHING_DEFAULT, param_str_2=None, dtype=None):
    """
    Registration algorithm. Registers a floating_image onto a reference_image.

    Parameters
    ----------
    :param *SpatialImage* floating_image: *SpatialImage*, floating image

    :param *SpatialImage* reference_image: *SpatialImage*, reference image

    :param *BalTransformation* init_result_transformation: optional, BalTransformation init transformation.
                By default init_result_transformation is None
                If given, init_result_transformation is modified as result

    :param *BalTransformation* left_transformation: optional, BalTransformation left transformation
                By default left_transformation is None

    :param str param_str1: by default a rigid registration is computed and
                            param_str_1=BLOCKMATCHING_DEFAULT='-trsf-type rigid'

    :param str param_str_2: optional, type string

    :param *np.dtype* dtype: optional, output image type. By default, the output type is equal to the input type.

    Returns
    ----------
    :return: ``BalTransformation`` transformation -- trsf_out

    :return: ``SpatialImage`` instance -- res_img, image and metadata

    Example
    -------
    >>> from timagetk.util import data_path
    >>> from timagetk.components import imread
    >>> flo_path = data_path('time_0_cut.inr')
    >>> floating_image = imread(flo_path)
    >>> ref_path = data_path('time_1_cut.inr')
    >>> reference_image = imread(ref_path)
    >>> trsf_rig, res_rig = blockmatching(floating_image, reference_image) # rigid registration
    >>> param_str_2 = '-trsf-type vectorfield'
    >>> trsf_def, res_def = blockmatching(floating_image, reference_image,
                                          init_result_transformation=trsf_rig,
                                          param_str_2=param_str_2) # deformable registration
    """
    try:
        assert isinstance(floating_image, SpatialImage)
    except AssertionError:
        raise TypeError(
            "Input 'floating_image' must be a SpatialImage instance")
    try:
        assert isinstance(reference_image, SpatialImage)
    except AssertionError:
        raise TypeError(
            "Input 'reference_image' must be a SpatialImage instance")

    # - Initialise BalImage:
    bal_floating_image = BalImage(floating_image)
    bal_reference_image = BalImage(reference_image)
    # - Get keyword arguments to initialise result image:
    kwargs = spatial_image_to_bal_image_fields(reference_image)
    if dtype:
        kwargs['np_type'] = dtype
    # - Initialise result image:
    c_img_res = BAL_IMAGE()
    init_c_bal_image(c_img_res, **kwargs)
    allocate_c_bal_image(c_img_res, np.ndarray(kwargs['shape'], kwargs['np_type']))
    img_res = BalImage(c_bal_image=c_img_res)

    # --- old API FRED, see plugins
    #    if transformation_type:
    #         param_str_2 = '-trsf-type '+transformation_type+' '+param_str
    #    else:
    #         param_str_2 = param_str
    # - Performs blockmatching registration:
    trsf_out_ptr = libblockmatching.API_blockmatching(bal_floating_image.c_ptr,
                                                      bal_reference_image.c_ptr,
                                                      pointer(c_img_res),
                                                      left_transformation.c_ptr if left_transformation else None,
                                                      init_result_transformation.c_ptr if init_result_transformation else None,
                                                      param_str_1, param_str_2)
    if init_result_transformation:
        # If init_result_transformation is given, this transformation is modified
        # during registration and trsf_out is init_result_transformation
        trsf_out = init_result_transformation
    else:
        trsf_out = BalTransformation(c_bal_trsf=trsf_out_ptr.contents)
    # - Transform result BalImage to SpatialImage:
    sp_img = img_res.to_spatial_image()
    # - Free memory:
    bal_floating_image.free(), bal_reference_image.free(), img_res.free()

    return trsf_out, sp_img

add_doc(blockmatching, libblockmatching.API_Help_blockmatching)

