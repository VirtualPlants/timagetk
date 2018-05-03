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
This module contain a generic implementation of several cell filtering algorithms.
"""

try:
    from timagetk.algorithms import cell_filter
    from timagetk.components import SpatialImage
except ImportError:
    raise ImportError('Import Error')

__all__ = ['cell_filtering']

POSS_METHODS = ['erosion', 'dilation', 'opening', 'closing', 'morpho_gradient',
                'contrast',
                'hat_transform', 'inverse_hat_transform',
                'oc_alternate_sequential_filter',
                'co_alternate_sequential_filter',
                'coc_alternate_sequential_filter',
                'oco_alternate_sequential_filter']
DEFAULT_METHOD = POSS_METHODS[0]


def cell_filtering(input_image, method=None, **kwds):
    """
    Grayscale cell_filtering plugin. Available methods are :

    * erosion
    * dilation
    * opening
    * closing
    * morpho_gradient
    * contrast
    * hat_transform
    * inverse_hat_transform
    * oc_alternate_sequential_filter
    * co_alternate_sequential_filter
    * coc_alternate_sequential_filter
    * oco_alternate_sequential_filter

    Parameters
    ----------
    :param *SpatialImage* input_image: input *SpatialImage*

    :param str method: used method (example: 'erosion')

    Returns
    ----------
    :return: ``SpatialImage`` instance -- image and metadata

    Example
    ----------
    >>> from timagetk.util import data, data_path
    >>> from timagetk.plugins import cell_filtering
    >>> image_path = data_path('time_0_cut.inr')
    >>> input_image = data(image_path)
    >>> dilation_image = cell_filtering(input_image, radius=2, iterations=2, method='dilation')
    >>> oc_asf_image = cell_filtering(input_image, max_radius=3, method='oc_alternate_sequential_filter')
    """
    try:
        assert isinstance(input_image, SpatialImage)
    except AssertionError:
        raise TypeError('Input image must be a SpatialImage')

    if method is None:
        method = DEFAULT_METHOD
    try:
        assert method in POSS_METHODS
    except AssertionError:
        raise NotImplementedError(
            "Unknown method '{}', available methods are: {}".format(method,
                                                                    POSS_METHODS))
    try:
        assert kwds.get('try_plugin', False)
        from openalea.core.service.plugin import plugin_function
    except AssertionError or ImportError:
        radius_val = kwds.pop('radius', None)
        it_val = kwds.pop('iterations', None)
        max_radius_val = kwds.pop('max_radius', None)
        if method == 'erosion':
            return cell_filtering_erosion(input_image, radius=radius_val,
                                          iterations=it_val, **kwds)
        elif method == 'dilation':
            return cell_filtering_dilation(input_image, radius=radius_val,
                                           iterations=it_val, **kwds)
        elif method == 'opening':
            return cell_filtering_opening(input_image, radius=radius_val,
                                          iterations=it_val, **kwds)
        elif method == 'closing':
            return cell_filtering_closing(input_image, radius=radius_val,
                                          iterations=it_val, **kwds)
        elif method == 'morpho_gradient':
            return cell_filtering_gradient(input_image, radius=radius_val,
                                           iterations=it_val, **kwds)
        elif method == 'contrast':
            return cell_filtering_contrast(input_image, radius=radius_val,
                                           iterations=it_val, **kwds)
        elif method == 'hat_transform':
            return cell_filtering_hat_transform(input_image, radius=radius_val,
                                                iterations=it_val, **kwds)
        elif method == 'inverse_hat_transform':
            return cell_filtering_inverse_hat_transform(input_image,
                                                        radius=radius_val,
                                                        iterations=it_val,
                                                        **kwds)
        elif method == 'oc_alternate_sequential_filter':
            return cell_filtering_oc_alternate_sequential_filter(input_image,
                                                                 max_radius=max_radius_val,
                                                                 **kwds)
        elif method == 'co_alternate_sequential_filter':
            return cell_filtering_co_alternate_sequential_filter(input_image,
                                                                 max_radius=max_radius_val,
                                                                 **kwds)
        elif method == 'coc_alternate_sequential_filter':
            return cell_filtering_coc_alternate_sequential_filter(input_image,
                                                                  max_radius=max_radius_val,
                                                                  **kwds)
        elif method == 'oco_alternate_sequential_filter':
            return cell_filtering_oco_alternate_sequential_filter(input_image,
                                                                  max_radius=max_radius_val,
                                                                  **kwds)
    else:
        func = plugin_function('openalea.image', method)
        if func is not None:
            print "WARNING: using 'plugin' functionality from 'openalea.core'!"
            return func(input_image, **kwds)
        else:
            raise NotImplementedError("Returned 'plugin_function' is None!")


def _check_radius_iterations(radius, iterations):
    """
    Performs 'radius' and 'iterations' transformation or set to default value.

    Parameters
    ----------
    radius : int|None
        value of the radius to use when creating the structuring element
    iterations : int|None
         number of iterations to performs

    Returns
    -------
    radius, iterations
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


