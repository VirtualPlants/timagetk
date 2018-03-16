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
from ctypes import pointer, c_int, Structure, POINTER
try:
    from timagetk.wrapping.clib import libvtexec, libvp, add_doc, return_value
    from timagetk.wrapping.vt_image import vt_image, new_vt_image
    from timagetk.components.spatial_image import SpatialImage
except ImportError:
    raise ImportError('Import Error')

__all__ = ['MORPHO_DEFAULT', 'morpho', 'CELL_FILTER_DEFAULT', 'cell_filter']

MORPHO_DEFAULT = '-dilation'
CELL_FILTER_DEFAULT = '-dilation'
# TODO : move and check morpho structures


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
    _list = pointer(_typeMorphoToolsPoint(0, 0, 0, 0))
    user_se = _typeMorphoToolsList(0, _list)
    return _typeStructuringElement(iterations, connectivity, user_se, radius)


def morpho(image, struct_elt=None, param_str_1=MORPHO_DEFAULT, param_str_2=None, dtype=None):
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

    Example
    -------
    >>> output_image = morpho(input_image)
    """
    if isinstance(image, SpatialImage):
        if dtype is None:
            dtype = image.dtype
        if struct_elt is not None:
            struct_elt = pointer(struct_elt)
        vt_input, vt_res = vt_image(image), new_vt_image(image, dtype=dtype)
        rvalue = libvtexec.API_morpho(vt_input.c_ptr, vt_res.c_ptr, struct_elt,
                                      param_str_1, param_str_2)
        out_sp_img = return_value(vt_res.get_spatial_image(), rvalue)
        vt_input.free(), vt_res.free()
        return out_sp_img
    else:
        raise TypeError('Input image must be a SpatialImage')
        return


def cell_filter(image, struct_elt=None, param_str_1=CELL_FILTER_DEFAULT, param_str_2=None, dtype=None):
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
    if isinstance(image, SpatialImage):
        if dtype is None:
            dtype = image.dtype
        if struct_elt is not None:
            struct_elt = pointer(struct_elt)
        vt_input, vt_res = vt_image(image), new_vt_image(image, dtype=dtype)
        rvalue = libvp.API_cellfilter(vt_input.c_ptr, vt_res.c_ptr, struct_elt,
                                      param_str_1, param_str_2)
        out_sp_img = return_value(vt_res.get_spatial_image(), rvalue)
        vt_input.free(), vt_res.free()
        return out_sp_img
    else:
        raise TypeError('Input image must be a SpatialImage')
        return

add_doc(morpho, libvtexec.API_Help_morpho)
add_doc(cell_filter, libvp.API_Help_cellfilter)
