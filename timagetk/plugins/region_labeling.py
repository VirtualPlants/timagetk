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
This module contain implementation of region labeling algorithms
"""

import numpy as np

try:
    from timagetk.util import _input_img_check
    from timagetk.util import _method_check
    from timagetk.util import _general_kwargs
    from timagetk.util import _parallel_kwargs
    from timagetk.algorithms import connexe
    from timagetk.components.spatial_image import SpatialImage
except ImportError as e:
    raise ImportError('Import Error: {}'.format(e))

__all__ = ['region_labeling']

POSS_METHODS = ['connected_components']
DEFAULT_METHOD = 0  # index of the default method in POSS_METHODS


def region_labeling(input_image, method=None, **kwargs):
    """
    Region labeling plugin.
    Available methods are:
      * connected_components

    Parameters
    ----------
    input_image : SpatialImage
         input image to transform
    method: str, optional
        used method, by default 'erosion'

    **kwargs
    ------
    low_threshold : int, optional
        low threshold
    high_threshold : int, optional
        high threshold

    Returns
    ----------
    SpatialImage
         transformed image with its metadata

    Example
    ----------
    >>> from timagetk.util import data_path
    >>> from timagetk.components import imread
    >>> from timagetk.plugins import h_transform, region_labeling
    >>> image_path = data_path('time_0_cut.inr')
    >>> input_image = imread(image_path)
    >>> reg_min_image = h_transform(input_image, h=3, method='h_transform_min')
    >>> reg_lab_image = region_labeling(reg_min_image, low_threshold=1, high_threshold=3, method='connected_components')
    """
    # - Assert the 'input_image' is a SpatialImage instance:
    _input_img_check(input_image)
    # - Set method if None and check it is a valid method:
    method = _method_check(method, POSS_METHODS, DEFAULT_METHOD)

    try:
        assert kwargs.get('try_plugin', False)
        from openalea.core.service.plugin import plugin_function
    except AssertionError or ImportError:
        if method == 'connected_components':
            low_threshold_val = kwargs.pop('low_threshold', None)
            high_threshold_val = kwargs.pop('high_threshold', None)
            return connected_components(input_image,
                                        low_threshold=low_threshold_val,
                                        high_threshold=high_threshold_val,
                                        **kwargs)
    else:
        func = plugin_function('openalea.image', method)
        if func is not None:
            print "WARNING: using 'plugin' functionality from 'openalea.core'!"
            return func(input_image, **kwargs)
        else:
            raise NotImplementedError("Returned 'plugin_function' is None!")


def connected_components(input_image, low_threshold=None, high_threshold=None,
                         **kwargs):
    """
    Connected components detection and labelling.

    Parameters
    ----------
    input_image : SpatialImage
        input image to transform
    low_threshold : int, optional
        low threshold
    high_threshold : int, optional
        high threshold

    Returns
    ----------
    SpatialImage: image and metadata
    """
    if low_threshold is None:
        low_threshold = 1
    else:
        low_threshold = abs(int(low_threshold))
    if high_threshold is None:
        high_threshold = 3
    else:
        high_threshold = abs(int(high_threshold))

    params = '-low-threshold %d -high-threshold %d -label-output' % (
        low_threshold, high_threshold)

    # - Parse general kwargs:
    params += _general_kwargs(**kwargs)
    # - Parse parallelism kwargs:
    params += _parallel_kwargs(**kwargs)

    return connexe(input_image, param_str_2=params, dtype=np.uint16)
