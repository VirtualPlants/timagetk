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
This module contains a generic implementation of several consecutive registration algorithms.
"""

try:
    from timagetk.algorithms import blockmatching
    from timagetk.algorithms import compose_trsf, apply_trsf
    from timagetk.components import SpatialImage
except ImportError:
    raise ImportError('Import Error')

__all__ = ['consecutive_registration']

POSS_METHODS = ['consecutive_rigid_registration',
                'consecutive_affine_registration',
                'consecutive_deformable_registration']
DEFAULT_METHOD = POSS_METHODS[0]


def consecutive_registration(list_images, method=None, **kwds):
    """
    Consecutive registration plugin. Available methods are :

    * consecutive_rigid_registration
    * consecutive_affine_registration
    * consecutive_deformable_registration

    Parameters
    ----------
    :param list list_images: list of *SpatialImage*

    :param str method: used method (example: 'consecutive_registration_rigid')

    Returns
    ----------
    :return: list_compo_trsf -- list of *BalTransformation* transformation

    :return: list_res_img -- list of *SpatialImage*

    Example
    ----------
    >>> from timagetk.util import data_path
    >>> from timagetk.components import imread
    >>> from timagetk.plugins import consecutive_registration
    >>> times = [0, 1, 2]
    >>> list_images = [imread(data_path('time_' + str(time) + '.inr'))
                        for time in times]
    >>> list_compo_trsf, list_res_img = consecutive_registration(list_images,
                                                              method='consecutive_rigid_registration')
    >>> list_compo_trsf, list_res_img = consecutive_registration(list_images,
                                                              method='consecutive_affine_registration')
    """
    # - Check list_images type:
    try:
        assert isinstance(list_images, list)
    except AssertionError:
        raise TypeError(
            "Parameter 'list_images' should be of type 'list', but is: {}".format(
                type(list_images)))
    # - Check SpatialImage consecutive:
    conds_list_img = [isinstance(img, SpatialImage) for img in list_images]
    try:
        assert sum(conds_list_img) == len(conds_list_img)
    except AssertionError:
        raise TypeError(
            "Parameter 'list_images' should be a list of SpatialImages!")
    # - Check SpatialImage consecutive length, this function is useless if length < 3!
    try:
        assert len(list_images) >= 3
    except AssertionError:
        raise ValueError(
            "Parameter 'list_images' should have a minimum length of 3!")

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
        if method == 'consecutive_rigid_registration':
            list_compo_trsf, list_res_img = consecutive_registration_rigid(
                list_images, )
        elif method == 'consecutive_affine_registration':
            list_compo_trsf, list_res_img = consecutive_registration_affine(
                list_images)
        elif method == 'consecutive_deformable_registration':
            list_compo_trsf, list_res_img = consecutive_registration_deformable(
                list_images)
        else:
            raise NotImplementedError(
                "Unknown method: {}".format(method))
        return list_compo_trsf, list_res_img
    else:
        func = plugin_function('openalea.image', method)
        if func:
            print "WARNING: using 'plugin' functionality from 'openalea.core'!"
            return func(list_images, **kwds)
        else:
            raise NotImplementedError("Returned 'plugin_function' is None!")


def consecutive_registration_rigid(list_images):
    """
    Performs a rigid consecutive registration of given images on the following
    one in the image list.

    Parameters
    ----------
    list_images : list(SpatialImage)
        list of time sorted SpatialImage

    Returns
    -------
    list_compo_trsf : list(BalTransformation)
        list of composed transformation
    list_res_img : list(SpatialImage)
        list of images after rigid registration
    """
    # --- consecutive pairwise registration
    list_res_trsf = []
    list_res_img = []
    for ind, sp_img in enumerate(list_images):
        if ind < len(list_images) - 1:
            # --- rigid registration
            print "\nPerforming rigid registration of t{} on t{}:".format(ind,
                                                                          ind + 1)
            trsf_rig, res_rig = blockmatching(sp_img,
                                              list_images[ind + 1],
                                              param_str_2='-trsf-type rigid')
            list_res_trsf.append(trsf_rig)
            list_res_img.append(res_rig)
    # add last reference image
    list_res_img.append(list_images[-1])

    return list_res_trsf, list_res_img


def consecutive_registration_affine(list_images):
    """
    Performs a affine consecutive registration of given images on the following
    one in the image list.

    Parameters
    ----------
    list_images : list(SpatialImage)
        list of time sorted SpatialImage

    Returns
    -------
    list_compo_trsf : list(BalTransformation)
        list of composed transformation
    list_res_img : list(SpatialImage)
        list of images after affine registration
    """
    # --- consecutive pairwise registration
    list_res_trsf = []
    list_res_img = []
    for ind, sp_img in enumerate(list_images):
        if ind < len(list_images) - 1:
            # --- rigid registration
            print "\nPerforming rigid registration of t{} on t{}:".format(ind,
                                                                          ind + 1)
            trsf_rig, res_rig = blockmatching(sp_img,
                                              list_images[ind + 1],
                                              param_str_2='-trsf-type rigid -py-ll 1')
            # --- affine registration, initialisation by a rigid transformation
            print "\nPerforming affine registration of t{} on t{}:".format(ind,
                                                                           ind + 1)
            trsf_aff, res_aff = blockmatching(sp_img,
                                              list_images[ind + 1],
                                              left_transformation=trsf_rig,
                                              param_str_2='-trsf-type affine')
            # --- composition of transformations
            res_trsf = compose_trsf([trsf_rig, trsf_aff],
                                    template_img=list_images[-1])
            list_res_trsf.append(res_trsf)
            list_res_img.append(apply_trsf(sp_img, res_trsf))
    # add last reference image
    list_res_img.append(list_images[-1])  # add last reference image

    return list_res_trsf, list_res_img


def consecutive_registration_deformable(list_images):
    """
    Performs a deformable consecutive registration of given images on the following
    one in the image list.

    Parameters
    ----------
    list_images : list(SpatialImage)
        list of time sorted SpatialImage

    Returns
    -------
    list_compo_trsf : list(BalTransformation)
        list of composed transformation
    list_res_img : list(SpatialImage)
        list of images after affine registration
    """
    # --- consecutive pairwise registration
    list_res_trsf = []
    list_res_img = []
    for ind, sp_img in enumerate(list_images):
        if ind < len(list_images) - 1:
            # --- rigid registration
            print "\nPerforming rigid registration of t{} on t{}:".format(ind,
                                                                          ind + 1)
            trsf_rig, res_rig = blockmatching(sp_img,
                                              list_images[ind + 1],
                                              param_str_2='-trsf-type rigid -py-ll 1')
            print "\nPerforming deformable registration of t{} on t{}:".format(
                ind, ind + 1)
            # --- deformable registration, initialisation by a rigid transformation
            trsf_vf, res_vf = blockmatching(sp_img,
                                            list_images[ind + 1],
                                            left_transformation=trsf_rig,
                                            param_str_2='-trsf-type vectorfield')
            # --- composition of transformations
            res_trsf = compose_trsf([trsf_rig, trsf_vf],
                                    template_img=list_images[-1])
            list_res_trsf.append(res_trsf)
            # -- application de la composition des transformations sur l'image
            list_res_img.append(apply_trsf(sp_img, res_trsf))
    # add last reference image
    list_res_img.append(list_images[-1])  # add last reference image

    return list_res_trsf, list_res_img
