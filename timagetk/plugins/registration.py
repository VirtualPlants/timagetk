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
This module contains a generic implementation of several registration algorithms.
"""

try:
    from timagetk.util import _input_img_check
    from timagetk.util import _method_check
    from timagetk.util import _general_kwargs
    from timagetk.util import _parallel_kwargs
    from timagetk.algorithms import blockmatching
    from timagetk.components import SpatialImage
    from timagetk.algorithms.blockmatching import BLOCKMATCHING_DEFAULT
except ImportError as e:
    raise ImportError('Import Error: {}'.format(e))

__all__ = ['registration']

POSS_METHODS = ['rigid_registration', 'affine_registration',
                'deformable_registration']
DEFAULT_METHOD = 0  # index of the default method in POSS_METHODS


def _registration_kwargs(**kwargs):
    """
    Set parameters default values and make sure they are of the right type.
    """
    str_param = ""
    # - By default 'pyramid_lowest_level' is equal to 1:
    py_ll = kwargs.get('pyramid_lowest_level', 1)
    str_param += ' -pyramid-lowest-level %d' % (py_ll)
    # - By default 'pyramid_highest_level' is equal to 3:
    py_hl = kwargs.get('pyramid_highest_level', 3)
    str_param += ' -pyramid-highest-level %d' % (py_hl)
    # TODO: make shorter version of param names available ? ie. py_ll & py_hl ?

    # - Parse general kwargs:
    str_param += _general_kwargs(**kwargs)
    # - Parse parallelism kwargs:
    str_param += _parallel_kwargs(**kwargs)

    return str_param


def registration(floating_img, reference_img, method=None, **kwargs):
    """
    Registration plugin
    Available methods are:

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

    **kwargs
    ------
    pyramid_lowest_level : int, optional
        lowest level at which to compute deformation, default is 1 (min is 0)
    pyramid_highest_level : int, optional
        highest level at which to compute deformation, default is 3 (max is 3)
    try_plugin : bool, optional
        manually control the use of openalea.core 'plugin' functionality, avoid
        it by default (ie. try_plugin=False)

    Returns
    ----------
    trsf_out : BalTransformation
        computed transformation
    res_image : SpatialImage
        deformed image and metadata

    Example
    ----------
    >>> from timagetk.util import data_path
    >>> from timagetk.components import imread
    >>> from timagetk.plugins import registration
    >>> image_path = data_path('time_0_cut.inr')
    >>> floating_image = imread(image_path)
    >>> image_path = data_path('time_1_cut.inr')
    >>> reference_image = imread(image_path)
    >>> trsf_rig, res_rig = registration(floating_image, reference_image, method='rigid_registration')
    >>> trsf_aff, res_aff = registration(floating_image, reference_image, method='affine_registration')
    >>> trsf_def, res_def = registration(floating_image, reference_image, method='deformable_registration')
    """
    # - Assert we have two SpatialImage:
    _input_img_check(floating_img)
    _input_img_check(reference_img)
    # - Set method if None and check it is a valid method:
    method = _method_check(method, POSS_METHODS, DEFAULT_METHOD)

    # - If provided 'init_trsf' will be used to initialize blockmatching
    # registration and the returned trsf will contain this trsf
    init_trsf = kwargs.pop('init_trsf', None)
    # - If provided 'left_trsf' will be used to initialize blockmatching
    # registration but returned trsf will NOT contain this trsf
    left_trsf = kwargs.pop('left_trsf', None)

    if init_trsf:
        try:
            assert left_trsf is None
        except:
            msg = "You cannot define both 'init_trsf' and 'left_trsf'!"
            raise ValueError(msg)

    # - Try to use the `plugin_function` or use the defined API:
    try:
        assert kwargs.get('try_plugin', False)
        from openalea.core.service.plugin import plugin_function
    except AssertionError or ImportError:
        if method == 'rigid_registration':
            trsf_out, res_image = rigid_registration(floating_img,
                                                     reference_img, init_trsf,
                                                     left_trsf, **kwargs)
            return trsf_out, res_image
        if method == 'affine_registration':
            trsf_out, res_image = affine_registration(floating_img,
                                                      reference_img, init_trsf,
                                                      left_trsf, **kwargs)
            return trsf_out, res_image
        if method == 'deformable_registration':
            trsf_out, res_image = deformable_registration(floating_img,
                                                          reference_img,
                                                          init_trsf, left_trsf,
                                                          **kwargs)
            return trsf_out, res_image
    else:
        func = plugin_function('openalea.image', method)
        if func is not None:
            print "WARNING: using 'plugin' functionality from 'openalea.core'!"
            return func(floating_img, reference_img, init_trsf, left_trsf,
                        **kwargs)
        else:
            raise NotImplementedError("Returned 'plugin_function' is None!")


def rigid_registration(floating_img, reference_img, init_trsf=None,
                       left_trsf=None, **kwargs):
    """
    Performs RIGID registration of `floating_img` on `reference_img` by calling
    `blockmatching` with adequate parameters.

    Parameters
    ----------
    floating_img : SpatialImage
        floating SpatialImage, ie. image to register
    reference_img : SpatialImage
        reference *SpatialImage* for registration
    init_trsf : BalTransformation, optional
        if provided (default None) the trsf will be used to initialize
        blockmatching registration and the returned trsf will contain this trsf
    left_trsf : BalTransformation, optional
        if provided (default None) the trsf will be used to initialize
        blockmatching registration but returned trsf will NOT contain this trsf

    **kwargs
    ------
    pyramid_lowest_level : int, optional
        lowest level at which to compute deformation, default is 1 (min is 0)
    pyramid_highest_level : int, optional
        highest level at which to compute deformation, default is 3 (max is 3)
    try_plugin : bool, optional
        manually control the use of openalea 'plugin' functionality, use
        it by default (True)

    Returns
    -------
    trsf_rig : BalTransformation
        estimated *BalTransformation* by registration process
    res_rig : SpatialImage
        interpolated *SpatialImage* after registration
    """
    param_str_2 = _registration_kwargs(**kwargs)

    # - Make sure the provided initialisation trsf matrix is linear:
    if init_trsf:
        try:
            assert init_trsf.is_linear()
        except:
            raise TypeError("Provided 'init_trsf' is not a linear deformation!")
    if left_trsf:
        try:
            assert left_trsf.is_linear()
        except:
            raise TypeError("Provided 'left_trsf' is not a linear deformation!")

    trsf_rig, res_rig = blockmatching(floating_img, reference_img,
                                      init_result_transformation=init_trsf,
                                      left_transformation=left_trsf,
                                      param_str_2=param_str_2)
    return trsf_rig, res_rig


def affine_registration(floating_img, reference_img, init_trsf=None,
                        left_trsf=None, **kwargs):
    """
    Performs AFFINE registration of `floating_img` on `reference_img` by calling
    `blockmatching` with adequate parameters.

    If no initial transformation is given ('init_trsf' or 'left_trsf'), first
    performs a rigid registration, then the affine deformation is estimated
    using the rigid trsf-matrix as initialisation ('init_trsf').

    If an initial transformation is given ('init_trsf' or 'left_trsf'), we
    use it to initialize the estimation of the affine deformation between the
    two images (ie. not rigid estimation first).

    Parameters
    ----------
    floating_img : SpatialImage
        floating SpatialImage, ie. image to register
    reference_img : SpatialImage
        reference *SpatialImage* for registration
    init_trsf : BalTransformation, optional
        if provided (default None) the trsf matrix will be used to initialize
        blockmatching registration and the returned trsf will contain this trsf
    left_trsf : BalTransformation, optional
        if provided (default None) the trsf matrix will be used to initialize
        blockmatching registration but returned trsf will NOT contain this trsf

    **kwargs
    ------
    pyramid_lowest_level : int, optional
        lowest level at which to compute deformation, default is 1 (min is 0)
    pyramid_highest_level : int, optional
        highest level at which to compute deformation, default is 3 (max is 3)
    try_plugin : bool, optional
        manually control the use of openalea 'plugin' functionality, use
        it by default (True)

    Returns
    -------
    trsf_aff : BalTransformation
        estimated *BalTransformation* by registration process
    res_aff : SpatialImage
        interpolated *SpatialImage* after registration
    """
    param_str_2 = _registration_kwargs(**kwargs)

    # - If no initialisation trsf-matrix were provided, initialise affine deformation estimation with rigid trsf:
    if not init_trsf or not left_trsf:
        trsf_rig, res_rig = rigid_registration(floating_img, reference_img,
                                               param_str_2=param_str_2)
        init_trsf = trsf_rig
    else:
        # - Make sure the provided initialisation trsf matrix is linear:
        if init_trsf:
            try:
                assert init_trsf.is_linear()
            except:
                raise TypeError(
                    "Provided 'init_trsf' is not a linear deformation!")
        if left_trsf:
            try:
                assert left_trsf.is_linear()
            except:
                raise TypeError(
                    "Provided 'left_trsf' is not a linear deformation!")

    param_str_2 += ' -trsf-type affine'
    # using 'left_transformation' returns an affine trsf matrix without the 'rigid deformation part!
    # ~ trsf_aff, res_aff = blockmatching(floating_img, reference_img, left_transformation=trsf_rig, param_str_2=param_str_2)
    trsf_aff, res_aff = blockmatching(floating_img, reference_img,
                                      init_result_transformation=init_trsf,
                                      left_transformation=left_trsf,
                                      param_str_2=param_str_2)

    return trsf_aff, res_aff


def deformable_registration(floating_img, reference_img, init_trsf=None,
                            left_trsf=None, **kwargs):
    """
    Performs NON-LINEAR registration of `floating_img` on `reference_img` by calling
    `blockmatching` with adequate parameters.

    If no initial transformation is given ('init_trsf' or 'left_trsf'), first
    performs a rigid registration, then the non-linear deformation is estimated
    using the rigid trsf-matrix as initialisation ('init_trsf').

    If an initial transformation is given ('init_trsf' or 'left_trsf'), we
    use it to initialize the estimation of the non-linear deformation between the
    two images (ie. not rigid estimation first).

    Parameters
    ----------
    floating_img : SpatialImage
        floating SpatialImage, ie. image to register
    reference_img : SpatialImage
        reference *SpatialImage* for registration
    init_trsf : BalTransformation, optional
        if provided (default None) the trsf matrix will be used to initialize
        blockmatching registration and the returned trsf will contain this trsf
    left_trsf : BalTransformation, optional
        if provided (default None) the trsf matrix will be used to initialize
        blockmatching registration but returned trsf will NOT contain this trsf

    **kwargs
    ------
    pyramid_lowest_level : int, optional
        lowest level at which to compute deformation, default is 1 (min is 0)
    pyramid_highest_level : int, optional
        highest level at which to compute deformation, default is 3 (max is 3)
    try_plugin : bool, optional
        manually control the use of openalea 'plugin' functionality, use
        it by default (True)

    Returns
    -------
    trsf_def : BalTransformation
        estimated *BalTransformation* by registration process
    res_def : SpatialImage
        interpolated *SpatialImage* after registration
    """
    param_str_2 = _registration_kwargs(**kwargs)

    # - If no initialisation trsf-matrix were provided, initialise non-linear deformation estimation with rigid trsf:
    if not init_trsf or not left_trsf:
        trsf_rig, res_rig = rigid_registration(floating_img, reference_img,
                                               param_str_2=param_str_2)
        init_trsf = trsf_rig

    param_str_2 += ' -trsf-type vectorfield'
    trsf_def, res_def = blockmatching(floating_img, reference_img,
                                      init_result_transformation=init_trsf,
                                      left_transformation=left_trsf,
                                      param_str_2=param_str_2)
    return trsf_def, res_def
