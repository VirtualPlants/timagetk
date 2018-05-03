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

"""
This module contains a generic implementation of several h_transform algorithms.
"""

try:
    from timagetk.algorithms import regionalext
    from timagetk.components.spatial_image import SpatialImage
except ImportError:
    raise ImportError('Import Error')

__all__ = ['h_transform']

POSS_METHODS = ['h_transform_min', 'h_transform_max']
DEFAULT_METHOD = POSS_METHODS[0]


def h_transform(input_image, method=None, **kwds):
    """
    h_transform plugin. Available methods are :

    * h_transform_min
    * h_transform_max

    Parameters
    ----------
    :param *SpatialImage* input_image: input *SpatialImage*

    :param str method: used method (example: 'min')

    :param int h: optional, h value. Default: h=1

    Returns
    ----------
    :return: ``SpatialImage`` instance -- image and metadata

    Example
    ----------
    >>> from timagetk.util import data, data_path
    >>> from timagetk.plugins import h_transform
    >>> img_path = data_path('filtering_src.inr')
    >>> input_image = data(img_path)
    >>> reg_max_image = h_transform(input_image)
    >>> reg_max_image = h_transform(input_image, h=3, method='h_transform_max')
    >>> reg_min_image = h_transform(input_image, h=3, method='h_transform_min')
    """
    try:
        assert isinstance(input_image, SpatialImage)
    except AssertionError:
        raise TypeError('Input image must be a SpatialImage')

    if method is None:
        method = DEFAULT_METHOD

    try:
        assert method in POSS_METHODS
    except AssertionError:
        raise NotImplementedError(
            "Unknown method '{}', available methods are: {}".format(method,
                                                                    POSS_METHODS))
    try:
        assert kwds.get('try_plugin', False)
        from openalea.core.service.plugin import plugin_function
    except AssertionError or ImportError:
        h_val = kwds.get('h', None)
        if method == 'h_transform_min':
            return h_transform_min(input_image, h=h_val)
        else:
            return h_transform_max(input_image, h=h_val)
    else:
        func = plugin_function('openalea.image', method)
        if func is not None:
            return func(input_image, **kwds)
        else:
            raise NotImplementedError("Returned 'plugin_function' is None!")


def h_transform_min(input_image, h=None, **kwds):
    """
    Min h transform.

     Parameters
    ----------
    :param *SpatialImage* input_image: input *SpatialImage*

    :param int h: optional, h value. Default: h=1

    Returns
    ----------
    :return: ``SpatialImage`` instance -- image and metadata
    """
    try:
        assert isinstance(input_image, SpatialImage)
    except AssertionError:
        raise TypeError('Input image must be a SpatialImage')

    if h is None:
        h = 1
    else:
        h = abs(int(h))
    params = '-minima -h %s' % h
    return regionalext(input_image, param_str_2=params)


def h_transform_max(input_image, h=None, **kwds):
    """
    Max h transform

     Parameters
    ----------
    :param *SpatialImage* input_image: input *SpatialImage*

    :param int h: optional, h value. Default: h=1

    Returns
    ----------
    :return: ``SpatialImage`` instance -- image and metadata
    """
    try:
        assert isinstance(input_image, SpatialImage)
    except AssertionError:
        raise TypeError('Input image must be a SpatialImage')

    if h is None:
        h = 1
    else:
        h = abs(int(h))
    params = '-maxima -h %s' % h
    return regionalext(input_image, param_str_2=params)
