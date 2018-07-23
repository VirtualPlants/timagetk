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
This module contains a generic implementation of several averaging algorithms.
"""

try:
    from timagetk.util import _method_check
    from timagetk.algorithms import mean_images
except ImportError as e:
    raise ImportError('Import Error: {}'.format(e))

__all__ = ['averaging']

POSS_METHODS = ['mean_averaging', 'robust_mean_averaging', 'median_averaging',
                'minimum_averaging', 'maximum_averaging']
DEFAULT_METHOD = 0  # index of the default method in POSS_METHODS


def averaging(list_images, method=None, **kwargs):
    """
    Averaging plugin
    Available methods are:

      * mean_averaging
      * robust_mean_averaging
      * median_averaging
      * minimum_averaging
      * maximum_averaging

    Parameters
    ----------
    :param list list_images: list of *SpatialImage*

    :param str method: used method (example: 'mean_averaging')

    Returns
    ----------
    SpatialImage
         transformed image with its metadata

    Example
    ----------
    >>> from timagetk.util import data_path
    >>> from timagetk.components import imread
    >>> from timagetk.plugins import averaging
    >>> image_path = data_path('time_0_cut.inr')
    >>> input_image = imread(image_path)
    >>> list_images = [input_image, input_image, input_image]
    >>> rob_mean_image = averaging(list_images, method='robust_mean_averaging')
    >>> mean_image = averaging(list_images, method='mean_averaging')
    """
    method = _method_check(method, POSS_METHODS, DEFAULT_METHOD)

    try:
        assert kwargs.get('try_plugin', False)
        from openalea.core.service.plugin import plugin_function
    except AssertionError or ImportError:
        if method == 'mean_averaging':
            return mean_averaging(list_images, **kwargs)
        elif method == 'robust_mean_averaging':
            return robust_mean_averaging(list_images, **kwargs)
        elif method == 'median_averaging':
            return median_averaging(list_images, **kwargs)
        elif method == 'minimum_averaging':
            return min_averaging(list_images, **kwargs)
        elif method == 'maximum_averaging':
            return max_averaging(list_images, **kwargs)
    else:
        func = plugin_function('openalea.image', method)
        if func is not None:
            print "WARNING: using 'plugin' functionality from 'openalea.core'!"
            return func(list_images, **kwargs)
        else:
            raise NotImplementedError("Returned 'plugin_function' is None!")


def mean_averaging(list_images, **kwargs):
    """
    Mean image from a list of *SpatialImages*.

    Parameters
    ----------
    :param list list_images: list of *SpatialImage*

    Returns
    ----------
    SpatialImage: image and metadata
    """
    sp_img = mean_images(list_images, param_str_2='-mean')
    return sp_img


def robust_mean_averaging(list_images, **kwargs):
    """
    Robust mean image from a list of *SpatialImages*.

    Parameters
    ----------
    :param list list_images: list of *SpatialImage*

    Returns
    ----------
    SpatialImage: image and metadata
    """
    sp_img = mean_images(list_images, param_str_2='-robust-mean')
    return sp_img


def median_averaging(list_images, **kwargs):
    """
    Median image from a list of *SpatialImages*.

    Parameters
    ----------
    :param list list_images: list of *SpatialImage*

    Returns
    ----------
    SpatialImage: image and metadata
    """
    sp_img = mean_images(list_images, param_str_2='-median')
    return sp_img


def min_averaging(list_images, **kwargs):
    """
    Minimum image from a list of *SpatialImages*.

    Parameters
    ----------
    :param list list_images: list of *SpatialImage*

    Returns
    ----------
    SpatialImage: image and metadata
    """
    sp_img = mean_images(list_images, param_str_2='-minimum')
    return sp_img


def max_averaging(list_images, **kwargs):
    """
    Maximum image from a list of *SpatialImages*.

    Parameters
    ----------
    :param list list_images: list of *SpatialImage*

    Returns
    ----------
    SpatialImage: image and metadata
    """
    sp_img = mean_images(list_images, param_str_2='-maximum')
    return sp_img