def cell_filtering_erosion(input_image, radius=None, iterations=None, **kwds):
    """
    Erosion

    Parameters
    ----------
    :param *SpatialImage* input_image: input *SpatialImage*

    :param int radius: optional, radius. Default: radius=1

    :param int iterations: optional, number of iterations. Default: iterations=1

    Returns
    ----------
    :return: ``SpatialImage`` instance -- image and metadata
    """
    try:
        assert isinstance(input_image, SpatialImage)
    except AssertionError:
        raise TypeError('Input image must be a SpatialImage')

    radius, iterations = _check_radius_iterations(radius, iterations)
    params = '-operation erosion -iterations %d -radius %d -v' % (
        iterations, radius)
    return cell_filter(input_image, param_str_2=params)


def cell_filtering_dilation(input_image, radius=None, iterations=None, **kwds):
    """
    Dilation

    Parameters
    ----------
    :param *SpatialImage* input_image: input *SpatialImage*

    :param int radius: optional, radius. Default: radius=1

    :param int iterations: optional, number of iterations. Default: iterations=1

    Returns
    ----------
    :return: ``SpatialImage`` instance -- image and metadata
    """
    try:
        assert isinstance(input_image, SpatialImage)
    except AssertionError:
        raise TypeError('Input image must be a SpatialImage')

    radius, iterations = _check_radius_iterations(radius, iterations)
    params = '-dilation -iterations %d -radius %d' % (iterations, radius)
    return cell_filter(input_image, param_str_2=params)


def cell_filtering_opening(input_image, radius=None, iterations=None, **kwds):
    """
    Opening

    Parameters
    ----------
    :param *SpatialImage* input_image: input *SpatialImage*

    :param int radius: optional, radius. Default: radius=1

    :param int iterations: optional, number of iterations. Default: iterations=1

    Returns
    ----------
    :return: ``SpatialImage`` instance -- image and metadata
    """
    try:
        assert isinstance(input_image, SpatialImage)
    except AssertionError:
        raise TypeError('Input image must be a SpatialImage')

    radius, iterations = _check_radius_iterations(radius, iterations)
    params = '-operation opening -iterations %d -radius %d' % (
        iterations, radius)
    return cell_filter(input_image, param_str_2=params)


def cell_filtering_closing(input_image, radius=None, iterations=None, **kwds):
    """
    Closing

    Parameters
    ----------
    :param *SpatialImage* input_image: input *SpatialImage*

    :param int radius: optional, radius. Default: radius=1

    :param int iterations: optional, number of iterations. Default: iterations=1

    Returns
    ----------
    :return: ``SpatialImage`` instance -- image and metadata
    """
    try:
        assert isinstance(input_image, SpatialImage)
    except AssertionError:
        raise TypeError('Input image must be a SpatialImage')

    radius, iterations = _check_radius_iterations(radius, iterations)
    params = '-operation closing -iterations %d -radius %d' % (
        iterations, radius)
    return cell_filter(input_image, param_str_2=params)


