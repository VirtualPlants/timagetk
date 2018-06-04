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
This module contains a generic implementation of several sequence registration algorithms.
"""

try:
    from timagetk.algorithms import blockmatching
    from timagetk.algorithms import compose_trsf, apply_trsf
    from timagetk.components import SpatialImage
except ImportError:
    raise ImportError('Import Error')

__all__ = ['sequence_registration']

POSS_METHODS = ['sequence_rigid_registration', 'sequence_affine_registration',
                'sequence_deformable_registration']
DEFAULT_METHOD = POSS_METHODS[0]


def sequence_registration(list_images, method=None, **kwds):
    """
    Sequence registration plugin. Available methods are :

    * sequence_rigid_registration
    * sequence_affine_registration
    * sequence_deformable_registration

    Parameters
    ----------
    :param list list_images: list of *SpatialImage*

    :param str method: used method (example: 'sequence_registration_rigid')

    Returns
    ----------
    :return: list_compo_trsf -- list of *BalTransformation* transformation

    :return: list_res_img -- list of *SpatialImage*

    Example
    ----------
    >>> from timagetk.util import data_path
    >>> from timagetk.components import imread
    >>> from timagetk.plugins import sequence_registration
    >>> times = [0, 1, 2]
    >>> list_images = [imread(data_path('time_' + str(time) + '.inr'))
                        for time in times]
    >>> list_compo_trsf, list_res_img = sequence_registration(list_images,
                                                              method='sequence_rigid_registration')
    >>> list_compo_trsf, list_res_img = sequence_registration(list_images,
                                                              method='sequence_affine_registration')
    """
    # - Check list_images type:
    try:
        assert isinstance(list_images, list)
    except AssertionError:
        raise TypeError(
            "Parameter 'list_images' should be of type 'list', but is: {}".format(
                type(list_images)))
    # - Check SpatialImage sequence:
    conds_list_img = [isinstance(img, SpatialImage) for img in list_images]
    try:
        assert sum(conds_list_img) == len(conds_list_img)
    except AssertionError:
        raise TypeError(
            "Parameter 'list_images' should be a list of SpatialImages!")
    # - Check SpatialImage sequence length, this function is useless if length < 3!
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
        if method == 'sequence_rigid_registration':
            list_compo_trsf, list_res_img = sequence_registration_rigid(
                list_images)
        elif method == 'sequence_affine_registration':
            list_compo_trsf, list_res_img = sequence_registration_affine(
                list_images)
        elif method == 'sequence_deformable_registration':
            list_compo_trsf, list_res_img = sequence_registration_deformable(
                list_images)
        else:
            raise NotImplementedError(
                "Unknown method: {}".format(method))
        return list_compo_trsf, list_res_img
    else:
        func = plugin_function('openalea.image', method)
        if func is not None:
            print "WARNING: using 'plugin' functionality from 'openalea.core'!"
            return func(list_images, **kwds)
        else:
            raise NotImplementedError("Returned 'plugin_function' is None!")



def sequence_registration_rigid(list_images):
    """
    Performs a rigid sequence registration on the last element of the image
    list.

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
    # --- pairwise registration
    pairwise_trsf = []
    for ind, sp_img in enumerate(list_images):
        if ind < len(list_images) - 1:
            # --- rigid registration
            trsf_rig, res_rig = blockmatching(list_images[ind],
                                              list_images[ind + 1],
                                              param_str_2='-trsf-type rigid')
            pairwise_trsf.append(trsf_rig)  # --- case rigid registration
    # --- composition of transformations
    list_compo_trsf, list_res_img = [], []
    for ind, trsf in enumerate(pairwise_trsf):
        if ind < len(pairwise_trsf) - 1:
            comp_trsf = compose_trsf(
                pairwise_trsf[ind:])  # matrix multiplication
            list_compo_trsf.append(comp_trsf)
        elif ind == len(pairwise_trsf) - 1:
            list_compo_trsf.append(pairwise_trsf[-1])
    # --- displacements compensation (whole sequence)
    for ind, trsf in enumerate(list_compo_trsf):
        tmp_img = apply_trsf(list_images[ind], trsf,
                             template_img=list_images[
                                 -1])  # global reference : list_images[-1]
        list_res_img.append(tmp_img)
    list_res_img.append(list_images[-1])  # add global reference image

    return list_compo_trsf, list_res_img


