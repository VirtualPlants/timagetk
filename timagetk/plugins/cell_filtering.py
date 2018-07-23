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
This module contain a generic implementation of several cell filtering algorithms.
"""

try:
    from timagetk.util import _input_img_check
    from timagetk.util import _method_check
    from timagetk.util import _general_kwargs
    from timagetk.util import _parallel_kwargs
    from timagetk.algorithms import cell_filter
    from timagetk.components import SpatialImage
except ImportError as e:
    raise ImportError('Import Error: {}'.format(e))

__all__ = ['cell_filtering']

POSS_METHODS = ['erosion', 'dilation', 'opening', 'closing', 'morpho_gradient',
                'contrast',
                'hat_transform', 'inverse_hat_transform',
                'oc_alternate_sequential_filter',
                'co_alternate_sequential_filter',
                'coc_alternate_sequential_filter',
                'oco_alternate_sequential_filter']
DEFAULT_METHOD = 0  # index of the default method in POSS_METHODS
DEF_RADIUS = 1  # default radius of the structuring elements
DEF_ITERS = 1  # default number of iterations during morphological operations


def _cellfilter_kwargs(**kwargs):
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


def cell_filtering(input_image, method=None, **kwargs):
    """
    Grayscale cell_filtering plugin
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
    input_image : SpatialImage
         input image to transform

    :param str method: used method (example: 'erosion')

    Returns
    ----------
    SpatialImage
         transformed image with its metadata

    Example
    ----------
    >>> from timagetk.util import data_path
    >>> from timagetk.components import imread
    >>> from timagetk.plugins import cell_filtering
    >>> image_path = data_path('time_0_cut.inr')
    >>> input_image = imread(image_path)
    >>> dilation_image = cell_filtering(input_image, radius=2, iterations=2, method='dilation')
    >>> oc_asf_image = cell_filtering(input_image, max_radius=3, method='oc_alternate_sequential_filter')
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
            return cell_filtering_erosion(input_image, **kwargs)
        elif method == 'dilation':
            return cell_filtering_dilation(input_image, **kwargs)
        elif method == 'opening':
            return cell_filtering_opening(input_image, **kwargs)
        elif method == 'closing':
            return cell_filtering_closing(input_image, **kwargs)
        elif method == 'morpho_gradient':
            return cell_filtering_gradient(input_image, **kwargs)
        elif method == 'contrast':
            return cell_filtering_contrast(input_image, **kwargs)
        elif method == 'hat_transform':
            return cell_filtering_hat_transform(input_image, **kwargs)
        elif method == 'inverse_hat_transform':
            return cell_filtering_inverse_hat_transform(input_image, **kwargs)
        elif method == 'oc_alternate_sequential_filter':
            return cell_filtering_oc_alternate_sequential_filter(input_image,
                                                                 max_radius=max_radius_val,
                                                                 **kwargs)
        elif method == 'co_alternate_sequential_filter':
            return cell_filtering_co_alternate_sequential_filter(input_image,
                                                                 max_radius=max_radius_val,
                                                                 **kwargs)
        elif method == 'coc_alternate_sequential_filter':
            return cell_filtering_coc_alternate_sequential_filter(input_image,
                                                                  max_radius=max_radius_val,
                                                                  **kwargs)
        elif method == 'oco_alternate_sequential_filter':
            return cell_filtering_oco_alternate_sequential_filter(input_image,
                                                                  max_radius=max_radius_val,
                                                                  **kwargs)
    else:
        func = plugin_function('openalea.image', method)
        if func is not None:
            print "WARNING: using 'plugin' functionality from 'openalea.core'!"
            return func(input_image, **kwargs)
        else:
            raise NotImplementedError("Returned 'plugin_function' is None!")


def cell_filtering_erosion(input_image, radius=DEF_RADIUS, iterations=DEF_ITERS,
                           **kwargs):
    """
    Erosion

    Parameters
    ----------
    input_image : SpatialImage
         input image to transform

    radius : int
         radius to use for structuring element, default is 1

    iterations : int
         number of iterations to performs with structuring element, default is 1

    Returns
    ----------
    SpatialImage
         transformed image with its metadata
    """
    params = '-erosion'
    params += _cellfilter_kwargs(radius=radius, iterations=iterations, **kwargs)
    return cell_filter(input_image, param_str_2=params)


def cell_filtering_dilation(input_image, radius=DEF_RADIUS,
                            iterations=DEF_ITERS, **kwargs):
    """
    Dilation

    Parameters
    ----------
    input_image : SpatialImage
         input image to transform

    radius : int
         radius to use for structuring element, default is 1

    iterations : int
         number of iterations to performs with structuring element, default is 1

    Returns
    ----------
    SpatialImage
         transformed image with its metadata
    """
    params = '-dilation'
    params += _cellfilter_kwargs(radius=radius, iterations=iterations, **kwargs)
    return cell_filter(input_image, param_str_2=params)


