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
This module contain a generic implementation of several grayscale morphology algorithms.
"""
try:
    from timagetk.util import _input_img_check
    from timagetk.util import _method_check
    from timagetk.util import _general_kwargs
    from timagetk.util import _parallel_kwargs
    from timagetk.algorithms import morpho
    from timagetk.components import SpatialImage
except ImportError as e:
    raise ImportError('Import Error: {}'.format(e))

__all__ = ['morphology']

POSS_METHODS = ['erosion', 'dilation', 'opening', 'closing', 'morpho_gradient',
                'contrast', 'hat_transform', 'inverse_hat_transform',
                'oc_alternate_sequential_filter',
                'co_alternate_sequential_filter',
                'coc_alternate_sequential_filter',
                'oco_alternate_sequential_filter']
DEFAULT_METHOD = 0  # index of the default method in POSS_METHODS
DEF_RADIUS = 1  # default radius of the structuring elements
DEF_ITERS = 1  # default number of iterations during morphological operations


def _morpho_kwargs(**kwargs):
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

    # - Parse general kwargs:
    str_param += _general_kwargs(**kwargs)
    # - Parse parallelism kwargs:
    str_param += _parallel_kwargs(**kwargs)

    return str_param


def morphology(input_image, method=None, **kwargs):
    """
    Grayscale morphology plugin.
    Available methods are:
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
    input_image: SpatialImage
        input SpatialImage to modify, should be a 'grayscale' (intensity) image
    method: str, optional
        used method, by default 'erosion'

    **kwargs
    ------
    radius: int, optional
        radius of the structuring element, default is 1
    iterations: int, optional
        number of time to apply the morphological operation, default is 1
    max_radius: int, optional
        maximum value reached by 'radius', starting from 1, when performing
        'sequential filtering' methods, default is 3
    connectivity: int, optional
        use it to override the default 'sphere' parameter for the structuring
        element, equivalent to 'connectivity=18'

    Returns
    -------
    SpatialImage: image and metadata

    Example
    -------
    >>> from timagetk.util import data_path
    >>> from timagetk.components import imread
    >>> from timagetk.plugins import morphology
    >>> image_path = data_path('time_0_cut.inr')
    >>> input_image = imread(image_path)
    >>> dilation_image = morphology(input_image, radius=2, iterations=2, method='dilation')
    >>> oc_asf_image = morphology(input_image, max_radius=3, method='oc_alternate_sequential_filter')
    """
    # - Assert the 'input_image' is a SpatialImage instance:
    _input_img_check(input_image)
    # - Set method if None and check it is a valid method:
    method = _method_check(method, POSS_METHODS, DEFAULT_METHOD)

    try:
        assert kwargs.get('try_plugin', False)
        from openalea.core.service.plugin import plugin_function
    except AssertionError or ImportError:
        max_radius_val = kwargs.pop('max_radius', 3)
        if method == 'erosion':
            return morphology_erosion(input_image, **kwargs)
        if method == 'dilation':
            return morphology_dilation(input_image, **kwargs)
        if method == 'opening':
            return morphology_opening(input_image, **kwargs)
        if method == 'closing':
            return morphology_closing(input_image, **kwargs)
        if method == 'morpho_gradient':
            return morphology_gradient(input_image, **kwargs)
        if method == 'contrast':
            return morphology_contrast(input_image, **kwargs)
        if method == 'hat_transform':
            return morphology_hat_transform(input_image, **kwargs)
        if method == 'inverse_hat_transform':
            return morphology_inverse_hat_transform(input_image, **kwargs)
        if method == 'oc_alternate_sequential_filter':
            return morphology_oc_alternate_sequential_filter(input_image,
                                                             max_radius_val,
                                                             **kwargs)
        if method == 'co_alternate_sequential_filter':
            return morphology_co_alternate_sequential_filter(input_image,
                                                             max_radius_val,
                                                             **kwargs)
        if method == 'coc_alternate_sequential_filter':
            return morphology_coc_alternate_sequential_filter(input_image,
                                                              max_radius_val,
                                                              **kwargs)
        if method == 'oco_alternate_sequential_filter':
            return morphology_oco_alternate_sequential_filter(input_image,
                                                              max_radius_val,
                                                              **kwargs)
    else:
        func = plugin_function('openalea.image', method)
        if func is not None:
            print "WARNING: using 'plugin' functionality from 'openalea.core'!"
            return func(input_image, **kwargs)
        else:
            raise NotImplementedError("Returned 'plugin_function' is None!")


def morphology_erosion(input_image, radius=DEF_RADIUS, iterations=DEF_ITERS,
                       **kwargs):
    """
    Morpholocial erosion on grayscale image.

    Parameters
    ----------
    input_image : SpatialImage
         input image to transform
    radius : int, optional
         radius to use for structuring element, default is 1
    iterations : int, optional
         number of iterations to performs with structuring element, default is 1

    Returns
    ----------
    SpatialImage
         transformed image with its metadata
    """
    params = '-erosion'
    params += _morpho_kwargs(radius=radius, iterations=iterations, **kwargs)
    return morpho(input_image, param_str_2=params)


def morphology_dilation(input_image, radius=DEF_RADIUS, iterations=DEF_ITERS,
                        **kwargs):
    """
    Morpholocial dilation on grayscale image.

    Parameters
    ----------
    input_image : SpatialImage
         input image to transform
    radius : int, optional
         radius to use for structuring element, default is 1
    iterations : int, optional
         number of iterations to performs with structuring element, default is 1

    Returns
    ----------
    SpatialImage
         transformed image with its metadata
    """
    params = '-dilation'
    params += _morpho_kwargs(radius=radius, iterations=iterations, **kwargs)
    return morpho(input_image, param_str_2=params)


def morphology_opening(input_image, radius=DEF_RADIUS, iterations=DEF_ITERS,
                       **kwargs):
    """
    Morpholocial opening on grayscale image.

    Parameters
    ----------
    input_image : SpatialImage
         input image to transform
    radius : int, optional
         radius to use for structuring element, default is 1
    iterations : int, optional
         number of iterations to performs with structuring element, default is 1

    Returns
    ----------
    SpatialImage
         transformed image with its metadata
    """
    params = '-opening'
    params += _morpho_kwargs(radius=radius, iterations=iterations, **kwargs)
    return morpho(input_image, param_str_2=params)


def morphology_closing(input_image, radius=DEF_RADIUS, iterations=DEF_ITERS,
                       **kwargs):
    """
    Closing on grayscale image.

    Parameters
    ----------
    input_image : SpatialImage
         input image to transform
    radius : int, optional
         radius to use for structuring element, default is 1
    iterations : int, optional
         number of iterations to performs with structuring element, default is 1

    Returns
    ----------
    SpatialImage
         transformed image with its metadata
    """
    params = '-closing'
    params += _morpho_kwargs(radius=radius, iterations=iterations, **kwargs)
    return morpho(input_image, param_str_2=params)


def morphology_hat_transform(input_image, radius=DEF_RADIUS,
                             iterations=DEF_ITERS, **kwargs):
    """
    Hat transform on grayscale image.

    Parameters
    ----------
    input_image : SpatialImage
         input image to transform
    radius : int, optional
         radius to use for structuring element, default is 1
    iterations : int, optional
         number of iterations to performs with structuring element, default is 1

    Returns
    ----------
    SpatialImage
         transformed image with its metadata
    """
    params = '-closinghat'
    params += _morpho_kwargs(radius=radius, iterations=iterations, **kwargs)
    return morpho(input_image, param_str_2=params)


def morphology_inverse_hat_transform(input_image, radius=DEF_RADIUS,
                                     iterations=DEF_ITERS,
                                     **kwargs):
    """
    Inverse hat transform on grayscale image.

    Parameters
    ----------
    input_image : SpatialImage
         input image to transform
    radius : int, optional
         radius to use for structuring element, default is 1
    iterations : int, optional
         number of iterations to performs with structuring element, default is 1

    Returns
    ----------
    SpatialImage
         transformed image with its metadata
    """
    params = '-openinghat'
    params += _morpho_kwargs(radius=radius, iterations=iterations, **kwargs)
    return morpho(input_image, param_str_2=params)


def morphology_gradient(input_image, radius=DEF_RADIUS, iterations=DEF_ITERS,
                        **kwargs):
    """
    Morphological gradient on grayscale image.

    Parameters
    ----------
    input_image : SpatialImage
         input image to transform
    radius : int, optional
         radius to use for structuring element, default is 1
    iterations : int, optional
         number of iterations to performs with structuring element, default is 1

    Returns
    ----------
    SpatialImage
         transformed image with its metadata
    """
    params = '-gradient'
    params += _morpho_kwargs(radius=radius, iterations=iterations, **kwargs)
    return morpho(input_image, param_str_2=params)


def morphology_contrast(input_image, radius=DEF_RADIUS, iterations=DEF_ITERS,
                        **kwargs):
    """
    Contrast enhancement on grayscale image.

    Parameters
    ----------
    input_image : SpatialImage
         input image to transform
    radius : int, optional
         radius to use for structuring element, default is 1
    iterations : int, optional
         number of iterations to performs with structuring element, default is 1

    Returns
    ----------
    SpatialImage
         transformed image with its metadata
    """
    params = '-contrast'
    params += _morpho_kwargs(radius=radius, iterations=iterations, **kwargs)
    return morpho(input_image, param_str_2=params)


def morphology_oc_alternate_sequential_filter(input_image, max_radius=None,
                                              **kwargs):
    """
    Opening Closing alternate sequential filter on grayscale image.

    Parameters
    ----------
    input_image : SpatialImage
        input image to transform
    max_radius : int, optional
        maximum radius to use, starting at 1

    Returns
    ----------
    SpatialImage
        transformed image and its metadata
    """
    if max_radius is None:
        max_radius = 1

    max_radius = abs(int(max_radius))
    sizes = range(1, max_radius + 1)
    output_img = input_image
    for size in sizes:
        output_img = morphology_opening(output_img, radius=size, **kwargs)
        output_img = morphology_closing(output_img, radius=size, **kwargs)

    return output_img


def morphology_co_alternate_sequential_filter(input_image, max_radius=None,
                                              **kwargs):
    """
    Closing Opening alternate sequential filter on grayscale image.

    Parameters
    ----------
    input_image : SpatialImage
        input image to transform
    max_radius : int, optional
        maximum radius to use, starting at 1

    Returns
    ----------
    SpatialImage
        transformed image and its metadata

    """
    if max_radius is None:
        max_radius = 1

    max_radius = abs(int(max_radius))
    sizes = range(1, max_radius + 1)
    output_img = input_image
    for size in sizes:
        output_img = morphology_closing(output_img, radius=size, **kwargs)
        output_img = morphology_opening(output_img, radius=size, **kwargs)

    return output_img


def morphology_coc_alternate_sequential_filter(input_image, max_radius=None,
                                               **kwargs):
    """
    Closing Opening Closing alternate sequential filter on grayscale image.

    Parameters
    ----------
    input_image : SpatialImage
        input image to transform
    max_radius : int, optional
        maximum radius to use, starting at 1

    Returns
    ----------
    SpatialImage
        transformed image and its metadata
    """
    if max_radius is None:
        max_radius = 1

    max_radius = abs(int(max_radius))
    sizes = range(1, max_radius + 1)
    output_img = input_image
    for size in sizes:
        output_img = morphology_closing(output_img, radius=size, **kwargs)
        output_img = morphology_opening(output_img, radius=size, **kwargs)
        output_img = morphology_closing(output_img, radius=size, **kwargs)

    return output_img


def morphology_oco_alternate_sequential_filter(input_image, max_radius=None,
                                               **kwargs):
    """
    Opening Closing Opening alternate sequential filter on grayscale image.

    Parameters
    ----------
    input_image : SpatialImage
        input image to transform
    max_radius : int, optional
        maximum radius to use, starting at 1

    Returns
    ----------
    SpatialImage
        transformed image and its metadata
    """
    if max_radius is None:
        max_radius = 1

    max_radius = abs(int(max_radius))
    sizes = range(1, max_radius + 1)
    output_img = input_image
    for size in sizes:
        output_img = morphology_opening(output_img, radius=size, **kwargs)
        output_img = morphology_closing(output_img, radius=size, **kwargs)
        output_img = morphology_opening(output_img, radius=size, **kwargs)

    return output_img
