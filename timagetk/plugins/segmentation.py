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


__all__ = ['segmentation']


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
    conds = isinstance(input_image, SpatialImage) and isinstance(seeds_image, SpatialImage)
    if conds:
        if method is None:
            return seeded_watershed(input_image, seeds_image)
        elif method is not None:
            if method in poss_methods:
                try:
                    from openalea.core.service.plugin import plugin_function
                    func = plugin_function('openalea.image', method)
                    if func is not None:
                        return func(input_image, seeds_image, **kwds)
                except:
                    control_val = kwds.get('control', None)
                    return seeded_watershed(input_image, seeds_image, control=control_val)
            else:
                print('Available methods :'), poss_methods
                raise NotImplementedError(method)
    else:
        raise TypeError('Input image and seeds image must be a SpatialImage')


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
    conds = isinstance(input_image, SpatialImage) and isinstance(seeds_image, SpatialImage)
    if conds:
        if control is None:
            control = 'most'
        elif control is not None:
            if control not in poss_controls:
                control = 'most'
        params = '-labelchoice ' + str(control)
        return watershed(input_image, seeds_image, param_str_2=params)
    else:
        raise TypeError('Input images must be a SpatialImage')