def cell_filtering_opening(input_image, radius=DEF_RADIUS, iterations=DEF_ITERS,
                           **kwargs):
    """
    Opening

    Parameters
    ----------
    input_image : SpatialImage
         input image to transform

    radius : int
         radius to use for structuring element, default is 1

    iterations : int
         number of iterations to performs with structuring element, default is 1

    Returns
    ----------
    SpatialImage
         transformed image with its metadata
    """
    params = '-opening'
    params += _cellfilter_kwargs(radius=radius, iterations=iterations, **kwargs)
    return cell_filter(input_image, param_str_2=params)


def cell_filtering_closing(input_image, radius=DEF_RADIUS, iterations=DEF_ITERS,
                           **kwargs):
    """
    Closing

    Parameters
    ----------
    input_image : SpatialImage
         input image to transform

    radius : int
         radius to use for structuring element, default is 1

    iterations : int
         number of iterations to performs with structuring element, default is 1

    Returns
    ----------
    SpatialImage
         transformed image with its metadata
    """
    params = '-closing'
    params += _cellfilter_kwargs(radius=radius, iterations=iterations, **kwargs)
    return cell_filter(input_image, param_str_2=params)


def cell_filtering_hat_transform(input_image, radius=DEF_RADIUS,
                                 iterations=DEF_ITERS,
                                 **kwargs):
    """
    Hat transform

    Parameters
    ----------
    input_image : SpatialImage
         input image to transform

    radius : int
         radius to use for structuring element, default is 1

    iterations : int
         number of iterations to performs with structuring element, default is 1

    Returns
    ----------
    SpatialImage
         transformed image with its metadata
    """
    params = '-closinghat'
    params += _cellfilter_kwargs(radius=radius, iterations=iterations, **kwargs)
    return cell_filter(input_image, param_str_2=params)


def cell_filtering_inverse_hat_transform(input_image, radius=DEF_RADIUS,
                                         iterations=DEF_ITERS, **kwargs):
    """
    Inverse hat transform

    Parameters
    ----------
    input_image : SpatialImage
         input image to transform

    radius : int
         radius to use for structuring element, default is 1

    iterations : int
         number of iterations to performs with structuring element, default is 1

    Returns
    ----------
    SpatialImage
         transformed image with its metadata
    """
    params = '-openinghat'
    params += _cellfilter_kwargs(radius=radius, iterations=iterations, **kwargs)
    return cell_filter(input_image, param_str_2=params)


def cell_filtering_gradient(input_image, radius=DEF_RADIUS,
                            iterations=DEF_ITERS, **kwargs):
    """
    Morphological gradient

    Parameters
    ----------
    input_image : SpatialImage
         input image to transform

    radius : int
         radius to use for structuring element, default is 1

    iterations : int
         number of iterations to performs with structuring element, default is 1

    Returns
    ----------
    SpatialImage
         transformed image with its metadata
    """
    params = '-gradient'
    params += _cellfilter_kwargs(radius=radius, iterations=iterations, **kwargs)
    return cell_filter(input_image, param_str_2=params)


def cell_filtering_contrast(input_image, radius=DEF_RADIUS,
                            iterations=DEF_ITERS, **kwargs):
    """
    Contrast enhancement

    Parameters
    ----------
    input_image : SpatialImage
         input image to transform

    radius : int
         radius to use for structuring element, default is 1

    iterations : int
         number of iterations to performs with structuring element, default is 1

    Returns
    ----------
    SpatialImage
         transformed image with its metadata
    """
    params = '-contrast'
    params += _cellfilter_kwargs(radius=radius, iterations=iterations, **kwargs)
    return cell_filter(input_image, param_str_2=params)


def cell_filtering_oc_alternate_sequential_filter(input_image, max_radius=None,
                                                  **kwargs):
    """
    Opening Closing alternate sequential filter

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
        output_img = cell_filtering_opening(output_img, radius=size, **kwargs)
        output_img = cell_filtering_closing(output_img, radius=size, **kwargs)

    return output_img


def cell_filtering_co_alternate_sequential_filter(input_image, max_radius=None,
                                                  **kwargs):
    """
    Closing Opening alternate sequential filter

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
        output_img = cell_filtering_closing(output_img, radius=size, **kwargs)
        output_img = cell_filtering_opening(output_img, radius=size, **kwargs)

    return output_img


def cell_filtering_coc_alternate_sequential_filter(input_image, max_radius=None,
                                                   **kwargs):
    """
    Closing Opening Closing alternate sequential filter

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
        output_img = cell_filtering_closing(output_img, radius=size, **kwargs)
        output_img = cell_filtering_opening(output_img, radius=size, **kwargs)
        output_img = cell_filtering_closing(output_img, radius=size, **kwargs)

    return output_img


def cell_filtering_oco_alternate_sequential_filter(input_image, max_radius=None,
                                                   **kwargs):
    """
    Opening Closing Opening alternate sequential filter

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
        output_img = cell_filtering_opening(output_img, radius=size, **kwargs)
        output_img = cell_filtering_closing(output_img, radius=size, **kwargs)
        output_img = cell_filtering_opening(output_img, radius=size, **kwargs)

    return output_img