def cell_filtering_hat_transform(input_image, radius=None, iterations=None,
                                 **kwds):
    """
    Hat transform

    Parameters
    ----------
    :param *SpatialImage* input_image: input *SpatialImage*

    :param int radius: optional, radius. Default: radius=1

    :param int iterations: optional, number of iterations. Default: iterations=1

    Returns
    ----------
    :return: ``SpatialImage`` instance -- image and metadata
    """
    try:
        assert isinstance(input_image, SpatialImage)
    except AssertionError:
        raise TypeError('Input image must be a SpatialImage')

    radius, iterations = _check_radius_iterations(radius, iterations)
    params = '-operation closinghat -iterations %d -radius %d' % (
        iterations, radius)
    return cell_filter(input_image, param_str_2=params)


def cell_filtering_inverse_hat_transform(input_image, radius=None,
                                         iterations=None, **kwds):
    """
    Inverse hat transform

    Parameters
    ----------
    :param *SpatialImage* input_image: input *SpatialImage*

    :param int radius: optional, radius. Default: radius=1

    :param int iterations: optional, number of iterations. Default: iterations=1

    Returns
    ----------
    :return: ``SpatialImage`` instance -- image and metadata
    """
    try:
        assert isinstance(input_image, SpatialImage)
    except AssertionError:
        raise TypeError('Input image must be a SpatialImage')

    radius, iterations = _check_radius_iterations(radius, iterations)
    params = '-operation openinghat -iterations %d -radius %d' % (
        iterations, radius)
    return cell_filter(input_image, param_str_2=params)


def cell_filtering_gradient(input_image, radius=None, iterations=None, **kwds):
    """
    Morphological gradient

    Parameters
    ----------
    :param *SpatialImage* input_image: input *SpatialImage*

    :param int radius: optional, radius. Default: radius=1

    :param int iterations: optional, number of iterations. Default: iterations=1

    Returns
    ----------
    :return: ``SpatialImage`` instance -- image and metadata
    """
    try:
        assert isinstance(input_image, SpatialImage)
    except AssertionError:
        raise TypeError('Input image must be a SpatialImage')

    radius, iterations = _check_radius_iterations(radius, iterations)
    params = '-operation gradient -iterations %d -radius %d' % (
        iterations, radius)
    return cell_filter(input_image, param_str_2=params)


def cell_filtering_contrast(input_image, radius=None, iterations=None, **kwds):
    """
    Contrast enhancement

    Parameters
    ----------
    :param *SpatialImage* input_image: input *SpatialImage*

    :param int radius: optional, radius. Default: radius=1

    :param int iterations: optional, number of iterations. Default: iterations=1

    Returns
    ----------
    :return: ``SpatialImage`` instance -- image and metadata
    """
    try:
        assert isinstance(input_image, SpatialImage)
    except AssertionError:
        raise TypeError('Input image must be a SpatialImage')

    radius, iterations = _check_radius_iterations(radius, iterations)
    params = '-operation contrast -iterations %d -radius %d' % (
        iterations, radius)
    return cell_filter(input_image, param_str_2=params)


def cell_filtering_oc_alternate_sequential_filter(input_image, max_radius=None,
                                                  **kwds):
    """
    Opening Closing alternate sequential filter

    Parameters
    ----------
    :param SpatialImage input_image: input *SpatialImage*

    :param int max_radius: max radius

    Returns
    ----------
    :return: *SpatialImage* instance -- image and associated information
    """
    try:
        assert isinstance(input_image, SpatialImage)
    except AssertionError:
        raise TypeError('Input image must be a SpatialImage')

    if max_radius is None:
        max_radius = 1
        output_img = cell_filtering_oc_alternate_sequential_filter(input_image,
                                                                   max_radius)
        return output_img
    else:
        max_radius = abs(int(max_radius))
        sizes = range(1, max_radius + 1)
        output_img = input_image
        for size in sizes:
            param_str_2 = "-operation closing -R " + str(size)
            output_img = cell_filter(output_img, param_str_2=param_str_2)
            param_str_2 = "-operation opening -R " + str(size)
            output_img = cell_filter(output_img, param_str_2=param_str_2)
        return output_img


