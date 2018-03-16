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
This module contain a generic implementation of several grayscale morphology algorithms.
"""

try:
    from timagetk.algorithms import morpho
    from timagetk.components import SpatialImage
except ImportError:
    raise ImportError('Import Error')

__all__ = ['morphology']

POSS_METHODS = ['erosion', 'dilation', 'opening', 'closing', 'morpho_gradient',
                'contrast', 'hat_transform', 'inverse_hat_transform',
                'oc_alternate_sequential_filter',
                'co_alternate_sequential_filter',
                'coc_alternate_sequential_filter',
                'oco_alternate_sequential_filter']
DEFAULT_METHOD = POSS_METHODS[0]


def morphology(input_image, method=None, **kwds):
    """
    Grayscale morphology plugin. Available methods are :

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
    >>> from timagetk.plugins import morphology
    >>> image_path = data_path('time_0_cut.inr')
    >>> input_image = data(image_path)
    >>> dilation_image = morphology(input_image, radius=2, iterations=2, method='dilation')
    >>> oc_asf_image = morphology(input_image, max_radius=3, method='oc_alternate_sequential_filter')
    """

    try:
        assert isinstance(input_image, SpatialImage)
    except AssertionError:
        raise TypeError('Input image must be a SpatialImage')

    if not method:
        method = DEFAULT_METHOD

    try:
        assert method in POSS_METHODS
    except AssertionError:
        print('Available methods :'), POSS_METHODS
        raise NotImplementedError(method)

    try:
        from openalea.core.service.plugin import plugin_function
        func = plugin_function('openalea.image', method)
        if func:
            return func(input_image, **kwds)
    except:
        radius_val = kwds.get('radius', None)
        it_val = kwds.get('iterations', None)
        max_radius_val = kwds.get('max_radius', None)
        if method == 'erosion':
            return morphology_erosion(input_image, radius=radius_val,
                                      iterations=it_val)
        if method == 'dilation':
            return morphology_dilation(input_image, radius=radius_val,
                                       iterations=it_val)
        if method == 'opening':
            return morphology_opening(input_image, radius=radius_val,
                                      iterations=it_val)
        if method == 'closing':
            return morphology_closing(input_image, radius=radius_val,
                                      iterations=it_val)
        if method == 'morpho_gradient':
            return morphology_gradient(input_image, radius=radius_val,
                                       iterations=it_val)
        if method == 'contrast':
            return morphology_contrast(input_image, radius=radius_val,
                                       iterations=it_val)
        if method == 'hat_transform':
            return morphology_hat_transform(input_image, radius=radius_val,
                                            iterations=it_val)
        if method == 'inverse_hat_transform':
            return morphology_inverse_hat_transform(input_image,
                                                    radius=radius_val,
                                                    iterations=it_val)
        if method == 'oc_alternate_sequential_filter':
            return morphology_oc_alternate_sequential_filter(input_image,
                                                             max_radius=max_radius_val)
        if method == 'co_alternate_sequential_filter':
            return morphology_co_alternate_sequential_filter(input_image,
                                                             max_radius=max_radius_val)
        if method == 'coc_alternate_sequential_filter':
            return morphology_coc_alternate_sequential_filter(input_image,
                                                              max_radius=max_radius_val)
        if method == 'oco_alternate_sequential_filter':
            return morphology_oco_alternate_sequential_filter(input_image,
                                                              max_radius=max_radius_val)


def morphology_erosion(input_image, radius=None, iterations=None, **kwds):
    """
    Erosion

    Parameters
    ----------
    :param *SpatialImage* input_image: input *SpatialImage*

    :param int radius: optinal, radius. Default: radius=1

    :param int iterations: optional, number of iterations. Default: iterations=1

    Returns
    ----------
    :return: ``SpatialImage`` instance -- image and metadata
    """
    try:
        assert isinstance(input_image, SpatialImage)
    except AssertionError:
        raise TypeError('Input image must be a SpatialImage')

    if not radius:
        radius = 1
    else:
        radius = abs(int(radius))
    if not iterations:
        iterations = 1
    else:
        iterations = abs(int(iterations))
    params = '-operation erosion -iterations %d -radius %d -v' % (
        iterations, radius)
    return morpho(input_image, param_str_2=params)


def morphology_dilation(input_image, radius=None, iterations=None, **kwds):
    """
    Dilation

    Parameters
    ----------
    :param *SpatialImage* input_image: input *SpatialImage*

    :param int radius: optinal, radius. Default: radius=1

    :param int iterations: optional, number of iterations. Default: iterations=1

    Returns
    ----------
    :return: ``SpatialImage`` instance -- image and metadata
    """
    try:
        assert isinstance(input_image, SpatialImage)
    except AssertionError:
        raise TypeError('Input image must be a SpatialImage')

    if not radius:
        radius = 1
    else:
        radius = abs(int(radius))
    if not iterations:
        iterations = 1
    else:
        iterations = abs(int(iterations))
    params = '-dilation -iterations %d -radius %d' % (iterations, radius)
    return morpho(input_image, param_str_2=params)


def morphology_opening(input_image, radius=None, iterations=None, **kwds):
    """
    Opening

    Parameters
    ----------
    :param *SpatialImage* input_image: input *SpatialImage*

    :param int radius: optinal, radius. Default: radius=1

    :param int iterations: optional, number of iterations. Default: iterations=1

    Returns
    ----------
    :return: ``SpatialImage`` instance -- image and metadata
    """
    try:
        assert isinstance(input_image, SpatialImage)
    except AssertionError:
        raise TypeError('Input image must be a SpatialImage')

    if not radius:
        radius = 1
    else:
        radius = abs(int(radius))
    if not iterations:
        iterations = 1
    else:
        iterations = abs(int(iterations))
    params = '-operation opening -iterations %d -radius %d' % (
        iterations, radius)
    return morpho(input_image, param_str_2=params)


def morphology_closing(input_image, radius=None, iterations=None, **kwds):
    """
    Closing

    Parameters
    ----------
    :param *SpatialImage* input_image: input *SpatialImage*

    :param int radius: optinal, radius. Default: radius=1

    :param int iterations: optional, number of iterations. Default: iterations=1

    Returns
    ----------
    :return: ``SpatialImage`` instance -- image and metadata
    """
    try:
        assert isinstance(input_image, SpatialImage)
    except AssertionError:
        raise TypeError('Input image must be a SpatialImage')

    if not radius:
        radius = 1
    else:
        radius = abs(int(radius))
    if not iterations:
        iterations = 1
    else:
        iterations = abs(int(iterations))
    params = '-operation closing -iterations %d -radius %d' % (
        iterations, radius)
    return morpho(input_image, param_str_2=params)


def morphology_hat_transform(input_image, radius=None, iterations=None, **kwds):
    """
    Hat transform

    Parameters
    ----------
    :param *SpatialImage* input_image: input *SpatialImage*

    :param int radius: optinal, radius. Default: radius=1

    :param int iterations: optional, number of iterations. Default: iterations=1

    Returns
    ----------
    :return: ``SpatialImage`` instance -- image and metadata
    """
    try:
        assert isinstance(input_image, SpatialImage)
    except AssertionError:
        raise TypeError('Input image must be a SpatialImage')

    if not radius:
        radius = 1
    else:
        radius = abs(int(radius))
    if not iterations:
        iterations = 1
    else:
        iterations = abs(int(iterations))
    params = '-operation closinghat -iterations %d -radius %d' % (
        iterations, radius)
    return morpho(input_image, param_str_2=params)


def morphology_inverse_hat_transform(input_image, radius=None, iterations=None,
                                     **kwds):
    """
    Inverse hat transform

    Parameters
    ----------
    :param *SpatialImage* input_image: input *SpatialImage*

    :param int radius: optinal, radius. Default: radius=1

    :param int iterations: optional, number of iterations. Default: iterations=1

    Returns
    ----------
    :return: ``SpatialImage`` instance -- image and metadata
    """
    try:
        assert isinstance(input_image, SpatialImage)
    except AssertionError:
        raise TypeError('Input image must be a SpatialImage')

    if not radius:
        radius = 1
    else:
        radius = abs(int(radius))
    if not iterations:
        iterations = 1
    else:
        iterations = abs(int(iterations))
    params = '-operation openinghat -iterations %d -radius %d' % (
        iterations, radius)
    return morpho(input_image, param_str_2=params)


def morphology_gradient(input_image, radius=None, iterations=None, **kwds):
    """
    Morphological gradient

    Parameters
    ----------
    :param *SpatialImage* input_image: input *SpatialImage*

    :param int radius: optinal, radius. Default: radius=1

    :param int iterations: optional, number of iterations. Default: iterations=1

    Returns
    ----------
    :return: ``SpatialImage`` instance -- image and metadata
    """
    try:
        assert isinstance(input_image, SpatialImage)
    except AssertionError:
        raise TypeError('Input image must be a SpatialImage')

    if not radius:
        radius = 1
    else:
        radius = abs(int(radius))
    if not iterations:
        iterations = 1
    else:
        iterations = abs(int(iterations))
    params = '-operation gradient -iterations %d -radius %d' % (
        iterations, radius)
    return morpho(input_image, param_str_2=params)


def morphology_contrast(input_image, radius=None, iterations=None, **kwds):
    """
    Contrast enhancement

    Parameters
    ----------
    :param *SpatialImage* input_image: input *SpatialImage*

    :param int radius: optinal, radius. Default: radius=1

    :param int iterations: optional, number of iterations. Default: iterations=1

    Returns
    ----------
    :return: ``SpatialImage`` instance -- image and metadata
    """
    try:
        assert isinstance(input_image, SpatialImage)
    except AssertionError:
        raise TypeError('Input image must be a SpatialImage')

    if not radius:
        radius = 1
    else:
        radius = int(radius)
    if not iterations:
        iterations = 1
    else:
        iterations = int(iterations)
    params = '-operation contrast -iterations %d -radius %d' % (
        iterations, radius)
    return morpho(input_image, param_str_2=params)


def morphology_oc_alternate_sequential_filter(input_image, max_radius=None,
                                              **kwds):
    """
    Opening Closing alternate sequential filter

    Parameters
    ----------
    :param SpatialImage input_image: input *SpatialImage*

    :param int max_radius: max radius

    Returns
    ----------
    :return: *SpatialImage* instance -- image and associated informations
    """
    try:
        assert isinstance(input_image, SpatialImage)
    except AssertionError:
        raise TypeError('Input image must be a SpatialImage')

    if not max_radius:
        max_radius = 1
        output_img = morphology_oc_alternate_sequential_filter(input_image,
                                                               max_radius)
        return output_img
    else:
        max_radius = abs(int(max_radius))
        sizes = range(1, max_radius + 1)
        output_img = input_image
        for size in sizes:
            param_str_2 = "-operation closing -R " + str(size)
            output_img = morpho(output_img, param_str_2=param_str_2)
            param_str_2 = "-operation opening -R " + str(size)
            output_img = morpho(output_img, param_str_2=param_str_2)
        return output_img


def morphology_co_alternate_sequential_filter(input_image, max_radius=None,
                                              **kwds):
    """
    Closing Opening alternate sequential filter

    Parameters
    ----------
    :param SpatialImage input_image: input *SpatialImage*

    :param int max_radius: max radius

    Returns
    ----------
    :return: *SpatialImage* instance -- image and associated informations
    """
    try:
        assert isinstance(input_image, SpatialImage)
    except AssertionError:
        raise TypeError('Input image must be a SpatialImage')

    if not max_radius:
        max_radius = 1
        output_img = morphology_co_alternate_sequential_filter(input_image,
                                                               max_radius)
        return output_img
    else:
        max_radius = abs(int(max_radius))
        sizes = range(1, max_radius + 1)
        output_img = input_image
        for size in sizes:
            param_str_2 = "-operation opening -R " + str(size)
            output_img = morpho(output_img, param_str_2=param_str_2)
            param_str_2 = "-operation closing -R " + str(size)
            output_img = morpho(output_img, param_str_2=param_str_2)
        return output_img


def morphology_coc_alternate_sequential_filter(input_image, max_radius=None,
                                               **kwds):
    """
    Closing Opening Closing alternate sequential filter

    Parameters
    ----------
    :param SpatialImage input_image: input *SpatialImage*

    :param int max_radius: max radius

    Returns
    ----------
    :return: *SpatialImage* instance -- image and associated informations
    """
    try:
        assert isinstance(input_image, SpatialImage)
    except AssertionError:
        raise TypeError('Input image must be a SpatialImage')

    if not max_radius:
        max_radius = 1
        output_img = morphology_coc_alternate_sequential_filter(input_image,
                                                                max_radius)
        return output_img
    else:
        max_radius = abs(int(max_radius))
        sizes = range(1, max_radius + 1)
        output_img = input_image
        for size in sizes:
            param_str_2 = "-operation closing -R " + str(size)
            output_img = morpho(output_img, param_str_2=param_str_2)
            param_str_2 = "-operation opening -R " + str(size)
            output_img = morpho(output_img, param_str_2=param_str_2)
            param_str_2 = "-operation closing -R " + str(size)
            output_img = morpho(output_img, param_str_2=param_str_2)
        return output_img


def morphology_oco_alternate_sequential_filter(input_image, max_radius=None,
                                               **kwds):
    """
    Opening Closing Opening alternate sequential filter

    Parameters
    ----------
    :param SpatialImage input_image: input *SpatialImage*

    :param int max_radius: max radius

    Returns
    ----------
    :return: *SpatialImage* instance -- image and associated informations
    """
    try:
        assert isinstance(input_image, SpatialImage)
    except AssertionError:
        raise TypeError('Input image must be a SpatialImage')

    if not max_radius:
        max_radius = 1
        output_img = morphology_oco_alternate_sequential_filter(input_image,
                                                                max_radius)
        return output_img
    else:
        max_radius = abs(int(max_radius))
        sizes = range(1, max_radius + 1)
        output_img = input_image
        for size in sizes:
            param_str_2 = "-operation opening -R " + str(size)
            output_img = morpho(output_img, param_str_2=param_str_2)
            param_str_2 = "-operation closing -R " + str(size)
            output_img = morpho(output_img, param_str_2=param_str_2)
            param_str_2 = "-operation opening -R " + str(size)
            output_img = morpho(output_img, param_str_2=param_str_2)
        return output_img
