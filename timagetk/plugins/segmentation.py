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
This module contains a generic implementation of several segmentation algorithms.
"""

try:
    from timagetk.algorithms import watershed
    from timagetk.components.spatial_image import SpatialImage
except ImportError:
    raise ImportError('Import Error')


__all__ = ['segmentation', 'seeded_watershed']

def _method_param_check(**kwds):
    """
    Set parameters default values and make sure they are of the right type.
    """
    str_param = ""
    if kwds.get('param', True):
        str_param += ' -param'
    if kwds.get('debug', False):
        str_param += ' -debug'
    if kwds.get('parallel', True):
        str_param += ' -parallel'
        str_param += ' -parallel-type '+kwds.get('parallel_type', 'thread')
    if kwds.get('time', True):
        str_param += ' -time'
    return str_param

def _method_img_check(input_image):
    """
    Used to check `input_image` type and isometry.
    """
    # - Check the `input_image` is indeed a `SpatialImage`
    conds = isinstance(input_image, SpatialImage)
    if not conds:
        raise TypeError('Input image must be a `SpatialImage` object.')
    else:
        return


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
    poss_methods = ['seeded_watershed']
    # - Check `input_image` type and isometry:
    _method_img_check(input_image)
    if method is None:
        return seeded_watershed(input_image, seeds_image, **kwds)

    try:
        assert method in poss_methods
    except:
        print('Available methods :'), poss_methods
        raise NotImplementedError(method)

    try:
        from openalea.core.service.plugin import plugin_function
        func = plugin_function('openalea.image', method)
        if func is not None:
            return func(input_image, seeds_image, **kwds)
    except:
        control_val = kwds.get('control', None)
        return seeded_watershed(input_image, seeds_image, control=control_val, **kwds)


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
    poss_controls = ['most', 'first', 'min']
    params = ""
    if control is None:
        control = 'most'
    else:
        if control not in poss_controls:
            raise NotImplementedError("Available control methods: {}".format(poss_controls))
        else:
            params = '-labelchoice ' + str(control)

    params += _method_param_check(**kwds)
    print params
    return watershed(input_image, seeds_image, param_str_2=params)
