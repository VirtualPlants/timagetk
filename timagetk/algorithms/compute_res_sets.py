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

__all__ = ['compute_res_sets']

np_around = np.around


def compute_res_sets(sp_img, nodes_list, bounding_box_dict=None):
    """
    Compute resulting sets (2D or 3D).

    Parameters
    ----------
    :param *SpatialImage* sp_img: input ``SpatialImage`` (segmentation)

    :param list nodes_list: type list, list of vertices to be merged

    :param dict bounding_box_dict: dictionary of vertices

    Returns
    ----------
    :return: glob_set (*Set*) -- global set

    Example
    -------
    >>> glob_set = compute_res_sets(sp_img, nodes_list, bounding_box_dict)
    """
    if isinstance(sp_img, SpatialImage) and isinstance(nodes_list, list):
        if bounding_box_dict is None:
            obj = GeometricalFeatures(sp_img, label=nodes_list)
            bounding_box_dict = obj.compute_bounding_box()

        bbox_list = [bounding_box_dict[node]['Bounding box'] for node in nodes_list]
        labels_list = [bounding_box_dict[node]['Label'] for node in nodes_list]

        xmin_list = [bbox_list[ind][0] for ind, val in enumerate(bbox_list)]
        xmax_list = [bbox_list[ind][1] for ind, val in enumerate(bbox_list)]
        ymin_list = [bbox_list[ind][2] for ind, val in enumerate(bbox_list)]
        ymax_list = [bbox_list[ind][3] for ind, val in enumerate(bbox_list)]

        if (len(bbox_list[0])==6):
            zmin_list = [bbox_list[ind][4] for ind, val in enumerate(bbox_list)]
            zmax_list = [bbox_list[ind][5] for ind, val in enumerate(bbox_list)]

        glob_set = []
        range_x, range_y = xrange(min(xmin_list), max(xmax_list)+1), xrange(min(ymin_list),max(ymax_list)+1)

        if (len(bbox_list[0])==4):

            glob_set = [(np_around(j, decimals=2).tolist(),
                         np_around(k, decimals=2).tolist())
                         for j in range_x for k in range_y
                         if sp_img[j, k] in labels_list]

        elif (len(bbox_list[0])==6):

            range_z = xrange(min(zmin_list), max(zmax_list)+1)
            glob_set = [(np_around(j, decimals=2).tolist(),
                         np_around(k, decimals=2).tolist(),
                         np_around(l, decimals=2).tolist())
                         for j in range_x for k in range_y for l in range_z
                         if sp_img[j, k, l] in labels_list]

        from sets import Set
        glob_set = Set(glob_set)
        return glob_set
    else:
        print('sp_img must be a SpatialImage instance and nodes_list must be a list of nodes')
        return