def sequence_registration_affine(list_images):
    """
    Performs an affine sequence registration on the last element of the image
    list.

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
    # --- pairwise registration
    pairwise_trsf = []
    for ind, sp_img in enumerate(list_images):
        if ind < len(list_images) - 1:
            # --- rigid registration
            trsf_rig, res_rig = blockmatching(list_images[ind],
                                              list_images[ind + 1],
                                              param_str_2='-trsf-type rigid -py-ll 1')
            # --- affine registration, initialisation by a rigid transformation
            trsf_aff, res_aff = blockmatching(list_images[ind],
                                              list_images[ind + 1],
                                              left_transformation=trsf_rig,
                                              param_str_2='-trsf-type affine')
            res_trsf = compose_trsf(
                [trsf_rig, trsf_aff])  # --- composition of transformations
            pairwise_trsf.append(res_trsf)  # --- case affine registration
            trsf_rig.free()  # --- add
    # --- composition of transformations
    list_compo_trsf, list_res_img = [], []
    for ind, trsf in enumerate(pairwise_trsf):
        if ind < len(pairwise_trsf) - 1:
            tmp_trsf = compose_trsf(
                pairwise_trsf[ind:])  # matrix multiplication
            list_compo_trsf.append(tmp_trsf)
        elif ind == pairwise_trsf.index(pairwise_trsf[-1]):
            list_compo_trsf.append(pairwise_trsf[-1])
    # --- displacements compensation (whole sequence)
    for ind, trsf in enumerate(list_compo_trsf):
        tmp_img = apply_trsf(list_images[ind], trsf,
                             template_img=list_images[-1])
        list_res_img.append(tmp_img)
    list_res_img.append(list_images[-1])  # add reference image

    return list_compo_trsf, list_res_img


def sequence_registration_deformable(list_images):
    """
    Performs a deformable sequence registration on the last element of the image 
    list.

    Parameters
    ----------
    list_images : list(SpatialImage)
        list of time sorted SpatialImage

    Returns
    -------
    list_compo_trsf : list(BalTransformation)
        list of composed transformation
    list_res_img : list(SpatialImage)
        list of images after deformable registration
    """
    # - Pairwise registration: 't_n' on 't_n+1'
    pairwise_trsf = []
    for ind, sp_img in enumerate(list_images[:-1]):
        # -- Rigid registration
        print "\nPerforming rigid registration of t{} on t{}:".format(ind,
                                                                      ind + 1)
        trsf_rig, res_rig = blockmatching(list_images[ind],
                                          list_images[ind + 1],
                                          param_str_2='-trsf-type rigid -py-ll 1')
        print "Done.\n"
        # -- Deformable registration, trsf initialisation by a rigid transformation
        print "Performing deformable registration of t{} on t{}:".format(ind,
                                                                         ind + 1)
        trsf_vf, res_vf = blockmatching(list_images[ind], list_images[ind + 1],
                                        left_transformation=trsf_rig,
                                        param_str_2='-trsf-type vectorfield')
        print "Done.\n"
        print "Composition of rigid and deformable registration..."
        res_trsf = compose_trsf([trsf_rig, trsf_vf], template_img=list_images[
            -1])  # --- composition of transformations
        pairwise_trsf.append(res_trsf)
        trsf_rig.free()
        print "Done.\n"

    # - Composition of composed (pairwise) transformations to create all 't_n'/'t_ref' trsf, where t_ref is the last time-point
    list_compo_trsf, list_res_img = [], []
    for ind, trsf in enumerate(pairwise_trsf):
        if ind < len(pairwise_trsf) - 1:
            tmp_trsf = compose_trsf(pairwise_trsf[ind:],
                                    template_img=list_images[
                                        -1])  # matrix multiplication
            list_compo_trsf.append(tmp_trsf)
        else:
            # 't_ref-1'/'t_ref' transformation does not need composition!
            list_compo_trsf.append(pairwise_trsf[-1])

    # - Apply list of transformation to list of images, except 't_ref' (whole sequence registration on last time-point)
    for ind, trsf in enumerate(list_compo_trsf):
        tmp_img = apply_trsf(list_images[ind], trsf,
                             template_img=list_images[-1])
        list_res_img.append(tmp_img)

    list_res_img.append(list_images[-1])  # add reference image 't_ref'

    return list_compo_trsf, list_res_img
