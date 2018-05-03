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
This module contain a generic implementation of labels post processing algorithms.
"""

import warnings

try:
    from timagetk.algorithms import cell_filter
    from timagetk.components import SpatialImage
except ImportError:
    raise ImportError('Import Error')

__all__ = ['labels_post_processing']

POSS_METHODS = ['labels_erosion', 'labels_dilation', 'labels_opening',
                'labels_closing']
DEFAULT_METHOD = POSS_METHODS[0]


def _method_img_check(input_image):
    """
    Used to check `input_image` type and isometry.
    """
    # - Check the `input_image` is indeed a `SpatialImage`
    try:
        assert isinstance(input_image, SpatialImage)
    except AssertionError:
        raise TypeError('Input image must be a SpatialImage')
    # - Check the isometry of the image:
    vx, vy, vz = input_image.get_voxelsize()
    if (vx != vy) or (vy != vz):
        warnings.warn(
            "The image is NOT isometric, this method operates on voxels!!")
    return


def get_param_str_2(**kwds):
    """
    Set parameters default values and make sure they are of the right type.
    """
    str_param = ""
    str_param += ' -radius %d' % (kwds.get('radius', 1))
    str_param += ' -iterations %d' % (kwds.get('iterations', 1))
    str_param += ' -connectivity %d' % (kwds.get('connectivity', 18))
    if kwds.get('param', True):
        str_param += ' -param'
    if kwds.get('verbose', True):
        str_param += ' -verbose'
    if kwds.get('parallel', True):
        str_param += ' -parallel'
        str_param += ' -parallel-type ' + kwds.get('parallel_type', 'thread')
    if kwds.get('time', True):
        str_param += ' -time'
    return str_param


def labels_post_processing(input_image, method, **kwds):
    """
    Labels post-processing algorithms. Available methods are :
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
    radius: int, optional; default = 1
        radius of the structuring element
    iterations: int, optional; default = 1
        number of iteration of the morphological operation
    connectivity: value in 6|10|18|26, default = 18
        structuring element is among the 6-, 10-, 18- & 26-neighborhoods.
    param: bool, optional; default = False
        print parameters
    parallel: bool, optional; default = True
        set usage of parrallel mode
    parallel-type str in none|openmp|omp|pthread|thread, default=thread
        type of parrallelism to use
    time: bool, optional; default = True
        print elapsed time
    try_plugin: bool, optional; default = True
        manually control the use of openalea 'plugin' functionality

    Returns
    ----------
    :return: ``SpatialImage`` instance -- image and metadata

    *WARNING*: If your images are not isometric, the morphological
     operation will not be the same in every directions!

    Example
    ----------
    >>> from timagetk.util import data, data_path
    >>> from timagetk.util import labels_post_processing
    >>> image_path = data_path('segmentation_seeded_watershed.inr')
    >>> segmented_image = data(image_path)
    >>> ero_image = labels_post_processing(segmented_image, radius=2, iterations=1,
                                           method='labels_erosion')
    >>> open_image = labels_post_processing(segmented_image, method='labels_opening')
    """
    # - Check `input_image` type and isometry:
    _method_img_check(input_image)
    # - Check the provided method is implemented:
    if method is None:
        method = DEFAULT_METHOD
    try:
        assert method in POSS_METHODS
    except AssertionError:
        raise NotImplementedError(
            "Unknown method '{}', available methods are: {}".format(method,
                                                                    POSS_METHODS))
    # - Try 'plugin function' or use direct wrapping:
    try:
        assert kwds.get('try_plugin', False)
        from openalea.core.service.plugin import plugin_function
    except AssertionError or ImportError:
        print "Plugin functionnality not available !"
        if method == 'labels_erosion':
            return labels_erosion(input_image, **kwds)
        if method == 'labels_dilation':
            return labels_dilation(input_image, **kwds)
        if method == 'labels_opening':
            return labels_opening(input_image, **kwds)
        if method == 'labels_closing':
            return labels_closing(input_image, **kwds)
    else:
        func = plugin_function('openalea.image', method)
        if func is not None:
            print "WARNING: using 'plugin' functionality from 'openalea.core'!"
            return func(input_image, **kwds)
        else:
            raise NotImplementedError("Returned 'plugin_function' is None!")


def labels_erosion(input_image, **kwds):
    """
    Labels erosion.

    Parameters
    ----------
    input_image: *SpatialImage*
        input *SpatialImage*
    method: str
        used method (example: 'labels_erosion')
    radius: int, optional; default = 1
        radius of the structuring element
    iterations: int, optional; default = 1
        number of iteration of the morphological operation

    Returns
    ----------
    :return: ``SpatialImage`` instance -- image and metadata
    """
    _method_img_check(input_image)
    params = '-erosion'
    params += get_param_str_2(**kwds)
    return cell_filter(input_image, param_str_2=params)


def labels_dilation(input_image, **kwds):
    """
    Labels dilation.

    Parameters
    ----------
    input_image: *SpatialImage*
        input *SpatialImage*
    method: str
        used method (example: 'labels_erosion')
    radius: int, optional; default = 1
        radius of the structuring element
    iterations: int, optional; default = 1
        number of iteration of the morphological operation

    Returns
    ----------
    :return: ``SpatialImage`` instance -- image and metadata
    """
    _method_img_check(input_image)
    params = '-dilation'
    params += get_param_str_2(**kwds)
    return cell_filter(input_image, param_str_2=params)


def labels_opening(input_image, **kwds):
    """
    Labels opening.

    Parameters
    ----------
    input_image: *SpatialImage*
        input *SpatialImage*
    method: str
        used method (example: 'labels_erosion')
    radius: int, optional; default = 1
        radius of the structuring element
    iterations: int, optional; default = 1
        number of iteration of the morphological operation

    Returns
    ----------
    :return: ``SpatialImage`` instance -- image and metadata
    """
    _method_img_check(input_image)
    params = '-opening'
    params += get_param_str_2(**kwds)
    return cell_filter(input_image, param_str_2=params)


def labels_closing(input_image, **kwds):
    """
    Labels closing.

    Parameters
    ----------
    input_image: *SpatialImage*
        input *SpatialImage*
    method: str
        used method (example: 'labels_erosion')
    radius: int, optional; default = 1
        radius of the structuring element
    iterations: int, optional; default = 1
        number of iteration of the morphological operation

    Returns
    ----------
    :return: ``SpatialImage`` instance -- image and metadata
    """
    _method_img_check(input_image)
    params = '-closing'
    params += get_param_str_2(**kwds)
    return cell_filter(input_image, param_str_2=params)
