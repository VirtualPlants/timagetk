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
This module contains a generic implementation of several registration algorithms.
"""

try:
    from timagetk.algorithms import blockmatching
    from timagetk.components import SpatialImage
except ImportError:
    raise ImportError('Import Error')

__all__ = ['registration']


def registration(floating_img, reference_img, method=None, **kwds):
    """
    Registration plugin. Available methods are :

    * rigid_registration
    * affine_registration
    * deformable_registration

    Parameters
    ----------
    :param *SpatialImage* floating_img: floating *SpatialImage*

    :param *SpatialImage* reference_img: reference *SpatialImage*

    :param str method: used method (example: 'rigid_registration')

    Returns
    ----------
    :return: ``BalTransformation`` instance -- trsf_out, *BalTransformation* transformation

    :return: ``SpatialImage`` instance -- res_image, *SpatialImage* image and metadata

    Example
    ----------
    >>> from timagetk.util import data, data_path
    >>> from timagetk.plugins import registration
    >>> image_path = data_path('time_0_cut.inr')
    >>> floating_image = data(image_path)
    >>> image_path = data_path('time_1_cut.inr')
    >>> reference_image = data(image_path)
    >>> trsf_rig, res_rig = registration(floating_image, reference_image, method='rigid_registration')
    >>> trsf_aff, res_aff = registration(floating_image, reference_image, method='affine_registration')
    >>> trsf_def, res_def = registration(floating_image, reference_image, method='deformable_registration')
    """
    poss_methods = ['rigid_registration', 'affine_registration', 'deformable_registration']

    conds = isinstance(floating_img, SpatialImage) and isinstance(reference_img, SpatialImage)
    if conds:
        if method is None:
            return rigid_registration(floating_img, reference_img)
        elif method is not None:
            if method in poss_methods:
                try:
                    from openalea.core.service.plugin import plugin_function
                    func = plugin_function('openalea.image', method)
                    if func is not None:
                        return func(floating_img, reference_img, **kwds)
                except:
                    if method=='rigid_registration':
                        trsf_out, res_image = rigid_registration(floating_img, reference_img)
                        return trsf_out, res_image
                    elif method=='affine_registration':
                        trsf_out, res_image = affine_registration(floating_img, reference_img)
                        return trsf_out, res_image
                    elif method=='deformable_registration':
                        trsf_out, res_image = deformable_registration(floating_img, reference_img)
                        return trsf_out, res_image
            else:
                print('Available methods :'), poss_methods
                raise NotImplementedError(method)
    else:
        raise TypeError('Floating image and Refernece image must be a SpatialImage')
        return


def rigid_registration(floating_img, reference_img):
    trsf_rig, res_rig = blockmatching(floating_img, reference_img)
    return trsf_rig, res_rig


def affine_registration(floating_img, reference_img):
    trsf_rig, res_rig = rigid_registration(floating_img, reference_img)
    param_str_2 = '-trsf-type affine'
    trsf_aff, res_aff = blockmatching(floating_img, reference_img, left_transformation=trsf_rig, param_str_2=param_str_2)
    return trsf_aff, res_aff


def deformable_registration(floating_img, reference_img):
    trsf_rig, res_rig = blockmatching(floating_img, reference_img)
    param_str_2 = '-trsf-type vectorfield'
    trsf_def, res_def = blockmatching(floating_img, reference_img,
                                      init_result_transformation=trsf_rig,
                                      param_str_2=param_str_2)
    return trsf_def, res_def