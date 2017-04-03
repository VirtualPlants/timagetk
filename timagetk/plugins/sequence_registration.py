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
This module contains a generic implementation of several sequence registration algorithms.
"""

try:
    from timagetk.algorithms import blockmatching
    from timagetk.algorithms import compose_trsf, apply_trsf
    from timagetk.components import SpatialImage
except ImportError:
    raise ImportError('Import Error')

__all__ = ['sequence_registration']


def sequence_registration(list_images, method=None, **kwds):
    """
    Sequence registration plugin. Available methods are :

    * sequence_rigid_registration
    * sequence_affine_registration

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
    poss_methods = ['sequence_rigid_registration', 'sequence_affine_registration']
    conds_init = isinstance(list_images, list) and len(list_images)>=3
    conds_list_img = [0 if isinstance(sp_img, SpatialImage) else 1
                        for sp_img in list_images]
    #--- end check
    if conds_init and 1 not in conds_list_img:
        if method is None:
            list_compo_trsf, list_res_img = sequence_registration_rigid(list_images)
            return list_compo_trsf, list_res_img
        elif method is not None:
            if method in poss_methods:
                try:
                    from openalea.core.service.plugin import plugin_function
                    func = plugin_function('openalea.image', method)
                    if func is not None:
                        return func(list_images, **kwds)
                except:
                    if method=='sequence_rigid_registration':
                        list_compo_trsf, list_res_img = sequence_registration_rigid(list_images)
                    elif method=='sequence_affine_registration':
                        list_compo_trsf, list_res_img = sequence_registration_affine(list_images)
                    return list_compo_trsf, list_res_img
            else:
                print('Available methods :'), poss_methods
                raise NotImplementedError(method)
    else:
        raise TypeError('list_images must be a list of SpatialImage')
        return


def sequence_registration_rigid(list_images):
    #--- check SpatialImage sequence
    conds_init = isinstance(list_images, list) and len(list_images)>=3
    conds_list_img = [0 if isinstance(sp_img, SpatialImage) else 1
                        for sp_img in list_images]
    #--- end check
    if conds_init and 1 not in conds_list_img:
        #--- pairwise registration
        pairwise_trsf = []
        for ind, sp_img in enumerate(list_images):
            if ind < len(list_images)-1:
                # --- rigid registration
                trsf_rig, res_rig = blockmatching(list_images[ind], list_images[ind+1],
                                                  param_str_2='-trsf-type rigid')
                pairwise_trsf.append(trsf_rig) #--- case rigid registration
        #--- composition of transformations
        list_compo_trsf, list_res_img = [], []
        for ind, trsf in enumerate(pairwise_trsf):
            if ind < len(pairwise_trsf)-1:
                comp_trsf = compose_trsf(pairwise_trsf[ind:]) # matrix multiplication
                list_compo_trsf.append(comp_trsf)
            elif ind == len(pairwise_trsf)-1:
                list_compo_trsf.append(pairwise_trsf[-1])
        #--- displacements compensation (whole sequence)
        for ind, trsf in enumerate(list_compo_trsf):
            tmp_img = apply_trsf(list_images[ind], trsf, template_img=list_images[-1]) # global reference : list_images[-1]
            list_res_img.append(tmp_img)
        list_res_img.append(list_images[-1]) # add global reference image
    return list_compo_trsf, list_res_img


def sequence_registration_affine(list_images):
    #--- check SpatialImage sequence
    conds_init = isinstance(list_images, list) and len(list_images)>=3
    conds_list_img = [0 if isinstance(sp_img, SpatialImage) else 1
                        for sp_img in list_images]
    #--- end check
    if conds_init and 1 not in conds_list_img:
        #--- pairwise registration
        pairwise_trsf = []
        for ind, sp_img in enumerate(list_images):
            if ind < len(list_images)-1:
                # --- rigid registration
                trsf_rig, res_rig = blockmatching(list_images[ind], list_images[ind+1],
                                                  param_str_2='-trsf-type rigid -py-ll 1')
                # --- affine registration, initialisation by a rigid transformation
                trsf_aff, res_aff = blockmatching(list_images[ind], list_images[ind+1],
                                                  left_transformation=trsf_rig,
                                                  param_str_2='-trsf-type affine')
                res_trsf = compose_trsf([trsf_rig, trsf_aff]) #--- composition of transformations
                pairwise_trsf.append(res_trsf) #--- case affine registration
                trsf_rig.free() #--- add
        #--- composition of transformations
        list_compo_trsf, list_res_img = [], []
        for ind, trsf in enumerate(pairwise_trsf):
            if ind < len(pairwise_trsf)-1:
                tmp_trsf = compose_trsf(pairwise_trsf[ind:]) # matrix multiplication
                list_compo_trsf.append(tmp_trsf)
            elif ind == pairwise_trsf.index(pairwise_trsf[-1]):
                list_compo_trsf.append(pairwise_trsf[-1])
        #--- displacements compensation (whole sequence)
        for ind, trsf in enumerate(list_compo_trsf):
            tmp_img = apply_trsf(list_images[ind], trsf, template_img=list_images[-1])
            list_res_img.append(tmp_img)
        list_res_img.append(list_images[-1]) # add reference image
    return list_compo_trsf, list_res_img
