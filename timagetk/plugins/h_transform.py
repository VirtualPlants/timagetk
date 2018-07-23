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
#           Jonathan Legrand <jonathan.legrand@ens-lyon.fr>
#
#       See accompanying file LICENSE.txt
# ------------------------------------------------------------------------------

"""
This module contains a generic implementation of several h_transform algorithms.
"""

try:
    from timagetk.util import _input_img_check
    from timagetk.util import _method_check
    from timagetk.util import _general_kwargs
    from timagetk.util import _parallel_kwargs
    from timagetk.algorithms import regionalext
    from timagetk.components.spatial_image import SpatialImage
except ImportError as e:
    raise ImportError('Import Error: {}'.format(e))

__all__ = ['h_transform']

POSS_METHODS = ['h_transform_min', 'h_transform_max']
DEFAULT_METHOD = 0  # index of the default method in POSS_METHODS
DEF_H = 1  # default h-transform value


def _ht_kwargs(**kwargs):
    """
    Set parameters default values and make sure they are of the right type.
    """
    str_param = ""
    # - By default 'h' is equal to 1:
    h = abs(int(kwargs.get('h', DEF_H)))
    str_param += ' -h %d' % h

    # - Parse general kwargs:
    str_param += _general_kwargs(**kwargs)
    # - Parse parallelism kwargs:
    str_param += _parallel_kwargs(**kwargs)

    return str_param


def h_transform(input_image, method=None, **kwargs):
    """
    h_transform plugin for local minima or maxima detection.
    Available methods are:
      * h_transform_min
      * h_transform_max

    Parameters
    ----------
    input_image : SpatialImage
         input image to transform
    method: str, optional
        used method, by default 'h_transform_min'

    Returns
    -------
    SpatialImage
         transformed image with its metadata

    Example
    -------
    >>> from timagetk.util import data_path
    >>> from timagetk.components import imread
    >>> from timagetk.plugins import h_transform
    >>> img_path = data_path('filtering_src.inr')
    >>> input_image = imread(img_path)
    >>> reg_max_image = h_transform(input_image)
    >>> reg_max_image = h_transform(input_image, h=3, method='h_transform_max')
    >>> reg_min_image = h_transform(input_image, h=3, method='h_transform_min')
    """
    # - Assert the 'input_image' is a SpatialImage instance:
    _input_img_check(input_image)
    # - Set method if None and check it is a valid method:
    method = _method_check(method, POSS_METHODS, DEFAULT_METHOD)

    try:
        assert kwargs.get('try_plugin', False)
        from openalea.core.service.plugin import plugin_function
    except AssertionError or ImportError:
        if method == 'h_transform_min':
            return h_transform_min(input_image, **kwargs)
        else:
            return h_transform_max(input_image, **kwargs)
    else:
        func = plugin_function('openalea.image', method)
        if func is not None:
            print "WARNING: using 'plugin' functionality from 'openalea.core'!"
            return func(input_image, **kwargs)
        else:
            raise NotImplementedError("Returned 'plugin_function' is None!")


def h_transform_min(input_image, h=DEF_H, **kwargs):
    """
    Minimum h-transform, detect local minima of given height 'h'.

    Parameters
    ----------
    input_image : SpatialImage
         input image to transform
    h : int, optional
        height value to use in transformation, default is 1

    Returns
    -------
    SpatialImage
         transformed image with its metadata
    """
    params = '-minima'
    params += _ht_kwargs(h=h, **kwargs)
    return regionalext(input_image, param_str_2=params)


def h_transform_max(input_image, h=DEF_H, **kwargs):
    """
    Maximum h-transform, detect local maxima of given height 'h'.

    Parameters
    ----------
    input_image : SpatialImage
         input image to transform
    h : int, optional
        height value to use in transformation, default is 1

    Returns
    -------
    SpatialImage
         transformed image with its metadata
    """
    params = '-maxima'
    params += _ht_kwargs(h=h, **kwargs)
    return regionalext(input_image, param_str_2=params)
