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
try:
    from timagetk.wrapping.clib import libvtexec, add_doc, return_value
    from timagetk.wrapping.vt_image import vt_image, new_vt_image
    from timagetk.components import SpatialImage
except ImportError as e:
    raise ImportError('Import Error: {}'.format(e))

__all__ = ['LINEARFILTER_DEFAULT', 'linearfilter']
LINEARFILTER_DEFAULT = '-x 0 -y 0 -z 0 -sigma 1.0'


def linearfilter(image, param_str_1=LINEARFILTER_DEFAULT, param_str_2=None, dtype=None):
    """
    Linear filtering algorithms.

    Parameters
    ----------
    :param *SpatialImage* image: *SpatialImage*, input image

    :param str param_str_1: LINEARFILTER_DEFAULT, default is a gaussian filter with an unitary sigma

    :param str param_str_2: optional, optional parameters

    :param *np.dtype* dtype: optional, output image type. By default, the output type is equal to the input type.

    Returns
    ----------
    :return: ``SpatialImage`` instance -- output image and metadata

    Example
    -------
    >>> from timagetk.util import data_path
    >>> from timagetk.components import imread
    >>> from timagetk.algorithms import linearfilter
    >>> img_path = data_path('time_0_cut.inr')
    >>> input_image = imread(img_path)
    >>> output_image = linearfilter(input_image)
    >>> param_str_2 = '-x 0 -y 0 -z 0 -sigma 2.0'
    >>> output_image = linearfilter(input_image, param_str_2=param_str_2)
    """
    try:
        assert isinstance(image, SpatialImage)
    except:
        raise TypeError('Input image must be a SpatialImage')

    if not dtype:
        dtype = image.dtype
    vt_input, vt_res = vt_image(image), new_vt_image(image, dtype=dtype)
    rvalue = libvtexec.API_linearFilter(vt_input.c_ptr, vt_res.c_ptr,
                                        param_str_1, param_str_2)
    out_sp_img = return_value(vt_res.get_spatial_image(), rvalue)
    vt_input.free(), vt_res.free()
    return out_sp_img

add_doc(linearfilter, libvtexec.API_Help_linearFilter)