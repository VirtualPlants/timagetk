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
    from timagetk.wrapping.clib import add_doc, return_value, libvtexec
    from timagetk.wrapping.vt_image import vt_image, new_vt_image
    from timagetk.components import SpatialImage
except ImportError as e:
    raise ImportError('Import Error: {}'.format(e))

__all__ = ['CONNEXE_DEFAULT', 'connexe']
CONNEXE_DEFAULT = '-low-threshold 1 -high-threshold 3 -labels -connectivity 26'  # --- of course, stupid !


def connexe(image, seeds=None, param_str_1=CONNEXE_DEFAULT, param_str_2=None,
            dtype=None):
    """
    Connected components labeling.

    Parameters
    ----------
    :param *SpatialImage* image: *SpatialImage*, input image

    :param *SpatialImage* seeds: *SpatialImage*, optional seeds image

    :param str param_str_1: by default param_str_1 is equal to CONNEXE_DEFAULT.
            By default thresholds are fixed, and one label is associated to
            each connected component

    :param str param_str_2: optional str parameter

    Returns
    ----------
    :return: ``SpatialImage`` instance -- output image and metadata

    Example
    -------
    >>> from timagetk.util import data_path
    >>> from timagetk.components import imread
    >>> from timagetk.algorithms import regionalext, connexe
    >>> img_path = data_path('time_0_cut.inr')
    >>> input_image = imread(img_path)
    >>> regext_img = regionalext(input_image)
    >>> output_image = connexe(regext_img)
    """
    try:
        assert isinstance(image, SpatialImage)
    except AssertionError:
        raise TypeError('Input image must be a SpatialImage')

    if dtype is None:
        dtype = image.dtype
    if seeds is None:
        pt_seeds = None
    else:
        try:
            assert isinstance(seeds, SpatialImage)
        except AssertionError:
            raise TypeError('Seeds image must be a SpatialImage')
        else:
            vt_seeds = vt_image(seeds)
            pt_seeds = vt_seeds.c_ptr

    vt_input, vt_res = vt_image(image), new_vt_image(image, dtype=dtype)
    rvalue = libvtexec.API_connexe(vt_input.c_ptr, pt_seeds, vt_res.c_ptr,
                                   param_str_1, param_str_2)
    out_sp_img = return_value(vt_res.get_spatial_image(), rvalue)
    vt_input.free(), vt_res.free()
    if seeds and isinstance(seeds, SpatialImage):
        vt_seeds.free()

    return out_sp_img

add_doc(connexe, libvtexec.API_Help_connexe)
