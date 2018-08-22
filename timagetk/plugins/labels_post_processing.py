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
This module contain a generic implementation of labels post processing algorithms.
"""

import warnings

try:
    from timagetk.util import _input_img_check
    from timagetk.util import _method_check
    from timagetk.util import _general_kwargs
    from timagetk.util import _parallel_kwargs
    from timagetk.algorithms import cell_filter
    from timagetk.components import SpatialImage
except ImportError as e:
    raise ImportError('Import Error: {}'.format(e))

__all__ = ['labels_post_processing']

POSS_METHODS = ['labels_erosion', 'labels_dilation', 'labels_opening',
                'labels_closing']
DEFAULT_METHOD = 0  # index of the default method in POSS_METHODS
DEF_RADIUS = 1  # default radius of the structuring elements
DEF_ITERS = 1  # default number of iterations during morphological operations
DEF_CONNECT = 18  # default connectivity during morphological operations


def _lpp_kwargs(**kwargs):
    """
    Set parameters default values and make sure they are of the right type.
    """
    str_param = ""
    # - By default 'radius' is equal to 1:
    radius = abs(int(kwargs.get('radius', DEF_RADIUS)))
    str_param += ' -radius %d' % radius
    # - By default 'iterations' is equal to 1:
    iterations = abs(int(kwargs.get('iterations', DEF_ITERS)))
    str_param += ' -iterations %d' % iterations
    # - By default 'connectivity' is equal to 18:
    connectivity = abs(int(kwargs.get('connectivity', DEF_CONNECT)))
    str_param += ' -connectivity %d' % connectivity

    # - Parse general kwargs:
    str_param += _general_kwargs(**kwargs)
    # - Parse parallelism kwargs:
    str_param += _parallel_kwargs(**kwargs)

    return str_param


def labels_post_processing(input_image, method=None, **kwargs):
    """
    Labels post-processing algorithms.
    Available methods are:
      * labels_erosion
      * labels_dilation
      * labels_opening
      * labels_closing

    Most parameters works only if 'try_plugin=False' is given!

    Parameters
    ----------
    input_image: *SpatialImage*
        input *SpatialImage*
    method: str
        used method (example: 'labels_erosion')

    **kwargs
    ------
    radius: int, optional; default = 1
        radius of the structuring element
    iterations: int, optional; default = 1
        number of iteration of the morphological operation
    connectivity: value in 4|8|10|18|26, default = 18
        structuring element is among the 6-, 10-, 18- & 26-neighborhoods.

    try_plugin: bool, optional; default = True
        manually control the use of openalea 'plugin' functionality

    Returns
    ----------
    SpatialImage
         transformed image with its metadata

    *WARNING*: If your images are not isometric, the morphological
     operation will not be the same in every directions!

    Example
    ----------
    >>> from timagetk.util import data_path
    >>> from timagetk.components import imread
    >>> from timagetk.plugins import labels_post_processing
    >>> image_path = data_path('segmentation_seeded_watershed.inr')
    >>> segmented_image = imread(image_path)
    >>> ero_image = labels_post_processing(segmented_image, radius=2, iterations=1, method='labels_erosion')
    >>> open_image = labels_post_processing(segmented_image, method='labels_opening')
    """
    # - Check `input_image` type and isometry:
    _input_img_check(input_image)
    # - Set method if None and check it is a valid method:
    method = _method_check(method, POSS_METHODS, DEFAULT_METHOD)

    # - Try 'plugin function' or use direct wrapping:
    try:
        assert kwargs.get('try_plugin', False)
        from openalea.core.service.plugin import plugin_function
    except AssertionError or ImportError:
        print "Plugin functionnality not available !"
        if method == 'labels_erosion':
            return labels_erosion(input_image, **kwargs)
        if method == 'labels_dilation':
            return labels_dilation(input_image, **kwargs)
        if method == 'labels_opening':
            return labels_opening(input_image, **kwargs)
        if method == 'labels_closing':
            return labels_closing(input_image, **kwargs)
    else:
        func = plugin_function('openalea.image', method)
        if func is not None:
            print "WARNING: using 'plugin' functionality from 'openalea.core'!"
            return func(input_image, **kwargs)
        else:
            raise NotImplementedError("Returned 'plugin_function' is None!")


def labels_erosion(input_image, radius=DEF_RADIUS, iterations=DEF_ITERS,
                   **kwargs):
    """
    Labels erosion.

    Parameters
    ----------
    input_image: *SpatialImage*
        input labelled SpatialImage to erode

    **kwargs
    ------
    radius: int, optional
        radius of the structuring element; default = 1
    iterations: int, optional
        number of iteration of the morphological operation; default = 1

    Returns
    ----------
    SpatialImage
         transformed image with its metadata
    """
    params = '-erosion'
    params += _lpp_kwargs(radius=radius, iterations=iterations, **kwargs)
    return cell_filter(input_image, param_str_2=params)


def labels_dilation(input_image, radius=DEF_RADIUS, iterations=DEF_ITERS,
                    **kwargs):
    """
    Labels dilation.

    Parameters
    ----------
    input_image : SpatialImage
        input labelled SpatialImage to dilate

    **kwargs
    ------
    radius: int, optional
        radius of the structuring element; default = 1
    iterations: int, optional
        number of iteration of the morphological operation; default = 1

    Returns
    ----------
    SpatialImage
         transformed image with its metadata
    """
    params = '-dilation'
    params += _lpp_kwargs(radius=radius, iterations=iterations, **kwargs)
    return cell_filter(input_image, param_str_2=params)


def labels_opening(input_image, radius=DEF_RADIUS, iterations=DEF_ITERS,
                   **kwargs):
    """
    Labels opening.

    Parameters
    ----------
    input_image: *SpatialImage*
        input *SpatialImage*

    **kwargs
    ------
    radius: int, optional
        radius of the structuring element; default = 1
    iterations: int, optional
        number of iteration of the morphological operation; default = 1

    Returns
    ----------
    SpatialImage
         transformed image with its metadata
    """
    params = '-opening'
    params += _lpp_kwargs(radius=radius, iterations=iterations, **kwargs)
    return cell_filter(input_image, param_str_2=params)


def labels_closing(input_image, radius=DEF_RADIUS, iterations=DEF_ITERS,
                   **kwargs):
    """
    Labels closing.

    Parameters
    ----------
    input_image: *SpatialImage*
        input *SpatialImage*

    **kwargs
    ------
    radius: int, optional
        radius of the structuring element; default = 1
    iterations: int, optional
        number of iteration of the morphological operation; default = 1

    Returns
    ----------
    SpatialImage
         transformed image with its metadata
    """
    params = '-closing'
    params += _lpp_kwargs(radius=radius, iterations=iterations, **kwargs)
    return cell_filter(input_image, param_str_2=params)
