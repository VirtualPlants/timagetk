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

import warnings
# --- Aug. 2016
from ctypes import pointer, c_int, Structure, POINTER

try:
    from timagetk.util import _input_img_check
    from timagetk.util import _general_kwargs
    from timagetk.util import _parallel_kwargs
    from timagetk.util import _method_check
    from timagetk.util import clean_warning
    from timagetk.wrapping.clib import libvtexec, libvp, add_doc, return_value
    from timagetk.wrapping.vt_image import vt_image, new_vt_image
    from timagetk.components.spatial_image import SpatialImage
except ImportError as e:
    raise ImportError('Import Error: {}'.format(e))

__all__ = ['MORPHO_DEFAULT', 'morpho', 'CELL_FILTER_DEFAULT', 'cell_filter']

# Using '-sphere' is equivalent to '-connectivity 18'
MORPHO_DEFAULT = '-dilation -sphere -radius 1 -iterations 1'
CELL_FILTER_DEFAULT = '-dilation -sphere -radius 1 -iterations 1'


class _typeMorphoToolsPoint(Structure):
    """
    Definition of typeMorphoToolsPoint structure
    """
    _fields_ = [("x", c_int),
                ("y", c_int),
                ("z", c_int),
                ("o", c_int)
                ]


class _typeMorphoToolsList(Structure):
    """
    Definition of typeMorphoToolsList structure
    """
    _fields_ = [("nb", c_int),
                ("list", POINTER(_typeMorphoToolsPoint))
                ]


class _typeStructuringElement(Structure):
    """
    Definition of typeStructuringElement structure

    Morphology :
        - iterations number
        - connectivity
    """
    _fields_ = [("nbIterations", c_int),
                ("connectivity", c_int),
                # structuring element given by the user
                ("userDefinedSE", _typeMorphoToolsList),
                ("radius", c_int),
                ("dimension", c_int)
                ]


def default_structuring_element():
    """
    Default structuring element.
    This is a cube (26-connectivity) of radius 1.
    """
    return structuring_element(1, 1, 26)


def structuring_element(radius, iterations, connectivity=26):
    """
    Create a structuring element to use with `cell_filter` or `morpho`.
    Connectivity is among the 4-, 6-, 8-, 18-, 26-neighborhoods.
    4 and 8 are 2-D elements, the others being 3-D (default = 26).

    Note: not sure why there is an `iterations` parameter... given to Morpheme functions?
    """
    assert connectivity in [4, 6, 8, 10, 18, 26]
    print "here"
    _list = pointer(_typeMorphoToolsPoint(0, 0, 0, 0))
    print "here1"
    user_se = _typeMorphoToolsList(0, _list)
    print "here2"
    return _typeStructuringElement(iterations, connectivity, user_se, radius)


def morpho(image, struct_elt=None, param_str_1=MORPHO_DEFAULT, param_str_2=None,
           dtype=None):
    """
    Mathematical morphology algorithms on grayscale images.

    Parameters
    ----------
    :param *SpatialImage* image: *SpatialImage*, input image

    :param struct_elt: optional, structuring element.
                    By default an approximation of an euclidean ball is used

    :param str param_str_1: MORPHO_DEFAULT, by default a dilation is applied

    :param str param_str_2: optional, optional parameters

    :param *np.dtype* dtype: optional, output image type. By default, the output type is equal to the input type.

    Returns
    ----------
    :return: ``SpatialImage`` instance -- output image and metadata

    Notes
    -----
    '-connectivity' parameter override '-sphere' parameter

    Example
    -------
    >>> output_image = morpho(input_image)
    """
    # - Assert 'image' is a SpatialImage instance:
    _input_img_check(image)

    if dtype is None:
        dtype = image.dtype
    if struct_elt is not None:
        struct_elt = pointer(struct_elt)
    # Core Dumped !
    # else:
    #     print "Initializing default structuring element..."
    #     struct_elt = default_structuring_element()

    # Raise a 'SyntaxWarning' when using both 'sphere' and 'connectivity' params
    all_params = param_str_1 + param_str_2
    if all_params.find('sphere') != -1 and all_params.find(
            'connectivity') != -1:
        msg = "Function `morpho`, overriding 'sphere' parameter by given 'connectivity'!"
        warnings.formatwarning = clean_warning
        warnings.warn(msg, SyntaxWarning)

    vt_input, vt_res = vt_image(image), new_vt_image(image, dtype=dtype)
    rvalue = libvtexec.API_morpho(vt_input.c_ptr, vt_res.c_ptr, struct_elt,
                                  param_str_1, param_str_2)
    out_sp_img = return_value(vt_res.get_spatial_image(), rvalue)
    vt_input.free(), vt_res.free()
    return out_sp_img


def cell_filter(image, struct_elt=None, param_str_1=CELL_FILTER_DEFAULT,
                param_str_2=None, dtype=None):
    """
    Mathematical morphology algorithms on segmented images.

    Parameters
    ----------
    :param *SpatialImage* image: *SpatialImage*, input image

    :param struct_elt: optional, structuring element.
                    By default an approximation of an euclidean ball is used

    :param str param_str_1: CELL_FILTER_DEFAULT, by default a dilation is applied

    :param str param_str_2: optional, optional parameters

    :param *np.dtype* dtype: optional, output image type. By default, the output type is equal to the input type.

    Returns
    ----------
    :return: ``SpatialImage`` instance -- output image and metadata

    Example
    -------
    >>> output_image = cell_filter(input_image)
    """
    # - Assert 'image' is a SpatialImage instance:
    _input_img_check(image)

    if dtype is None:
        dtype = image.dtype
    if struct_elt is not None:
        struct_elt = pointer(struct_elt)

    # Raise a 'SyntaxWarning' when using both 'sphere' and 'connectivity' params
    all_params = param_str_1 + param_str_2
    if all_params.find('sphere') != -1 and all_params.find(
            'connectivity') != -1:
        msg = "Function `morpho`, overriding 'sphere' parameter by given 'connectivity'!\n"
        warnings.formatwarning = clean_warning
        warnings.warn(msg, SyntaxWarning)

    vt_input, vt_res = vt_image(image), new_vt_image(image, dtype=dtype)
    rvalue = libvp.API_cellfilter(vt_input.c_ptr, vt_res.c_ptr, struct_elt,
                                  param_str_1, param_str_2)
    out_sp_img = return_value(vt_res.get_spatial_image(), rvalue)
    vt_input.free(), vt_res.free()
    return out_sp_img


add_doc(morpho, libvtexec.API_Help_morpho)
add_doc(cell_filter, libvp.API_Help_cellfilter)
