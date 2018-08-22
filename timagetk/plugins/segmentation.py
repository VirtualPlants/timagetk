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
This module contains a generic implementation of several segmentation algorithms.
"""

try:
    from timagetk.util import _input_img_check
    from timagetk.util import _method_check
    from timagetk.util import _general_kwargs
    from timagetk.util import _parallel_kwargs
    from timagetk.algorithms import watershed
    from timagetk.components.spatial_image import SpatialImage
except ImportError as e:
    raise ImportError('Import Error: {}'.format(e))

__all__ = ['segmentation', 'seeded_watershed']

POSS_METHODS = ['seeded_watershed']
DEFAULT_METHOD = 0  # index of the default method in POSS_METHODS
POSS_CONTROLS = ['most', 'first', 'min']
DEFAULT_CONTROL = 0


def _seg_kwargs(**kwargs):
    """
    Set parameters default values and make sure they are of the right type.
    """
    str_param = ""

    # - Parse general kwargs:
    str_param += _general_kwargs(**kwargs)
    # - Parse parallelism kwargs:
    str_param += _parallel_kwargs(**kwargs)

    return str_param


def segmentation(input_image, seeds_image, method=None, **kwargs):
    """
    Segmentation plugin.
    Available methods are:
      * seeded_watershed

    Parameters
    ----------
    input_image : SpatialImage
         input image to transform
    seeds_image : SpatialImage
         seed image to use for initialisation of watershed algorithm
    method: str, optional
        used method, by default 'seeded_watershed'

    Returns
    -------
    SpatialImage
         transformed image with its metadata

    Example
    -------
    >>> from timagetk.util import data_path
    >>> from timagetk.components import imread
    >>> from timagetk.plugins import linear_filtering, h_transform, region_labeling, segmentation
    >>> image_path = data_path('segmentation_src.inr')
    >>> input_image = imread(image_path)
    >>> smooth_image = linear_filtering(input_image, std_dev=2.0, method='gaussian_smoothing')
    >>> regext_image = h_transform(smooth_image, h=5, method='h_transform_min')
    >>> seeds_image = region_labeling(regext_image, low_threshold=1, high_threshold=3, method='connected_components')
    >>> segmented_image = segmentation(smooth_image, seeds_image, control='first', method='seeded_watershed')
    """
    # - Assert the 'input_image' is a SpatialImage instance:
    _input_img_check(input_image)
    # - Assert the 'seeds_image' is a SpatialImage instance:
    _input_img_check(seeds_image)
    # - Set method if None and check it is a valid method:
    method = _method_check(method, POSS_METHODS, DEFAULT_METHOD)

    try:
        assert kwargs.get('try_plugin', False)
        from openalea.core.service.plugin import plugin_function
    except AssertionError or ImportError:
        control_val = kwargs.pop('control', None)
        return seeded_watershed(input_image, seeds_image, control=control_val,
                                **kwargs)
    else:
        func = plugin_function('openalea.image', method)
        if func is not None:
            print "WARNING: using 'plugin' functionality from 'openalea.core'!"
            return func(input_image, seeds_image, **kwargs)
        else:
            raise NotImplementedError("Returned 'plugin_function' is None!")


def seeded_watershed(input_image, seeds_image, control=None, **kwargs):
    """
    Seeded watershed algorithm

    Parameters
    ----------
    input_image : SpatialImage
         input image to transform
    seeds_image : SpatialImage
         seed image to use for initialisation of watershed algorithm
    control: str, optional
        used control to deal with watershed conflicts, default='most', ie. the
        most represented label

    Returns
    ----------
    SpatialImage
         transformed image with its metadata
    """
    # - Set method if None and check it is a valid method:
    control = _method_check(control, POSS_CONTROLS, DEFAULT_CONTROL)
    params = '-labelchoice ' + str(control)
    params += _seg_kwargs(**kwargs)
    return watershed(input_image, seeds_image, param_str_2=params)
