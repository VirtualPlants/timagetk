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
This module contain a generic implementation of labels post processing algorithms.
"""

import warnings

try:
    from timagetk.algorithms import cell_filter
    from timagetk.components import SpatialImage
except ImportError:
    raise ImportError('Import Error')

__all__ = ['labels_post_processing']

DEFAULT_METHOD = 'labels_erosion'
POSS_METHODS = ['labels_erosion', 'labels_dilation', 'labels_opening', 'labels_closing']


def _method_img_check(input_image):
    """
    Used to check `input_image` type and isometry.
    """
    # - Check the `input_image` is indeed a `SpatialImage`
    conds = isinstance(input_image, SpatialImage)
    if not conds:
        raise TypeError('Input image must be a `SpatialImage` object.')
    # - Check the isometry of the image:
    vx, vy, vz = input_image.get_voxelsize()
    if (vx != vy) or (vy != vz):
        warnings.warn("The image is NOT isometric, this method operates on voxels!!")
    return


def _method_param_check(radius, iterations):
    """
    Set parameters default values and make sure they are of the right type.
    """
    if radius is None:
        radius = 1
    else:
        radius = abs(int(radius))
    if iterations is None:
        iterations = 1
    else:
        iterations = abs(int(iterations))
    return radius, iterations


def labels_post_processing(input_image, method=None, **kwds):
    """
    Labels post-processing algorithms. Available methods are :

    * labels_erosion
    * labels_dilation
    * labels_opening
    * labels_closing

    Parameters
    ----------
    :param *SpatialImage* input_image: input *SpatialImage*

    :param str method: used method (example: 'labels_erosion')

    Returns
    ----------
    :return: ``SpatialImage`` instance -- image and metadata

    :WARNING: If your images are not isometric, the morphological 
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
    # - Set the 'DEFAULT_METHOD' if needed:
    if method is None:
        method = DEFAULT_METHOD
    # - Check the provided method is implemented:
    try:
        assert method in POSS_METHODS
    except:
        print('Available methods :'), POSS_METHODS
        raise NotImplementedError(method)

    try:
        from openalea.core.service.plugin import plugin_function
        func = plugin_function('openalea.image', method)
        if func is not None:
            return func(input_image, **kwds)
    except:
        print "Plugin functionnality not available !"
        radius_val = kwds.get('radius', None)
        it_val = kwds.get('iterations', None)
        if method=='labels_erosion':
            return labels_erosion(input_image, radius=radius_val, iterations=it_val)
        if method=='labels_dilation':
            return labels_dilation(input_image, radius=radius_val, iterations=it_val)
        if method=='labels_opening':
            return labels_opening(input_image, radius=radius_val, iterations=it_val)
        if method=='labels_closing':
            return labels_closing(input_image, radius=radius_val, iterations=it_val)


def labels_erosion(input_image, radius=None, iterations=None, **kwds):
    """
    Labels erosion.

    Parameters
    ----------
    :param *SpatialImage* input_image: input *SpatialImage*

    :param int radius: optinal, radius. Default: radius=1

    :param int iterations: optional, number of iterations. Default: iterations=1

    Returns
    ----------
    :return: ``SpatialImage`` instance -- image and metadata
    """
    _method_img_check(input_image)
    radius, iterations = _method_param_check(radius, iterations)
    params = '-erosion -iterations %d -radius %d' % (iterations, radius)
    return cell_filter(input_image, param_str_2=params)


def labels_dilation(input_image, radius=None, iterations=None, **kwds):
    """
    Labels dilation.

    Parameters
    ----------
    :param *SpatialImage* input_image: input *SpatialImage*

    :param int radius: optinal, radius. Default: radius=1

    :param int iterations: optional, number of iterations. Default: iterations=1

    Returns
    ----------
    :return: ``SpatialImage`` instance -- image and metadata
    """
    _method_img_check(input_image)
    radius, iterations = _method_param_check(radius, iterations)
    params = '-dilation -iterations %d -radius %d' % (iterations, radius)
    return cell_filter(input_image, param_str_2=params)


def labels_opening(input_image, radius=None, iterations=None, **kwds):
    """
    Labels opening.

    Parameters
    ----------
    :param *SpatialImage* input_image: input *SpatialImage*

    :param int radius: optinal, radius. Default: radius=1

    :param int iterations: optional, number of iterations. Default: iterations=1

    Returns
    ----------
    :return: ``SpatialImage`` instance -- image and metadata
    """
    _method_img_check(input_image)
    radius, iterations = _method_param_check(radius, iterations)
    params = '-opening -iterations %d -radius %d' % (iterations, radius)
    return cell_filter(input_image, param_str_2=params)


def labels_closing(input_image, radius=None, iterations=None, **kwds):
    """
    Labels closing.

    Parameters
    ----------
    :param *SpatialImage* input_image: input *SpatialImage*

    :param int radius: optinal, radius. Default: radius=1

    :param int iterations: optional, number of iterations. Default: iterations=1

    Returns
    ----------
    :return: ``SpatialImage`` instance -- image and metadata
    """
    radius, iterations = _method_param_check(radius, iterations)
    params = '-closing -iterations %d -radius %d' % (iterations, radius)
    return cell_filter(input_image, param_str_2=params)
