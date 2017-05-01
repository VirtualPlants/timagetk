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

import numpy as np
import itertools
try:
    from timagetk.wrapping.bal_trsf import BalTransformation
    from timagetk.algorithms import blockmatching
    from timagetk.algorithms import compose_trsf, apply_trsf, mean_trsfs, inv_trsf
    from timagetk.algorithms import mean_images
    from timagetk.components import SpatialImage
except ImportError:
    raise ImportError('Import Error')
__all__ = ['fusion']


def fusion(list_images, iterations=None, man_trsf_list=None, all_iter_mean_img=False):
    """
    Multiview reconstruction (registration)

    Parameters
    ----------
    :param list list_images: list of input ``SpatialImage``

    :param int iterations: number of iterations, optional. Default: 5

    :param list man_trsf_list: list of input ``BalTransformation``, optional
        Use it to define initial rigid or affine trsf matrix list obtained
         by manually registering all images (except the first) on the first one.
        They will be used as `init_result_transformation` for blockmaching
         FIRST iteration of rigid step. Should be in real units.
        Successively obtained `rig_trsf` will be used as
         `init_result_transformation` for blockmaching REMAINING
          iterations of rigid step.

    :param bool all_iter_mean_img: should all averaged image from 3-steps
         registration iterations be returned, optional. Default: False

    Returns
    ----------
    :return: ``SpatialImage`` instance -- image and metadata

    Example
    -------
    >>> from timagetk.util import data_path
    >>> from timagetk.components import imread
    >>> from timagetk.algorithms import fusion
    >>> vues = [0, 1, 2]
    >>> list_images = [imread(data_path('fusion_img_' + str(vue) + '.inr'))
                       for vue in vues]
    >>> fus_img = fusion(list_images)
    """
    if iterations is None:
        iterations = 5
    else:
        iterations = int(abs(iterations))
    #--- check: list of SpatialImage images
    conds_init = isinstance(list_images, list) and len(list_images)>=2
    conds_list_img = [0 if isinstance(sp_img, SpatialImage)
                      else 1 for sp_img in list_images]
    #--- check: list of BalTransformation trsf
    if man_trsf_list is None:
        conds_list_trsf = []
        man_trsf_list = [None]*(len(list_images)-1)
    else:
        conds_init = conds_init and isinstance(list_images, list) and len(list_images)-1==len(man_trsf_list)
        conds_list_trsf = [0 if isinstance(trsf, BalTransformation)
                          else 1 for trsf in man_trsf_list]
    #--- end check
    if conds_init and 1 not in conds_list_img and 1 not in conds_list_trsf:

        succ_ref_img = []
        vox_list = [sp_img.get_voxelsize() for sp_img in list_images]
        vox_list = [i for i in itertools.chain.from_iterable(vox_list)] # voxel list
        ext_list = [sp_img.get_extent() for sp_img in list_images]
        ext_list = [i for i in itertools.chain.from_iterable(ext_list)] # extent list

        # -- Use first image in list as reference template:
        if list_images[0].get_dim() == 3:
            min_vox, val = np.min(vox_list), int(np.max(ext_list)/np.min(vox_list))
            tmp_arr = np.zeros((val, val, val), dtype=list_images[0].dtype)
            template_img = SpatialImage(tmp_arr, voxelsize=[min_vox, min_vox, min_vox])
        # -- Use first image in list to initialise reference template (ref. image for first round of blockmatching):
        init_ref = apply_trsf(list_images[0], bal_transformation=None, template_img=template_img)
        succ_ref_img.append(init_ref)

        # -- FIRST iteration of 3-steps registrations: rigid, affine & vectorfield
        # All images in 'list_images' are registered on the first of the list
        init_trsf_list, init_img_list = [], []
        for ind, sp_img in enumerate(list_images):
            if ind>0: #
                trsf_rig, res_rig = blockmatching(sp_img, init_ref,
                                                  init_result_transformation=man_trsf_list[ind-1],
                                                  param_str_2='-trsf-type rigid -py-ll 1')
                # - Update the `man_trsf_list` for the next iteration
                if man_trsf_list[ind-1] is not None:
                    man_trsf_list[ind-1] = trsf_rig
                trsf_aff, res_aff = blockmatching(sp_img, init_ref,
                                                  left_transformation=trsf_rig,
                                                  param_str_2='-trsf-type affine')

                tmp_trsf = compose_trsf([trsf_rig, trsf_aff])
                trsf_def, res_def = blockmatching(sp_img, init_ref,
                                                  init_result_transformation=tmp_trsf,
                                                  param_str_2='-trsf-type vectorfield')

                out_trsf = BalTransformation(c_bal_trsf=trsf_def)
                init_trsf_list.append(out_trsf)
                init_img_list.append(res_def)
        # - Add the reference image to the list of images that will be averaged:
        init_img_list.append(init_ref)
        # - Computing the mean image from previous 3-steps registrations
        mean_ref = mean_images(init_img_list)
        # - Computing the mean transformation from previous 3-steps registrations
        mean_trsf = mean_trsfs(init_trsf_list)
        # - Computing the inverted mean transformation:
        mean_trsf_inv = inv_trsf(mean_trsf)
        # - Apply it to the mean image and add this invTrsf_image to a list:
        mean_ref_update = apply_trsf(mean_ref, mean_trsf_inv, template_img=template_img)
        succ_ref_img.append(mean_ref_update)

        # -- REMAINING iterations of 3-steps registrations: rigid, affine & vectorfield
        # For each iteration we use `mean_ref_update` from previous round of registration
        # REMEMBER: the floating images are the ORIGINAL images from `list_images`!!
        # Now even the first image from `list_images` is 3-steps registered !!
        man_trsf_list = [None] + man_trsf_list
        for index in range(0, iterations):
            # Again, all images in 'list_images' are registered on the first of the list
            init_trsf_list, init_img_list = [], []
            for ind, sp_img in enumerate(list_images):
                trsf_rig, res_rig = blockmatching(sp_img, mean_ref_update,
                                                  init_result_transformation=man_trsf_list[ind],
                                                  param_str_2='-trsf-type rigid -py-ll 1')
                # - Update the `man_trsf_list` for the next iteration
                if man_trsf_list[ind] is not None:
                    man_trsf_list[ind] = trsf_rig
                trsf_aff, res_aff = blockmatching(sp_img, mean_ref_update,
                                                  left_transformation=trsf_rig,
                                                  param_str_2='-trsf-type affine')
                tmp_trsf = compose_trsf([trsf_rig, trsf_aff])
                trsf_def, res_def = blockmatching(sp_img, mean_ref_update,
                                                  init_result_transformation=tmp_trsf,
                                                  param_str_2='-trsf-type vectorfield')
                out_trsf = BalTransformation(c_bal_trsf=trsf_def)
                init_trsf_list.append(out_trsf)
                init_img_list.append(res_def)
            # - Add the mean reference image of this round to the list of images that will be averaged:
            init_img_list.append(mean_ref_update)
            # - Compute the mean_image, mean_trsf, mean_trsf_inv & mean_ref_update
            mean_ref = mean_images(init_img_list)
            mean_trsf = mean_trsfs(init_trsf_list)
            mean_trsf_inv = inv_trsf(mean_trsf)
            # This update the reference image for the next round!
            mean_ref_update = apply_trsf(mean_ref, mean_trsf_inv, template_img=template_img)
            succ_ref_img.append(mean_ref_update)
        if all_iter_mean_img:
            return succ_ref_img
        else:
            return succ_ref_img[-1] # why keep them all (without using them) and returning the last one?
    else:
        print('Incorrect specifications:')
        print "Specs: conds_init={}; conds_list_img={}; conds_list_trsf={}".format(conds_init, conds_list_img, conds_list_trsf)
        return
