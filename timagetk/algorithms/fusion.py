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


def fusion(list_images, iterations=None):
    """
    Multiview reconstruction (registration)

    Parameters
    ----------
    :param list list_images: list of input ``SpatialImage``

    :param int iterations: number of iterations, optional. Default: 5

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
    #--- end check
    if conds_init and 1 not in conds_list_img:

        succ_ref_img = []
        vox_list = [sp_img.get_voxelsize() for sp_img in list_images]
        vox_list = [i for i in itertools.chain.from_iterable(vox_list)] # voxel list
        ext_list = [sp_img.get_extent() for sp_img in list_images]
        ext_list = [i for i in itertools.chain.from_iterable(ext_list)] # extent list

        if list_images[0].get_dim() == 3:
            min_vox, val = np.min(vox_list), int(np.max(ext_list)/np.min(vox_list))
            tmp_arr = np.zeros((val, val, val), dtype=list_images[0].dtype)
            template_img = SpatialImage(tmp_arr, voxelsize=[min_vox, min_vox, min_vox])

        init_ref = apply_trsf(list_images[0], bal_transformation=None, template_img=template_img)
        succ_ref_img.append(init_ref)

        init_trsf_list, init_img_list = [], []
        for ind, sp_img in enumerate(list_images):
            if ind>0:
                trsf_rig, res_rig = blockmatching(sp_img, init_ref,
                                                  param_str_2='-trsf-type rigid -py-ll 1')

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
        init_img_list.append(init_ref)
        mean_ref = mean_images(init_img_list)
        mean_trsf = mean_trsfs(init_trsf_list)
        mean_trsf_inv = inv_trsf(mean_trsf)
        mean_ref_update = apply_trsf(mean_ref, mean_trsf_inv, template_img=template_img)
        succ_ref_img.append(mean_ref_update)
        for index in range(0, iterations):
            init_trsf_list, init_img_list = [], []
            for ind, sp_img in enumerate(list_images):
                trsf_rig, res_rig = blockmatching(sp_img, mean_ref_update,
                                                  param_str_2='-trsf-type rigid -py-ll 1')
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
            init_img_list.append(mean_ref_update)
            mean_ref = mean_images(init_img_list)
            mean_trsf = mean_trsfs(init_trsf_list)
            mean_trsf_inv = inv_trsf(mean_trsf)
            mean_ref_update = apply_trsf(mean_ref, mean_trsf_inv, template_img=template_img)
            succ_ref_img.append(mean_ref_update)
        return succ_ref_img[-1]
    else:
        print('Incorrect specification')
        return