def cell_filtering_co_alternate_sequential_filter(input_image, max_radius=None,
                                                  **kwds):
    """
    Closing Opening alternate sequential filter

    Parameters
    ----------
    :param SpatialImage input_image: input *SpatialImage*

    :param int max_radius: max radius

    Returns
    ----------
    :return: *SpatialImage* instance -- image and associated information
    """
    try:
        assert isinstance(input_image, SpatialImage)
    except AssertionError:
        raise TypeError('Input image must be a SpatialImage')

    if max_radius is None:
        max_radius = 1
        output_img = cell_filtering_co_alternate_sequential_filter(input_image,
                                                                   max_radius)
        return output_img
    else:
        max_radius = abs(int(max_radius))
        sizes = range(1, max_radius + 1)
        output_img = input_image
        for size in sizes:
            param_str_2 = "-operation opening -R " + str(size)
            output_img = cell_filter(output_img, param_str_2=param_str_2)
            param_str_2 = "-operation closing -R " + str(size)
            output_img = cell_filter(output_img, param_str_2=param_str_2)
        return output_img


def cell_filtering_coc_alternate_sequential_filter(input_image, max_radius=None,
                                                   **kwds):
    """
    Closing Opening Closing alternate sequential filter

    Parameters
    ----------
    :param SpatialImage input_image: input *SpatialImage*

    :param int max_radius: max radius

    Returns
    ----------
    :return: *SpatialImage* instance -- image and associated information
    """
    try:
        assert isinstance(input_image, SpatialImage)
    except AssertionError:
        raise TypeError('Input image must be a SpatialImage')

    if max_radius is None:
        max_radius = 1
        output_img = cell_filtering_coc_alternate_sequential_filter(input_image,
                                                                    max_radius)
        return output_img
    else:
        max_radius = abs(int(max_radius))
        sizes = range(1, max_radius + 1)
        output_img = input_image
        for size in sizes:
            param_str_2 = "-operation closing -R " + str(size)
            output_img = cell_filter(output_img, param_str_2=param_str_2)
            param_str_2 = "-operation opening -R " + str(size)
            output_img = cell_filter(output_img, param_str_2=param_str_2)
            param_str_2 = "-operation closing -R " + str(size)
            output_img = cell_filter(output_img, param_str_2=param_str_2)
        return output_img


def cell_filtering_oco_alternate_sequential_filter(input_image, max_radius=None,
                                                   **kwds):
    """
    Opening Closing Opening alternate sequential filter

    Parameters
    ----------
    :param SpatialImage input_image: input *SpatialImage*

    :param int max_radius: max radius

    Returns
    ----------
    :return: *SpatialImage* instance -- image and associated information
    """
    try:
        assert isinstance(input_image, SpatialImage)
    except AssertionError:
        raise TypeError('Input image must be a SpatialImage')

    if max_radius is None:
        max_radius = 1
        output_img = cell_filtering_oco_alternate_sequential_filter(input_image,
                                                                    max_radius)
        return output_img
    else:
        max_radius = abs(int(max_radius))
        sizes = range(1, max_radius + 1)
        output_img = input_image
        for size in sizes:
            param_str_2 = "-operation opening -R " + str(size)
            output_img = cell_filter(output_img, param_str_2=param_str_2)
            param_str_2 = "-operation closing -R " + str(size)
            output_img = cell_filter(output_img, param_str_2=param_str_2)
            param_str_2 = "-operation opening -R " + str(size)
            output_img = cell_filter(output_img, param_str_2=param_str_2)
        return output_img
