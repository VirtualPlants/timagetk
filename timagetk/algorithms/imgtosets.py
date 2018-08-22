# -*- coding: utf-8 -*-
# -*- python -*-
#
#
#       Copyright 2016 INRIA
#
#       File author(s):
#           Sophie Ribes <sophie.ribes@inria.fr>
#
#       See accompanying file LICENSE.txt
#------------------------------------------------------------------------------
import numpy as np
try:
    from timagetk.components import SpatialImage
    from timagetk.algorithms import GeometricalFeatures
except ImportError as e:
    raise ImportError('Import Error: {}'.format(e))

__all__ = ['imgtosets']

np_unique, np_around = np.unique, np.around


def imgtosets(sp_img, label=[], background_id=None):
    """
    From labeled image to dict of sets (2D/3D)

    Parameters
    ----------
    :param *SpatialImage* sp_img: input ``SpatialImage`` (segmentation)

    :param list label: optional, labels of interest

    :param int background_id: optional, background_id. If specified,
                              removed from the computation

    Returns
    ----------
    :return: sets_dict (*dict*) -- dict. of sets

    Example
    -------
    >>> out_sets = image_to_sets(input_image, label=[2,3,4], background_id=1)
    """
    if isinstance(sp_img, SpatialImage):

        lab_max = np_unique(sp_img).tolist()
        if label is None or label==[]:
            if background_id is not None and background_id in lab_max:
                lab_max.remove(background_id)
            label = lab_max
        else:
            check_lab = [0 if lab in lab_max else 1 for ind, lab in enumerate(label)]
            if 1 not in check_lab:
                if background_id is not None and background_id in label:
                    label.remove(background_id)
                label=label
            else:
                return

        obj = GeometricalFeatures(sp_img, label=label)
        bounding_box_dict = obj.compute_bounding_box()
        sets_dict = {}
        from sets import Set
        for ind in bounding_box_dict:
            bbox, lab, glob_set = bounding_box_dict[ind]["Bounding box"], bounding_box_dict[ind]["Label"], []
            range_x, range_y = xrange(bbox[0], bbox[1]+1), xrange(bbox[2], bbox[3]+1)

            if sp_img.get_dim()==2:
                glob_set = [(np_around(j, decimals=2).tolist(),
                             np_around(k, decimals=2).tolist())
                             for j in range_x for k in range_y
                             if sp_img[j, k]==lab]

            elif sp_img.get_dim()==3:
                range_z = xrange(bbox[4], bbox[5]+1)
                glob_set = [(np_around(j, decimals=2).tolist(),
                             np_around(k, decimals=2).tolist(),
                             np_around(l, decimals=2).tolist())
                             for j in range_x for k in range_y for l in range_z
                             if sp_img[j, k, l]==lab]
            glob_set = Set(glob_set)
            sets_dict[lab] = glob_set
        return sets_dict
    else:
        print('sp_img must be a SpatialImage instance')
        return