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
This module contains a generic implementation of several registration algorithms.
"""

try:
    from timagetk.algorithms import blockmatching
    from timagetk.components import SpatialImage
    from timagetk.algorithms.blockmatching import BLOCKMATCHING_DEFAULT
except ImportError:
    raise ImportError('Import Error')

__all__ = ['registration']

POSS_METHODS = ['rigid_registration', 'affine_registration',
                'deformable_registration']
DEFAULT_METHOD = POSS_METHODS[0]


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

    **kwds
    ------
    pyramid_lowest_level : int, optional
        lowest level at which to compute deformation, default is 1 (min is 0)
    pyramid_highest_level : int, optional
        highest level at which to compute deformation, default is 3 (max is 3)
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
    # - Assert we have two SpatialImage:
    try:
        assert isinstance(floating_img, SpatialImage)
    except:
        raise TypeError('Floating image must be a SpatialImage instance.')
    try:
        assert isinstance(reference_img, SpatialImage)
    except:
        raise TypeError('Reference image must be a SpatialImage instance.')

    # - Use the default `method`:
    if method is None:
        method = DEFAULT_METHOD
    try:
        assert method in POSS_METHODS
    except AssertionError:
        raise NotImplementedError(
            "Unknown method '{}', available methods are: {}".format(method,
                                                                    POSS_METHODS))
    # - If provided 'init_trsf' will be used to initialize blockmatching
    # registration and the returned trsf will contain this trsf
    init_trsf = kwds.pop('init_trsf', None)
    # - If provided 'left_trsf' will be used to initialize blockmatching
    # registration but returned trsf will NOT contain this trsf
    left_trsf = kwds.pop('left_trsf', None)

    if init_trsf:
        try:
            assert left_trsf is None
        except:
            raise ValueError(
                "You cannot define both 'init_trsf' and 'left_trsf', please choose!")

    # - Try to use the `plugin_function` or use the defined API:
    try:
        assert kwds.get('try_plugin', False)
        from openalea.core.service.plugin import plugin_function
    except AssertionError or ImportError:
        if method == 'rigid_registration':
            trsf_out, res_image = rigid_registration(floating_img,
                                                     reference_img, init_trsf,
                                                     left_trsf, **kwds)
            return trsf_out, res_image
        if method == 'affine_registration':
            trsf_out, res_image = affine_registration(floating_img,
                                                      reference_img, init_trsf,
                                                      left_trsf, **kwds)
            return trsf_out, res_image
        if method == 'deformable_registration':
            trsf_out, res_image = deformable_registration(floating_img,
                                                          reference_img,
                                                          init_trsf, left_trsf,
                                                          **kwds)
            return trsf_out, res_image
    else:
        func = plugin_function('openalea.image', method)
        if func is not None:
            print "WARNING: using 'plugin' functionality from 'openalea.core'!"
            return func(floating_img, reference_img, init_trsf, left_trsf,
                        **kwds)
        else:
            raise NotImplementedError("Returned 'plugin_function' is None!")


def get_param_str_2(**kwds):
    """
    Set parameters default values and make sure they are of the right type.
    """
    str_param = ""
    # - By default 'pyramid_lowest_level' is equal to 1:
    str_param += ' -pyramid-lowest-level %d' % (
        kwds.get('pyramid_lowest_level', 1))
    # - By default 'pyramid_highest_level' is equal to 3:
    str_param += ' -pyramid-highest-level %d' % (
        kwds.get('pyramid_highest_level', 3))
    # TODO: make shorter version of param names available ? ie. py_ll & py_hl ?

    # - Providing 'param' will result in printing blockmatching parameters
    if kwds.get('param', False):
        str_param += ' -param'
    # - Providing 'command_line' will result in
    if kwds.get('command_line', False):
        str_param += ' -command-line'
    # - Providing 'verbose' will result in increased verbosity of the blockmatching code
    if kwds.get('verbose', False):
        str_param += ' -verbose'
    # - Providing 'parallel' will result in using parallelization capabilities
    if kwds.get('parallel', True):
        str_param += ' -parallel'
        str_param += ' -parallel-type ' + \
                     kwds.get('parallel_type', 'thread')
    # - Providing 'time' will result in printing CPU & User elapsed time
    if kwds.get('time', True):
        str_param += ' -time'

    return str_param


def rigid_registration(floating_img, reference_img, init_trsf=None,
                       left_trsf=None, **kwds):
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

    **kwds
    ------
    pyramid_lowest_level : int, optional
        lowest level at which to compute deformation, default is 1 (min is 0)
    pyramid_highest_level : int, optional
        highest level at which to compute deformation, default is 3 (max is 3)
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
    -------
    trsf_rig : BalTransformation
        estimated *BalTransformation* by registration process
    res_rig : SpatialImage
        interpolated *SpatialImage* after registration
    """
    param_str_2 = get_param_str_2(**kwds)

    # - Make sure the provided initialisation trsf matrix is linear:
    if init_trsf:
        try:
            assert init_trsf.isLinear()
        except:
            raise TypeError("Provided 'init_trsf' is not a linear deformation!")
    if left_trsf:
        try:
            assert left_trsf.isLinear()
        except:
            raise TypeError("Provided 'left_trsf' is not a linear deformation!")

    trsf_rig, res_rig = blockmatching(floating_img, reference_img,
                                      init_result_transformation=init_trsf,
                                      left_transformation=left_trsf,
                                      param_str_2=param_str_2)
    return trsf_rig, res_rig


def affine_registration(floating_img, reference_img, init_trsf=None,
                        left_trsf=None, **kwds):
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

    **kwds
    ------
    pyramid_lowest_level : int, optional
        lowest level at which to compute deformation, default is 1 (min is 0)
    pyramid_highest_level : int, optional
        highest level at which to compute deformation, default is 3 (max is 3)
    param : bool, optional
        print parameters, default is False
    parallel : bool, optional
        set usage of parallel mode, default is True
    parallel-type : str, optional in ['none'|'openmp'|'omp'|'pthread'|'thread']
        type of parrallelism to use, default is thread
    time : bool, optional
        print elapsed time, default is True
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
    param_str_2 = get_param_str_2(**kwds)

    # - If no initialisation trsf-matrix were provided, initialise affine deformation estimation with rigid trsf:
    if not init_trsf or not left_trsf:
        trsf_rig, res_rig = rigid_registration(floating_img, reference_img,
                                               param_str_2=param_str_2)
        init_trsf = trsf_rig
    else:
        # - Make sure the provided initialisation trsf matrix is linear:
        if init_trsf:
            try:
                assert init_trsf.isLinear()
            except:
                raise TypeError(
                    "Provided 'init_trsf' is not a linear deformation!")
        if left_trsf:
            try:
                assert left_trsf.isLinear()
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
                            left_trsf=None, **kwds):
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

    **kwds
    ------
    pyramid_lowest_level : int, optional
        lowest level at which to compute deformation, default is 1 (min is 0)
    pyramid_highest_level : int, optional
        highest level at which to compute deformation, default is 3 (max is 3)
    param : bool, optional
        print parameters, default is False
    parallel : bool, optional
        set usage of parallel mode, default is True
    parallel-type : str, optional in ['none'|'openmp'|'omp'|'pthread'|'thread']
        type of parrallelism to use, default is thread
    time : bool, optional
        print elapsed time, default is True
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
    param_str_2 = get_param_str_2(**kwds)

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
