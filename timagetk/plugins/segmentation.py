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
This module contains a generic implementation of several segmentation algorithms.
"""

try:
    from timagetk.algorithms import watershed
    from timagetk.components.spatial_image import SpatialImage
except ImportError:
    raise ImportError('Import Error')

__all__ = ['segmentation', 'seeded_watershed']


POSS_METHODS = ['seeded_watershed']
DEFAULT_METHOD = POSS_METHODS[0]
POSS_CONTROLS = ['most', 'first', 'min']
DEFAULT_CONTROL = POSS_CONTROLS[0]

def get_param_str_2(**kwds):
    """
    Set parameters default values and make sure they are of the right type.
    """
    str_param = ""
    if kwds.get('param', False):
        str_param += ' -param'
    if kwds.get('debug', False):
        str_param += ' -debug'
    if kwds.get('parallel', True):
        str_param += ' -parallel'
        str_param += ' -parallel-type ' + kwds.get('parallel_type', 'thread')
    if kwds.get('time', True):
        str_param += ' -time'

    return str_param


def segmentation(input_image, seeds_image, method=None, **kwds):
    """
    Segmentation plugin. Available methods are :

    * seeded_watershed

    Parameters
    ----------
    :param *SpatialImage* input_image: input *SpatialImage*

    :param *SpatialImage* seeds_image: seeds *SpatialImage*

    :param str method: used method (example: 'seeded_watershed')

    Returns
    ----------
    :return: ``SpatialImage`` instance -- image and metadata

    Example
    ----------
    >>> from timagetk.util import data, data_path
    >>> from timagetk.plugins import linear_filtering, h_transform, region_labeling, segmentation
    >>> image_path = data_path('segmentation_src.inr')
    >>> input_image = data(image_path)
    >>> smooth_image = linear_filtering(input_image, std_dev=2.0, method='gaussian_smoothing')
    >>> regext_image = h_transform(smooth_image, h=5, method='h_transform_min')
    >>> seeds_image = region_labeling(regext_image, low_threshold=1, high_threshold=3,
                                      method='connected_components')
    >>> segmented_image = segmentation(smooth_image, seeds_image, control='first',
                                       method='seeded_watershed')
    """
    # - Check `input_image` type:
    try:
        assert isinstance(input_image, SpatialImage)
    except AssertionError:
        raise TypeError('Input image must be a `SpatialImage` object.')

    if method is None:
        method = DEFAULT_METHOD

    try:
        assert method in POSS_METHODS
    except AssertionError:
        raise NotImplementedError(
            "Unknown method '{}', possible methods are: {}.".format(method,
                                                                    POSS_METHODS))

    try:
        assert kwds.get('try_plugin', True)
        from openalea.core.service.plugin import plugin_function
    except AssertionError or ImportError:
        control_val = kwds.get('control', None)
        return seeded_watershed(input_image, seeds_image, control=control_val,
                                **kwds)
    else:
        func = plugin_function('openalea.image', method)
        if func is not None:
            return func(input_image, seeds_image, **kwds)
        else:
            raise NotImplementedError("Returned 'plugin_function' is None!")


def seeded_watershed(input_image, seeds_image, control=None, **kwds):
    """
    Seeded watershed algorithm

    Parameters
    ----------
    :param *SpatialImage* input_image: input *SpatialImage*

    :param *SpatialImage* seeds_image: seeds *SpatialImage*

    :param str control: optional, deal with watershed conflicts.
                        Default is most represented label

    Returns
    ----------
    :return: ``SpatialImage`` instance -- image and metadata
    """
    if control:
        if control not in POSS_CONTROLS:
            raise NotImplementedError(
                "Unknown control '{}', available control methods: {}".format(
                    control, POSS_CONTROLS))
        else:
            param_str_2 = '-labelchoice ' + str(control)
    else:
        param_str_2 = '-labelchoice ' + DEFAULT_CONTROL

    param_str_2 += get_param_str_2(**kwds)
    return watershed(input_image, seeds_image, param_str_2=param_str_2)
