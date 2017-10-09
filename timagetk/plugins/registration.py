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

DEFAULT_METHOD = 'rigid_registration'

def registration(floating_img, reference_img, method=None, **kwds):
    """
    Registration plugin. Available methods are :

    * rigid_registration
    * affine_registration
    * deformable_registration

    Parameters
    ----------
    floating_img : SpatialImage
        floating SpatialImage, ie. image to register
    reference_img : SpatialImage
        reference *SpatialImage* for registration
    method : str, optional
        used method, eg. 'rigid_registration' (default)
    param : bool, optional
        print parameters, default is False
    parallel : bool, optional
        set usage of parallel mode, default is True
    parallel-type : str, in ['none'|'openmp'|'omp'|'pthread'|'thread']
        type of parrallelism to use, default is thread
    time : bool, optional
        print elapsed time, default is True
    try_plugin : bool, optional
        manually control the use of openalea 'plugin' functionality, use
        it by default (True)


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

    try:
        assert isinstance(floating_img, SpatialImage)
    except:
        raise TypeError('Floating image must be a SpatialImage instance.')
    try:
        assert isinstance(reference_img, SpatialImage)
    except:
        raise TypeError('Reference image must be a SpatialImage instance.')

    if method is None:
        method = DEFAULT_METHOD
    else:
        try:
            assert method in poss_methods
        except:
            print 'Available methods :', poss_methods
            raise NotImplementedError(method)
    try:
        assert kwds.get('try_plugin', True)
        from openalea.core.service.plugin import plugin_function
        func = plugin_function('openalea.image', method)
        if func is not None:
            return func(floating_img, reference_img, **kwds)
    except:
        if method=='rigid_registration':
            trsf_out, res_image = rigid_registration(floating_img, reference_img, **kwds)
            return trsf_out, res_image
        elif method=='affine_registration':
            trsf_out, res_image = affine_registration(floating_img, reference_img, **kwds)
            return trsf_out, res_image
        elif method=='deformable_registration':
            trsf_out, res_image = deformable_registration(floating_img, reference_img, **kwds)
            return trsf_out, res_image


def _method_param_check(**kwds):
    """
    Set parameters default values and make sure they are of the right type.
    """
    str_param = ""
    str_param += ' -pyramid-lowest-level %d' % (kwds.get('pyramid_lowest_level', 0))
    str_param += ' -pyramid-highest-level %d' % (kwds.get('pyramid_highest_level', 3))
    if kwds.get('param', True):
        str_param += ' -param'
    if kwds.get('command_line', False):
        str_param += ' -command-line'
    if kwds.get('verbose', True):
        str_param += ' -verbose'
    if kwds.get('parallel', True):
        str_param += ' -parallel'
        str_param += ' -parallel-type '+kwds.get('parallel_type', 'thread')
    if kwds.get('time', True):
        str_param += ' -time'
    return str_param

def rigid_registration(floating_img, reference_img, **kwds):
    """
    Performs RIGID registration by calling `blockmatching` with adequate parameters.
    """
    param_str_2 = _method_param_check(**kwds)
    trsf_rig, res_rig = blockmatching(floating_img, reference_img, param_str_2=param_str_2)
    return trsf_rig, res_rig


def affine_registration(floating_img, reference_img, **kwds):
    """
    Calls blockmatching with the RIGID registration parameters.
    """
    param_str_2 = _method_param_check(**kwds)
    bm_default_param = '-trsf-type {}'.format(DEFAULT_METHOD)
    if '-trsf-type {}'.format(DEFAULT_METHOD) in param_str_2:
        param_str_2 = param_str_2.replace(bm_default_param, '')
    trsf_rig, res_rig = rigid_registration(floating_img, reference_img, param_str_2=param_str_2)
    param_str_2 += '-trsf-type affine'
    # using 'left_transformation' returns an affine trsf matrix without the 'rigid deformation part!
    #~ trsf_aff, res_aff = blockmatching(floating_img, reference_img, left_transformation=trsf_rig, param_str_2=param_str_2)
    trsf_aff, res_aff = blockmatching(floating_img, reference_img, init_result_transformation=trsf_rig, param_str_2=param_str_2)
    return trsf_aff, res_aff


def deformable_registration(floating_img, reference_img, **kwds):
    param_str_2 = _method_param_check(**kwds)
    bm_default_param = '-trsf-type {}'.format(DEFAULT_METHOD)
    if BLOCKMATCHING_DEFAULT in param_str_2:
        param_str_2 = param_str_2.replace(bm_default_param, '')
    trsf_rig, res_rig = rigid_registration(floating_img, reference_img, param_str_2=param_str_2)
    param_str_2 += '-trsf-type vectorfield'
    trsf_def, res_def = blockmatching(floating_img, reference_img,
                                      init_result_transformation=trsf_rig,
                                      param_str_2=param_str_2)
    return trsf_def, res_def
