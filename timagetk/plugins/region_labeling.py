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
This module contain implementation of region labeling algorithms
"""

import numpy as np
try:
    from timagetk.algorithms import connexe
    from timagetk.components.spatial_image import SpatialImage

except ImportError:
    raise ImportError('Import Error')


__all__ = ['region_labeling']


def region_labeling(input_image, method=None, **kwds):
    """
    Region labeling plugin. Available methods are :

    * connected_components

    Parameters
    ----------
    :param *SpatialImage* input_image: input *SpatialImage*

    :param str method: used method (example: 'connected_components')

    Returns
    ----------
    :return: ``SpatialImage`` instance -- image and metadata

    Example
    ----------
    >>> from timagetk.util import data, data_path
    >>> from timagetk.plugins import h_transform, region_labeling
    >>> image_path = data_path('time_0_cut.inr')
    >>> input_image = data(image_path)
    >>> reg_min_image = h_transform(input_image, h=3, method='h_transform_min')
    >>> reg_lab_image = region_labeling(reg_min_image, low_threshold=1,
                                        high_threshold=3, method='connected_components')
    """
    poss_methods = ['connected_components']
    conds = isinstance(input_image, SpatialImage)
    if conds:
        if method is None:
            return connected_components(input_image)
        elif method is not None:
            if method in poss_methods:
                try:
                    from openalea.core.service.plugin import plugin_function
                    func = plugin_function('openalea.image', method)
                    if func is not None:
                        return func(input_image, **kwds)
                except:
                    if method=='connected_components':
                        low_threshold_val = kwds.get('low_threshold', None)
                        high_threshold_val = kwds.get('high_threshold', None)
                        return connected_components(input_image, low_threshold=low_threshold_val,
                                                    high_threshold=high_threshold_val)
            else:
                print('Available methods :'), poss_methods
                raise NotImplementedError(method)
    else:
        raise TypeError('Input image must be a SpatialImage')
        return


def connected_components(input_image, low_threshold=None, high_threshold=None, **kwds):
    """
    Connected components

    Parameters
    ----------
    :param SpatialImage input_image: input *SpatialImage*

    :param int low_threshold: low threshold

    :param int high_threshold: high threshold

    Returns
    ----------
    :return: *SpatialImage* instance -- image and associated informations
    """
    conds = isinstance(input_image, SpatialImage)
    if conds:
        if low_threshold is None:
            low_threshold = 1
        else:
            low_threshold = abs(int(low_threshold))
        if high_threshold is None:
            high_threshold = 3
        else:
            high_threshold = abs(int(high_threshold))
        params = '-low-threshold %d -high-threshold %d -label-output' % (low_threshold, high_threshold)
        return connexe(input_image, param_str_2=params, dtype=np.uint16)
    else:
        raise TypeError('Input image must be a SpatialImage')
        return