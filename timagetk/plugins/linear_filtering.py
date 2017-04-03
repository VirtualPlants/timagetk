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
This module contains a generic implementation of several linear filtering algorithms.
"""

try:
    from timagetk.algorithms import linearfilter
    from timagetk.components import SpatialImage
except ImportError:
    raise ImportError('Import Error')

__all__ = ['linear_filtering']


def linear_filtering(input_image, method=None, **kwds):
    """
    Linear filtering plugin. Available methods are :

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
    :param *SpatialImage* input_image: input *SpatialImage*

    :param str method: used method (example: 'gaussian_smoothing')

    Returns
    ----------
    :return: ``SpatialImage`` instance -- image and metadata

    Example
    ----------
    >>> from timagetk.util import data, data_path
    >>> from timagetk.plugins import linear_filtering
    >>> image_path = data_path('time_0_cut.inr')
    >>> input_image = data(image_path)
    >>> gauss_image = linear_filtering(input_image, std_dev=2.0, method='gaussian_smoothing')
    >>> grad_mod_image = linear_filtering(input_image, method='gradient_modulus')
    >>> lap_image = linear_filtering(input_image, method='laplacian')
    """
    poss_methods = ['gaussian_smoothing', 'gradient', 'gradient_modulus', 'hessian', 'laplacian',
                    'gradient_hessian', 'gradient_laplacian', 'zero_crossings_hessian',
                    'zero_crossings_laplacian']

    conds = isinstance(input_image, SpatialImage)
    if conds:
        if method is None:
            return linear_filtering_gaussian_smoothing(input_image)
        elif method is not None:
            if method in poss_methods:
                try:
                    from openalea.core.service.plugin import plugin_function
                    func = plugin_function('openalea.image', method)
                    if func is not None:
                        return func(input_image, **kwds)
                except:
                    if method=='gaussian_smoothing':
                        std_dev_val = kwds.get('std_dev', None)
                        return linear_filtering_gaussian_smoothing(input_image, std_dev=std_dev_val)
                    elif method=='gradient':
                        return linear_filtering_gradient(input_image)
                    elif method=='gradient_modulus':
                        return linear_filtering_gradient_modulus(input_image)
                    elif method=='hessian':
                        return linear_filtering_hessian(input_image)
                    elif method=='laplacian':
                        return linear_filtering_laplacian(input_image)
                    elif method=='gradient_hessian':
                        return linear_filtering_gradient_hessian(input_image)
                    elif method=='gradient_laplacian':
                        return linear_filtering_gradient_laplacian(input_image)
                    elif method=='zero_crossings_hessian':
                        return linear_filtering_zero_crossings_hessian(input_image)
                    elif method=='zero_crossings_laplacian':
                        return linear_filtering_zero_crossings_laplacian(input_image)
            else:
                print('Available methods :'), poss_methods
                raise NotImplementedError(method)
    else:
        raise TypeError('Input image must be a SpatialImage')
        return


def linear_filtering_gaussian_smoothing(input_image, std_dev=None, **kwds):
    """
    Gaussian smoothing filter.

    Parameters
    ----------
    :param *SpatialImage* input_image: input_image *SpatialImage*
    :param float std_dev: optinal, standard deviation. Default: std_dev=1.0

    Returns
    ----------
    :return: ``SpatialImage`` instance -- image and metadata
    """
    conds = isinstance(input_image, SpatialImage)
    if conds:
        if std_dev is None:
            std_dev = 1.0
        elif std_dev is not None:
            std_dev = abs(float(std_dev))
        params = '-smoothing -sigma %s' % std_dev
        return linearfilter(input_image, param_str_2=params)
    else:
        raise TypeError('Input image must be a SpatialImage')
        return


def linear_filtering_gradient_modulus(input_image):
    """
    Gradient modulus filter.

    Parameters
    ----------
    :param *SpatialImage* input_image: input_image *SpatialImage*

    Returns
    ----------
    :return: ``SpatialImage`` instance -- image and metadata
    """
    conds = isinstance(input_image, SpatialImage)
    if conds:
        params = '-gradient-modulus'
        return linearfilter(input_image, param_str_2=params)
    else:
        raise TypeError('Input image must be a SpatialImage')
        return


def linear_filtering_gradient(input_image):
    """
    Gradient filter.

    Parameters
    ----------
    :param *SpatialImage* input_image: input_image *SpatialImage*

    Returns
    ----------
    :return: ``SpatialImage`` instance -- image and metadata
    """
    conds = isinstance(input_image, SpatialImage)
    if conds:
        params = '-gradient'
        return linearfilter(input_image, param_str_2=params)
    else:
        raise TypeError('Input image must be a SpatialImage')
        return


def linear_filtering_hessian(input_image):
    """
    Hessian filter.

    Parameters
    ----------
    :param *SpatialImage* input_image: input_image *SpatialImage*

    Returns
    ----------
    :return: ``SpatialImage`` instance -- image and metadata
    """
    conds = isinstance(input_image, SpatialImage)
    if conds:
        params = '-hessian'
        return linearfilter(input_image, param_str_2=params)
    else:
        raise TypeError('Input image must be a SpatialImage')
        return


def linear_filtering_laplacian(input_image):
    """
    Laplacian filter.

    Parameters
    ----------
    :param *SpatialImage* input_image: input_image *SpatialImage*

    Returns
    ----------
    :return: ``SpatialImage`` instance -- image and metadata
    """
    conds = isinstance(input_image, SpatialImage)
    if conds:
        params = '-laplacian'
        return linearfilter(input_image, param_str_2=params)
    else:
        raise TypeError('Input image must be a SpatialImage')
        return


def linear_filtering_zero_crossings_hessian(input_image, **kwds):
    """
    Zero crossings hessian filter.

    Parameters
    ----------
    :param *SpatialImage* input_image: input_image *SpatialImage*

    Returns
    ----------
    :return: ``SpatialImage`` instance -- image and metadata
    """
    conds = isinstance(input_image, SpatialImage)
    if conds:
        params = '-zero-crossings-hessian'
        return linearfilter(input_image, param_str_2=params)
    else:
        raise TypeError('Input image must be a SpatialImage')
        return


def linear_filtering_zero_crossings_laplacian(input_image, **kwds):
    """
    Zero crossings laplacian filter.

    Parameters
    ----------
    :param *SpatialImage* input_image: input_image *SpatialImage*

    Returns
    ----------
    :return: ``SpatialImage`` instance -- image and metadata
    """
    conds = isinstance(input_image, SpatialImage)
    if conds:
        params = '-zero-crossings-laplacian'
        return linearfilter(input_image, param_str_2=params)
    else:
        raise TypeError('Input image must be a SpatialImage')
        return


def linear_filtering_gradient_hessian(input_image, **kwds):
    """
    Gradient modulus onto zero-crossings of hessian image.

    Parameters
    ----------
    :param *SpatialImage* input_image: input_image *SpatialImage*

    Returns
    ----------
    :return: ``SpatialImage`` instance -- image and metadata
    """
    conds = isinstance(input_image, SpatialImage)
    if conds:
        params = '-gradient-hessian'
        return linearfilter(input_image, param_str_2=params)
    else:
        raise TypeError('Input image must be a SpatialImage')
        return


def linear_filtering_gradient_laplacian(input_image, **kwds):
    """
    Gradient modulus onto zero-crossings of laplacian image.

    Parameters
    ----------
    :param *SpatialImage* input_image: input_image *SpatialImage*

    Returns
    ----------
    :return: ``SpatialImage`` instance -- image and metadata
    """
    conds = isinstance(input_image, SpatialImage)
    if conds:
        params = '-gradient-laplacian'
        return linearfilter(input_image, param_str_2=params)
    else:
        raise TypeError('Input image must be a SpatialImage')
        return