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
from ctypes import pointer, POINTER
import numpy as np
try:
    from timagetk.wrapping.clib import libvtexec, add_doc, return_value
    from timagetk.wrapping.vtImage import _VT_IMAGE
    from timagetk.wrapping.vt_image import vt_image, new_vt_image
    from timagetk.components import SpatialImage
except ImportError:
    raise ImportError('Import Error')

__all__ = ['MEANIMAGES_DEFAULT', 'mean_images']
MEANIMAGES_DEFAULT = '-mean'


def mean_images(list_spatial_images, list_spatial_masks=None,
                param_str_1=MEANIMAGES_DEFAULT, param_str_2=None):
    """
    Mean image algorithms.

    Parameters
    ----------
    :param list list_spatial_images: input list of *SpatialImage* (grayscale)

    :param list list_spatial_masks: optional, input list of *SpatialImages* (binary)

    :param str param_str_1: MEANIMAGES_DEFAULT, by default a mean image is computed

    :param str param_str_2: optional, optional parameters

    Returns
    ----------
    :return: *SpatialImage* output image

    Example
    -------
    >>> from timagetk.util import data_path
    >>> from timagetk.components import imread
    >>> from timagetk.algorithms import mean_images
    >>> img_path = data_path('time_0_cut.inr')
    >>> input_image = imread(img_path)
    >>> output_image = mean_images([input_image, input_image, input_image])
    """
    conds = [True if isinstance(val, SpatialImage) else False
                for ind, val in enumerate(list_spatial_images)]
    if False not in conds:
        dtype_list = [sp_img.dtype for ind, sp_img in enumerate(list_spatial_images)]
        comm_type = np.find_common_type(dtype_list, [])
        if list_spatial_masks is None:
            mask_ptr = None
        else:
            list_c_vt_spatial_masks = POINTER(_VT_IMAGE) * len(list_spatial_images)
            c_input_masks = []
            for ind, spatial_mask in enumerate(list_spatial_masks):
                vt_input_mask = vt_image(spatial_mask)
                c_input_masks.append(vt_input_mask.get_vt_image())
            mask_ptr = list_c_vt_spatial_masks( *[pointer(c_input_mask)
                                                for c_input_mask in c_input_masks])
        list_c_vt_images = POINTER(_VT_IMAGE) * len(list_spatial_images)
        c_input_images = []
        for ind, spatial_image in enumerate(list_spatial_images):
            vt_input = vt_image(spatial_image)
            c_input_images.append(vt_input.get_vt_image())
        sp_img_ptr = list_c_vt_images(*[pointer(c_input)
                                        for c_input in c_input_images])
        vt_res = new_vt_image(list_spatial_images[0], dtype=comm_type)
        rvalue = libvtexec.API_meanImages(sp_img_ptr, mask_ptr,
                                          len(list_spatial_images), vt_res.c_ptr,
                                          param_str_1, param_str_2)
        out_sp_img = return_value(vt_res.get_spatial_image(), rvalue)
        return out_sp_img
    else:
        raise TypeError('Input images must be a list of SpatialImage')
        return
add_doc(mean_images, libvtexec.API_Help_meanImages)