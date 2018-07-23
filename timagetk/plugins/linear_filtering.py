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
This module contains a generic implementation of several linear filtering algorithms.
"""
try:
    from timagetk.util import _input_img_check
    from timagetk.util import _method_check
    from timagetk.util import _general_kwargs
    from timagetk.util import _parallel_kwargs
    from timagetk.algorithms import linearfilter
    from timagetk.components import SpatialImage
except ImportError as e:
    raise ImportError('Import Error: {}'.format(e))

__all__ = ['linear_filtering', 'list_linear_methods']

POSS_METHODS = ['gaussian_smoothing', 'gradient', 'gradient_modulus', 'hessian',
                'laplacian', 'gradient_hessian', 'gradient_laplacian',
                'zero_crossings_hessian', 'zero_crossings_laplacian']
DEFAULT_METHOD = 0  # index of the default method in POSS_METHODS


def list_linear_methods():
    """
    List the available 'methods' to call with `linear_filtering`.

    Returns
    -------
    str: the list of available methods.
    """
    return POSS_METHODS


def linear_filtering(input_image, method=None, **kwargs):
    """
    Linear filtering plugin
    Available methods are:

      * gaussian_smoothing
      * gradient
      * gradient_modulus
      * hessian
      * laplacian
      * gradient_hessian
      * gradient_laplacian
      * zero_crossings_hessian
      * zero_crossings_laplacian

    Parameters
    ----------
    input_image: SpatialImage
        input SpatialImage to filter
    method: str, optional
        used method, default is 'gaussian_smoothing'

    **kwargs
    ------
    std_dev: float, optional
        the standard deviation to apply for Gaussian smoothing, in
    real: bool, optional
        define if the standard deviation ('std_dev') to apply is in real or
        voxel units, default is False

    Returns
    -------
    SpatialImage: image and metadata

    Notes
    -----
    Only 'gaussian_smoothing' get the keyword arguments.

    Examples
    --------
    >>> from timagetk.util import data_path
    >>> from timagetk.components import imread
    >>> from timagetk.plugins import linear_filtering
    >>> image_path = data_path('time_0_cut.inr')
    >>> input_image = imread(image_path)
    >>> gauss_image = linear_filtering(input_image, std_dev=2.0, method='gaussian_smoothing')
    >>> grad_mod_image = linear_filtering(input_image, method='gradient_modulus')
    >>> lap_image = linear_filtering(input_image, method='laplacian')
    """
    # - Assert the 'input_image' is a SpatialImage instance:
    _input_img_check(input_image)
    # - Set method if None and check it is a valid method:
    method = _method_check(method, POSS_METHODS, DEFAULT_METHOD)

    try:
        assert kwargs.get('try_plugin', False)
        from openalea.core.service.plugin import plugin_function
    except AssertionError or ImportError:
        if method == 'gaussian_smoothing':
            std_dev_val = kwargs.pop('std_dev', 1.0)
            real_units = kwargs.pop('real', False)
            return linear_filtering_gaussian_smoothing(input_image,
                                                       std_dev=std_dev_val,
                                                       real=real_units)
        if method == 'gradient':
            return linear_filtering_gradient(input_image)
        if method == 'gradient_modulus':
            return linear_filtering_gradient_modulus(input_image)
        if method == 'hessian':
            return linear_filtering_hessian(input_image)
        if method == 'laplacian':
            return linear_filtering_laplacian(input_image)
        if method == 'gradient_hessian':
            return linear_filtering_gradient_hessian(input_image)
        if method == 'gradient_laplacian':
            return linear_filtering_gradient_laplacian(input_image)
        if method == 'zero_crossings_hessian':
            return linear_filtering_zero_crossings_hessian(input_image)
        if method == 'zero_crossings_laplacian':
            return linear_filtering_zero_crossings_laplacian(input_image)
    else:
        func = plugin_function('openalea.image', method)
        if func is not None:
            print "WARNING: using 'plugin' functionality from 'openalea.core'!"
            return func(input_image, **kwargs)
        else:
            raise NotImplementedError("Returned 'plugin_function' is None!")


def linear_filtering_gaussian_smoothing(input_image, std_dev=1.0, real=False):
    """
    Gaussian smoothing filter.

    Parameters
    ----------
    input_image: SpatialImage
        input_image SpatialImage to smooth using Gaussian filtering
    std_dev: float|list, optional
        standard deviation to use, by default std_dev=1.0, can be specified for
        each dimension by a list (eg. [2.0, 2.0, 1.0] apply twice the smoothing
        in X and Y direction than in Z)
    real: bool, optional
        if False (default) the standard deviation is in voxel unit, else in real
        units

    Returns
    -------
    SpatialImage: image and metadata

    Notes
    -----
    If 'real' is False and 'std_dev', the same 'std_dev' value is used in all direction
    regardless of the image voxelsize!
    Use 'real=True' to adapt to a non-isometric image.
    """

    _input_img_check(input_image, real)
    # - Initiate 'param_str_2' for `linearfilter`:
    if real:
        params = '-smoothing -real-sigma '
    else:
        params = '-smoothing -sigma '

    # - Standard deviation value should be a positive float:
    if isinstance(std_dev, list):
        try:
            assert len(std_dev) == input_image.ndim
        except AssertionError:
            msg = "When using a list of 'std_dev', it should be of the same lenght than the image dimension!"
            msg += "\n"
            msg += "Got {} 'std_dev' values".format(len(std_dev))
            msg += "for an image of dimension {}.".format(input_image.ndim)
            raise ValueError(msg)
        else:
            params += ' '.join([str(abs(float(s))) for s in std_dev])
    else:
        params += str(abs(float(std_dev)))

    return linearfilter(input_image, param_str_2=params)


def linear_filtering_gradient_modulus(input_image):
    """
    Gradient modulus filter.

    Parameters
    ----------
    :param *SpatialImage* input_image: input_image *SpatialImage*

    Returns
    ----------
    SpatialImage
         transformed image with its metadata
    """
    _input_img_check(input_image)
    params = '-gradient-modulus'
    return linearfilter(input_image, param_str_2=params)


def linear_filtering_gradient(input_image):
    """
    Gradient filter.

    Parameters
    ----------
    :param *SpatialImage* input_image: input_image *SpatialImage*

    Returns
    ----------
    SpatialImage
         transformed image with its metadata
    """
    _input_img_check(input_image)
    params = '-gradient'
    return linearfilter(input_image, param_str_2=params)


def linear_filtering_hessian(input_image):
    """
    Hessian filter.

    Parameters
    ----------
    :param *SpatialImage* input_image: input_image *SpatialImage*

    Returns
    ----------
    SpatialImage
         transformed image with its metadata
    """
    _input_img_check(input_image)
    params = '-hessian'
    return linearfilter(input_image, param_str_2=params)


def linear_filtering_laplacian(input_image):
    """
    Laplacian filter.

    Parameters
    ----------
    :param *SpatialImage* input_image: input_image *SpatialImage*

    Returns
    ----------
    SpatialImage
         transformed image with its metadata
    """
    _input_img_check(input_image)
    params = '-laplacian'
    return linearfilter(input_image, param_str_2=params)


def linear_filtering_zero_crossings_hessian(input_image, **kwargs):
    """
    Zero crossings hessian filter.

    Parameters
    ----------
    :param *SpatialImage* input_image: input_image *SpatialImage*

    Returns
    ----------
    SpatialImage
         transformed image with its metadata
    """
    params = '-zero-crossings-hessian'
    return linearfilter(input_image, param_str_2=params)


def linear_filtering_zero_crossings_laplacian(input_image, **kwargs):
    """
    Zero crossings laplacian filter.

    Parameters
    ----------
    :param *SpatialImage* input_image: input_image *SpatialImage*

    Returns
    ----------
    SpatialImage
         transformed image with its metadata
    """
    _input_img_check(input_image)
    params = '-zero-crossings-laplacian'
    return linearfilter(input_image, param_str_2=params)


def linear_filtering_gradient_hessian(input_image, **kwargs):
    """
    Gradient modulus onto zero-crossings of hessian image.

    Parameters
    ----------
    :param *SpatialImage* input_image: input_image *SpatialImage*

    Returns
    ----------
    SpatialImage
         transformed image with its metadata
    """
    _input_img_check(input_image)
    params = '-gradient-hessian'
    return linearfilter(input_image, param_str_2=params)


def linear_filtering_gradient_laplacian(input_image, **kwargs):
    """
    Gradient modulus onto zero-crossings of laplacian image.

    Parameters
    ----------
    :param *SpatialImage* input_image: input_image *SpatialImage*

    Returns
    ----------
    SpatialImage
         transformed image with its metadata
    """
    _input_img_check(input_image)
    params = '-gradient-laplacian'
    return linearfilter(input_image, param_str_2=params)
