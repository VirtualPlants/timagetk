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
This module contains a generic implementation of several averaging algorithms.
"""

try:
    from timagetk.algorithms import mean_images
except ImportError:
    raise ImportError('Import Error')

__all__ = ['averaging']

POSS_METHODS = ['mean_averaging', 'robust_mean_averaging', 'median_averaging',
                'minimum_averaging', 'maximum_averaging']
DEFAULT_METHOD = POSS_METHODS[0]


def averaging(list_images, method=None, **kwds):
    """
    Averaging plugin. Available methods are :

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
    :return: ``SpatialImage`` instance -- image and metadata

    Example
    ----------
    >>> from timagetk.util import data, data_path
    >>> from timagetk.plugins import averaging
    >>> image_path = data_path('time_0_cut.inr')
    >>> input_image = data(image_path)
    >>> list_images = [input_image, input_image, input_image]
    >>> rob_mean_image = averaging(list_images, method='robust_mean_averaging')
    >>> mean_image = averaging(list_images, method='mean_averaging')
    """
    if not method:
        method = DEFAULT_METHOD

    try:
        assert method in POSS_METHODS
    except AssertionError:
        raise NotImplementedError(
            "Unknown method '{}', available methods are: {}".format(method,
                                                                    POSS_METHODS))

    try:
        assert kwds.get('try_plugin', True)
        from openalea.core.service.plugin import plugin_function
    except ImportError:
        if method == 'mean_averaging':
            return mean_averaging(list_images, **kwds)
        elif method == 'robust_mean_averaging':
            return robust_mean_averaging(list_images, **kwds)
        elif method == 'median_averaging':
            return median_averaging(list_images, **kwds)
        elif method == 'minimum_averaging':
            return min_averaging(list_images, **kwds)
        elif method == 'maximum_averaging':
            return max_averaging(list_images, **kwds)
    else:
        func = plugin_function('openalea.image', method)
        if func:
            return func(list_images, **kwds)
        else:
            raise NotImplementedError("Returned 'plugin_function' is None!")


def mean_averaging(list_images, **kwds):
    """
    Mean image from a list of *SpatialImages*.

    Parameters
    ----------
    :param list list_images: list of *SpatialImage*

    Returns
    ----------
    :return: *SpatialImage* instance -- image and associated informations
    """
    sp_img = mean_images(list_images, param_str_2='-mean')
    return sp_img


def robust_mean_averaging(list_images, **kwds):
    """
    Robust mean image from a list of *SpatialImages*.

    Parameters
    ----------
    :param list list_images: list of *SpatialImage*

    Returns
    ----------
    :return: *SpatialImage* instance -- image and associated informations
    """
    sp_img = mean_images(list_images, param_str_2='-robust-mean')
    return sp_img


def median_averaging(list_images, **kwds):
    """
    Median image from a list of *SpatialImages*.

    Parameters
    ----------
    :param list list_images: list of *SpatialImage*

    Returns
    ----------
    :return: *SpatialImage* instance -- image and associated informations
    """
    sp_img = mean_images(list_images, param_str_2='-median')
    return sp_img


def min_averaging(list_images, **kwds):
    """
    Minimum image from a list of *SpatialImages*.

    Parameters
    ----------
    :param list list_images: list of *SpatialImage*

    Returns
    ----------
    :return: *SpatialImage* instance -- image and associated informations
    """
    sp_img = mean_images(list_images, param_str_2='-minimum')
    return sp_img


def max_averaging(list_images, **kwds):
    """
    Maximum image from a list of *SpatialImages*.

    Parameters
    ----------
    :param list list_images: list of *SpatialImage*

    Returns
    ----------
    :return: *SpatialImage* instance -- image and associated informations
    """
    sp_img = mean_images(list_images, param_str_2='-maximum')
    return sp_img
