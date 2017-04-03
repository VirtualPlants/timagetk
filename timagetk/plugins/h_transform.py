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

"""
This module contains a generic implementation of several h_transform algorithms.
"""

try:
    from timagetk.algorithms import regionalext
    from timagetk.components.spatial_image import SpatialImage
except ImportError:
    raise ImportError('Import Error')


__all__ = ['h_transform']


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
    poss_methods = ['h_transform_min', 'h_transform_max']
    conds = isinstance(input_image, SpatialImage)
    if conds:
        if method is None:
            return h_transform_min(input_image)
        elif method is not None:
            if method in poss_methods:
                try:
                    from openalea.core.service.plugin import plugin_function
                    func = plugin_function('openalea.image', method)
                    if func is not None:
                        return func(input_image, **kwds)
                except:
                    h_val = kwds.get('h', None)
                    if method=='h_transform_min':
                        return h_transform_min(input_image, h=h_val)
                    elif method=='h_transform_max':
                        return h_transform_max(input_image, h=h_val)
            else:
                print('Available methods :'), poss_methods
                raise NotImplementedError(method)
    else:
        raise TypeError('Input image must be a SpatialImage')
        return


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
    conds = isinstance(input_image, SpatialImage)
    if conds:
        if h is None:
            h = 1
        else:
            h = abs(int(h))
        params = '-minima -h %s' % h
        return regionalext(input_image, param_str_2=params)
    else:
        raise TypeError('Input image must be a SpatialImage')
        return


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
    conds = isinstance(input_image, SpatialImage)
    if conds:
        if h is None:
            h = 1
        else:
            h = abs(int(h))
        params = '-maxima -h %s' % h
        return regionalext(input_image, param_str_2=params)
    else:
        raise TypeError('Input image must be a SpatialImage')
        return