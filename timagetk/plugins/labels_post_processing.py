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

try:
    from timagetk.algorithms import cell_filter
    from timagetk.components import SpatialImage
except ImportError:
    raise ImportError('Import Error')

__all__ = ['labels_post_processing']


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

    poss_methods = ['labels_erosion', 'labels_dilation', 'labels_opening', 'labels_closing']
    conds = isinstance(input_image, SpatialImage)
    if conds:
        if method is None:
            return labels_erosion(input_image)
        elif method is not None:
            if method in poss_methods:
                try:
                    from openalea.core.service.plugin import plugin_function
                    func = plugin_function('openalea.image', method)
                    if func is not None:
                        return func(input_image, **kwds)
                except:
                    radius_val = kwds.get('radius', None)
                    it_val = kwds.get('iterations', None)
                    if method=='labels_erosion':
                        return labels_erosion(input_image, radius=radius_val, iterations=it_val)
                    elif method=='labels_dilation':
                        return labels_dilation(input_image, radius=radius_val, iterations=it_val)
                    elif method=='labels_opening':
                        return labels_opening(input_image, radius=radius_val, iterations=it_val)
                    elif method=='labels_closing':
                        return labels_closing(input_image, radius=radius_val, iterations=it_val)
            else:
                print('Available methods :'), poss_methods
                raise NotImplementedError(method)
    else:
        raise TypeError('Input image must be a SpatialImage')
        return


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
    conds = isinstance(input_image, SpatialImage)
    if conds:
        if radius is None:
            radius = 1
        elif radius is not None:
            radius = abs(int(radius))
        if iterations is None:
            iterations = 1
        elif iterations is not None:
            iterations = abs(int(iterations))
        params = '-erosion -iterations %d -radius %d' % (iterations, radius)
        return cell_filter(input_image, param_str_2=params)
    else:
        raise TypeError('Input image must be a SpatialImage')
        return


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
    conds = isinstance(input_image, SpatialImage)
    if conds:
        if radius is None:
            radius = 1
        elif radius is not None:
            radius = abs(int(radius))
        if iterations is None:
            iterations = 1
        elif iterations is not None:
            iterations = abs(int(iterations))
        params = '-dilation -iterations %d -radius %d' % (iterations, radius)
        return cell_filter(input_image, param_str_2=params)
    else:
        raise TypeError('Input image must be a SpatialImage')
        return


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
    conds = isinstance(input_image, SpatialImage)
    if conds:
        if radius is None:
            radius = 1
        elif radius is not None:
            radius = abs(int(radius))
        if iterations is None:
            iterations = 1
        elif iterations is not None:
            iterations = abs(int(iterations))
        params = '-opening -iterations %d -radius %d' % (iterations, radius)
        return cell_filter(input_image, param_str_2=params)
    else:
        raise TypeError('Input image must be a SpatialImage')
        return


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
    conds = isinstance(input_image, SpatialImage)
    if conds:
        if radius is None:
            radius = 1
        elif radius is not None:
            radius = abs(int(radius))
        if iterations is None:
            iterations = 1
        elif iterations is not None:
            iterations = abs(int(iterations))
        params = '-closing -iterations %d -radius %d' % (iterations, radius)
        return cell_filter(input_image, param_str_2=params)
    else:
        raise TypeError('Input image must be a SpatialImage')
        return