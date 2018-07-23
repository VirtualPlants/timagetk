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
# ------------------------------------------------------------------------------

# --- Aug. 2016
try:
    from timagetk.wrapping.clib import libvtexec, add_doc, return_value
    from timagetk.wrapping.vt_image import vt_image, new_vt_image
    from timagetk.components.spatial_image import SpatialImage
except ImportError as e:
    raise ImportError('Import Error: {}'.format(e))

__all__ = ['WATERSHED_DEFAULT', 'watershed']

WATERSHED_DEFAULT = '-labelchoice most'
# WATERSHED_DEFAULT = ''


def watershed(image, seeds, param_str_1=WATERSHED_DEFAULT, param_str_2=None,
              dtype=None):
    """
    Seeded watershed algorithm.

    Parameters
    ----------
    :param *SpatialImage* image: *SpatialImage*, input image

    :param *SpatialImage* seeds: *SpatialImage*, seeds image, each marker have an unique value

    :param str param_str_1: WATERSHED_DEFAULT

    :param str param_str_2: optional, optional parameters

    :param *np.dtype* dtype: optional, output image type. By default, the output type is equal to the input type.

    Returns
    ----------
    :return: ``SpatialImage`` instance -- output image and metadata

    Example
    -------
    >>> from timagetk.util import data_path
    >>> from timagetk.components import imread
    >>> from timagetk.algorithms import linearfilter, regionalext, connexe, watershed
    >>> img_path = data_path('time_0_cut.inr')
    >>> input_image = imread(img_path)
    >>> smooth_img = linearfilter(input_image)
    >>> regext_img = regionalext(smooth_img)
    >>> conn_img = connexe(regext_img)
    >>> output_image = watershed(smooth_img, conn_img)
    """
    # - Check image is a `SpatialImage`:
    try:
        assert isinstance(image, SpatialImage)
    except AssertionError:
        raise ValueError("Parameter 'image' is not a SpatialImage")
    # - Check seeds is a `SpatialImage`:
    try:
        assert isinstance(seeds, SpatialImage)
    except AssertionError:
        raise ValueError("Parameter 'seeds' is not a SpatialImage")

    if dtype is None:
        dtype = seeds.dtype
    # - Initialise objects:
    vt_img = vt_image(image)
    vt_seeds = vt_image(seeds)
    vt_res = new_vt_image(seeds, dtype=dtype)
    # - Run watershed algorithm:
    rvalue = libvtexec.API_watershed(vt_img.c_ptr, vt_seeds.c_ptr, vt_res.c_ptr,
                                     param_str_1, param_str_2)
    # - Get segmented image:
    out_sp_img = return_value(vt_res.get_spatial_image(), rvalue)
    # - Free memory:
    vt_img.free(), vt_seeds.free(), vt_res.free()

    return out_sp_img


add_doc(watershed, libvtexec.API_Help_watershed)
