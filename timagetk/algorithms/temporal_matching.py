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

from os import sep as separator
from itertools import chain
import numpy as np
try:
    from timagetk.components import SpatialImage
    from timagetk.algorithms import GeometricalFeatures
    from timagetk.algorithms.sets_metrics import sets_metrics
    from timagetk.algorithms.compute_res_sets import compute_res_sets
except ImportError as e:
    raise ImportError('Import Error: {}'.format(e))
__all__ = ['TemporalMatching']
av_number = 2

"""
This class allows the temporal matching of labeled objects.
"""


#--- from numpy
np_unique, np_ceil, np_around = np.unique, np.ceil, np.around
np_min, np_max, np_abs, np_sqrt = np.min, np.max, np.abs, np.sqrt
np_array = np.array


def around_val(input_val, DEC_VAL=3):
    return np_around(input_val, decimals=DEC_VAL).tolist()


class TemporalMatching(object):


    def __init__(self, segmentation_list, feature_space_list=None, background_id_list=None):
        """
        TemporalMatching constructor

        Parameters
        ----------
        :param list segmentation_list: list of input ``SpatialImage`` (segmentations)

        :param list feature_space_list: optional, list of input feature_space dictionaries

        :param list background_id_list: optional, list of input background identifiers
        """
        # - Check list_images type:
        try:
            assert isinstance(segmentation_list, list)
        except AssertionError:
            raise TypeError(
                "Input 'segmentation_list' must be a list, but is: {}".format(
                    type(segmentation_list)))
        # - Check SpatialImage sequence:
        conds_list_img = [isinstance(img, SpatialImage) for img in
                          segmentation_list]
        try:
            assert sum(conds_list_img) == len(conds_list_img)
        except AssertionError:
            raise TypeError(
                "Input 'segmentation_list' must be a list of SpatialImages!")
        # - Check SpatialImage sequence length, this function is useless if length < 2!
        try:
            assert len(segmentation_list) >= 2
        except AssertionError:
            raise ValueError(
                "Input 'segmentation_list' must have a minimal length of 2!")

        # - Create segmentation list and associated voxelsize list:
        # TODO: WHY? we loose important info (origin, metadata, ...), especially when recreating the SpatialImage with 'get_segmentation_list'
        self.number_of_sets = len(segmentation_list)
        seg_arr_list = [np_array(segmentation_list[ind], dtype=segmentation_list[ind].dtype)
                        for ind, val in enumerate(segmentation_list)] #--- numpy array instance
        self.segmentation_list = seg_arr_list
        self.voxelsize_list = [segmentation_list[ind].get_voxelsize() for ind, val in enumerate(segmentation_list)]

        #--- background id list
        if background_id_list:
            try:
                assert isinstance(background_id_list, list)
            except AssertionError:
                raise TypeError("Input 'background_id_list' must be a list of background label")
            try:
                assert len(background_id_list) == len(segmentation_list)
            except AssertionError:
                raise ValueError("Input 'background_id_list' must be of the same length than 'segmentation_list'")
            
            self.back_id_list = background_id_list
        else:
            mini_labs = [min(((np_unique(segmentation_list[ind])).tolist())) for
                         ind, val in enumerate(segmentation_list)]
            self.back_id_list = mini_labs

        #--- feature space list
        if feature_space_list:
            try:
                assert isinstance(feature_space_list, list)
            except AssertionError:
                raise TypeError("Input 'feature_space_list' must be a list")
            try:
                assert len(feature_space_list) == len(segmentation_list)
            except AssertionError:
                raise ValueError("Input 'feature_space_list' must be of the same length than 'segmentation_list'")

            conds_feat = [isinstance(feat, dict) for ind, feat in
                            enumerate(feature_space_list)]
            try:
                assert np.alltrue(conds_feat)
            except AssertionError:
                raise TypeError("Input 'feature_space_list' must be a list of dictionaries")
            
            for ind, feat_space_dict in enumerate(feature_space_list):
                for key in feat_space_dict:
                    if feat_space_dict[key]['Label'] == self.back_id_list[ind]:
                        # if present remove background features
                        del feat_space_dict[key]
                feature_space_list[ind] = feat_space_dict
            self.feature_space_list = feature_space_list

        else:
            feature_space_list = []
            for ind, segmentation in enumerate(segmentation_list):
                obj = GeometricalFeatures(segmentation)
                labels = (np_unique(segmentation)).tolist()
                labels.remove(self.back_id_list[ind])
                obj.set_labels(labels)
                feat_space_dict = obj.compute_feature_space(
                    background_id=self.back_id_list[ind])
                feature_space_list.append(feat_space_dict)
            self.feature_space_list = feature_space_list

        if self.feature_space_list:
            self.nodes_list = self.compute_nodes_list()

        # - Initialise object associated variables:
        self.admissible_matching_list = None
        self.single_cost_list, self.pairwise_cost_list, self.normalized_cost_list = None, None, None
        self.min_cost, self.max_cost = None, None
        self.source, self.sink, self.appear, self.disappear = None, None, None, None


    def get_part_nodes(self):
        """
        Get part. nodes i.e. source, sink, appear and disappear nodes

        Returns
        ----------
        :return: list -- list of nodes
        """
        val = [self.source, self.sink, self.appear, self.disappear]
        return val


    def get_segmentation_list(self):
        """
        Get segmentation list

        Returns
        ----------
        :return: list -- list of SpatialImage instances
        """
        sp_img_list = [SpatialImage(self.segmentation_list[ind], voxelsize=self.voxelsize_list[ind])
                        for ind, val in enumerate(self.segmentation_list)]
        return sp_img_list


    def set_segmentation_list(self, segmentation_list):
        """
        Set segmentation list

        Parameters
        ----------
        :param list segmentation_list: list of input ``SpatialImage`` (segmentations)
        """
        try:
            assert isinstance(segmentation_list, list)
        except AssertionError:
            raise TypeError(
                "Parameter 'segmentation_list' should be of type 'list', but is: {}".format(
                    type(segmentation_list)))
        # - Check SpatialImage sequence:
        conds_list_img = [isinstance(img, SpatialImage) for img in segmentation_list]
        try:
            assert sum(conds_list_img) == len(conds_list_img)
        except AssertionError:
            raise TypeError(
                "Parameter 'segmentation_list' should be a list of SpatialImages!")
        # - Check SpatialImage sequence length, this function is useless if length < 2!
        try:
            assert len(segmentation_list) >= 2
        except AssertionError:
            raise ValueError(
                "Parameter 'segmentation_list' should have a minimum length of 2!")

        self.__init__(segmentation_list)
        return "Updated self.segmentation list to:\n{}".format(self.segmentation_list)


    def get_feature_space_list(self):
        """
        Get feature space list

        Returns
        ----------
        :return: list -- list of dictionaries
        """
        return self.feature_space_list


    def set_feature_space_list(self, feature_space_list):
        """
        Set feature space list

        Parameters
        ----------
        :param list feature_space_list: list of dictionaries
        """
        conds_list = isinstance(feature_space_list, list) and len(feature_space_list)==len(self.segmentation_list)
        conds_feat = [0 if isinstance(feat_space_dict, dict) else 1
                      for ind, feat_space_dict in enumerate(feature_space_list)]
        if conds_list and 1 not in conds_feat:
            for ind, feat_space_dict in enumerate(feature_space_list):
                for key in feat_space_dict:
                    if feat_space_dict[key]['Label'] == self.back_id_list[ind]:
                        del feat_space_dict[key] # if present remove background features
                feature_space_list[ind] = feat_space_dict
            self.feature_space_list = feature_space_list


    def get_nodes_list(self):
        """
        Get nodes list as a contiguous dictionaries of nodes

        Returns
        ----------
        :return: nodes_list (*list*) -- list of contiguous vertices
        """
        return self.nodes_list


    def get_admissible_matching_list(self):
        """
        Get admissible matching list

        Returns
        ----------
        :return: list -- list of dictionaries
        """
        if self.admissible_matching_list is None:
            self.compute_admissible_matching()
        return self.admissible_matching_list


    def set_admissible_matching_list(self, adm_match_list):
        """
        Set admissible matching list

        Parameters
        ----------
        :param list adm_match_list: list of dictionaries
        """
        conds_list = isinstance(adm_match_list, list) and len(adm_match_list)==len(self.segmentation_list)-1
        if conds_list:
            self.admissible_matching_list = adm_match_list


    def get_single_cost_list(self, criterion=None):
        """
        Get single cost list

        Returns
        ----------
        :return: list -- list of dictionaries
        """
        if self.single_cost_list is None:
            adm_match_list = self.get_admissible_matching_list()
            self.compute_single_cost(adm_match_list, criterion=criterion)
        return self.single_cost_list


    def set_single_cost_list(self, sing_cost_list):
        """
        Set single cost list

        Parameters
        ----------
        :param list sing_cost_list: list of dictionaries
        """
        conds_list = isinstance(sing_cost_list, list) and len(sing_cost_list)==len(self.segmentation_list)-1
        if conds_list:
            self.single_cost_list = sing_cost_list


    def get_pairwise_cost_list(self, criterion=None):
        """
        Get pairwise cost list

        Returns
        ----------
        :return: list -- list of dictionaries
        """
        if self.pairwise_cost_list is None:
            adm_match_list = self.get_admissible_matching_list()
            self.compute_pairwise_cost(adm_match_list, criterion=criterion)
        return self.pairwise_cost_list


    def set_pairwise_cost_list(self, pairw_cost_list):
        """
        Set pairwise cost list

        Parameters
        ----------
        :param list pairw_cost_list: list of dictionaries
        """
        conds_list = isinstance(pairw_cost_list, list) and len(pairw_cost_list)==len(self.segmentation_list)-1
        if conds_list:
            self.pairwise_cost_list = pairw_cost_list


    def get_normalized_cost_list(self, criterion=None, min_cost=None, max_cost=None):
        """
        Get normalized cost list

        Returns
        ----------
        :return: list -- list of dictionaries
        """
        if self.normalized_cost_list is None:
            adm_match_list = self.get_admissible_matching_list()
            self.compute_normalized_cost(adm_match_list, criterion=criterion,
                                         min_cost=min_cost, max_cost=max_cost)
        return self.normalized_cost_list


    def set_normalized_cost_list(self, norm_cost_list):
        """
        Set normalized cost list

        Parameters
        ----------
        :param list norm_cost_list: list of dictionaries
        """
        conds_list = isinstance(norm_cost_list, list) and len(norm_cost_list)==len(self.segmentation_list)-1
        if conds_list:
            self.normalized_cost_list = norm_cost_list
            self.max_cost = norm_cost_list[0][1]['Max cost']
            self.min_cost = norm_cost_list[0][1]['Min cost']


#---
    def labels_to_nodes(self, feat_space_dict, min_node_val=1):
        """
        From dictionary of features to dictionary of contiguous nodes

        Parameters
        ----------
        :param dict feat_space_dict: dictionary of features

        :param int min_node_val: min node value

        Returns
        ----------
        :return: nodes_dict (*dict*) -- dictionary of contiguous vertices

        Example
        ----------
        >>> nodes_dict = self.labels_to_nodes(feature_dict, min_node_val)
        """
        nodes_dict = {}
        nodes_list = chain(xrange(min_node_val, min_node_val+len(feat_space_dict)))
        for ind_1, ind_2 in zip(nodes_list, feat_space_dict.keys()):
            nodes_dict[ind_1] = feat_space_dict[ind_2]
        return nodes_dict


    def compute_nodes_list(self):
        """
        Contiguous dictionaries of nodes

        Returns
        ----------
        :return: nodes_list (*list*) -- list of contiguous vertices

        Example
        ----------
        >>> nodes_list = self.compute_nodes_list()
        """
        feature_space_list = self.get_feature_space_list()
        nodes_list = []
        for ind, feat_space_dict in enumerate(feature_space_list):
            if ind==0:
                nodes_list.append(self.labels_to_nodes(feature_space_list[ind], 1))
            elif ind>0:
                nodes_list.append(self.labels_to_nodes(feature_space_list[ind], max(nodes_list[ind-1].keys())+1))
        return nodes_list


    def compute_admissible_matching(self, inc_max=None):
        """
        Computation of admissible matching based on spatio temporal information.
        Through spatial and temporal information, the set of solutions is reduced to a set of
        admissible solutions

        Parameters
        ----------
        :param int inc_max: maximal increment (neighborhood information), default None

        Returns
        ----------
        :return: adm_match_list (*list*) -- list of admissible matchings
                    Computation of : initial node, initial label, admissible nodes, admissible labels

        Example
        ----------
        >>> adm_match_list = self.compute_admissible_matching()
        """
        segmentation_list, feature_space_list = self.get_segmentation_list(), self.get_feature_space_list()
        nodes_list = self.get_nodes_list()
        img_dim = self.get_segmentation_list()[0].get_dim()
        adm_match_list = []
        for glob_index, feat_dict in enumerate(feature_space_list):
            if glob_index < len(feature_space_list)-1:

                # inc_max management
                if inc_max is None:
                    feat_0 = feature_space_list[glob_index]
                    bbox_size_list = [feat_0[key]['Bounding box size'] for key in feat_0]
                    x_list = [bbox_size_list[ind][0] for ind, val in enumerate(bbox_size_list)]
                    y_list = [bbox_size_list[ind][1] for ind, val in enumerate(bbox_size_list)]
                    if (img_dim == 3):
                        z_list = [bbox_size_list[ind][2] for ind, val in enumerate(bbox_size_list)]
                    if (img_dim == 2):
                        inc_max = max(max(x_list), max(y_list))
                    elif (img_dim == 3):
                        inc_max = max(max(x_list), max(y_list), max(z_list))
                else:
                    inc_max = np_abs(int(inc_max))

                adm_match_dict = {}

                nodes_0, nodes_1 = nodes_list[glob_index], nodes_list[glob_index+1]
                matrix_1, shape_1, back_id_1 = segmentation_list[glob_index+1], segmentation_list[glob_index+1].get_shape(), self.back_id_list[glob_index+1]

                for key in nodes_0:
                    adm_match_dict[key] = {}
                    adm_match_dict[key]['Initial node'], adm_match_dict[key]['Initial label']  = [key], [nodes_0[key]['Label']]
                    lab = []

                    index_x, index_y = nodes_0[key]['Index centroid'][0], nodes_0[key]['Index centroid'][1]
                    range_x, range_y = xrange(0, shape_1[0]), xrange(0, shape_1[1])
                    if (img_dim == 3):
                        index_z, range_z = nodes_0[key]['Index centroid'][2], xrange(0, shape_1[2])

                    if (img_dim == 2) and (index_x in range_x) and (index_y in range_y):
                        init_label = matrix_1[index_x, index_y]
                        if (init_label == back_id_1):
                            step_x = 0
                            if ((shape_1[0] -1 - index_x) >= index_x):
                                while (init_label == back_id_1):
                                    step_x+= 1
                                    if ((index_x + step_x < shape_1[0]) and (step_x < inc_max)):
                                        init_label = matrix_1[index_x + step_x, index_y]
                                        if ((init_label != back_id_1) and (len(lab)==0)):
                                            lab.append(init_label)
                                    else:
                                        break
                            elif ((shape_1[0] -1 - index_x) < index_x):
                                while (init_label == back_id_1):
                                    step_x-=1
                                    if (index_x + step_x >= 0) and (np_abs(step_x) < inc_max):
                                        init_label = matrix_1[index_x + step_x, index_y]
                                        if ((init_label != back_id_1) and (len(lab)==0)):
                                            lab.append(init_label)
                                    else:
                                        break
                        elif ((init_label != back_id_1) and (len(lab)==0)):
                            lab.append(init_label)

                    elif (img_dim==3) and (index_y in range_x) and (index_y in range_y) and (index_z in range_z):
                        init_label = matrix_1[index_x, index_y, index_z]
                        if (init_label == back_id_1):
                            step_x = 0
                            if ((shape_1[0] -1 - index_x) >= index_x):
                                while (init_label == back_id_1):
                                    step_x+= 1
                                    if ((index_x + step_x < shape_1[0]) and (step_x < inc_max)):
                                        init_label = matrix_1[index_x + step_x, index_y, index_z]
                                        if ((init_label != back_id_1) and (len(lab)==0)):
                                            lab.append(init_label)
                                    else:
                                        break
                            elif ((shape_1[0] -1 - index_x) < index_x):
                                while (init_label == back_id_1):
                                    step_x-=1
                                    if (index_x + step_x >= 0) and (np_abs(step_x) < inc_max):
                                        init_label = matrix_1[index_x + step_x, index_y, index_z]
                                        if ((init_label != back_id_1) and (len(lab)==0)):
                                            lab.append(init_label)
                                    else:
                                        break
                        elif ((init_label != back_id_1) and (len(lab)==0)):
                            lab.append(init_label)

                    if len(lab)==1:
                        neighbors = [nodes_1[nd]['Neighbors'] for nd in nodes_1
                                     if nodes_1[nd]['Label']==init_label]
                        neighbors = neighbors[0] #--- list type
                        for neigh in neighbors:
                            lab.append(neigh)

                        adm_match_dict[key]['Admissible labels'] = lab
                        adm_nodes = []
                        for lab in adm_match_dict[key]['Admissible labels']:
                            for nd in nodes_1:
                                if nodes_1[nd]['Label'] == lab:
                                    adm_nodes.append(nd)
                        adm_match_dict[key]['Admissible nodes'] = adm_nodes

                rem_keys = [key for key in adm_match_dict
                            if not adm_match_dict[key].has_key('Admissible nodes')]
                if rem_keys !=[]:
                    for val in rem_keys:
                        del adm_match_dict[val]
                adm_match_list.append(adm_match_dict)
        self.admissible_matching_list = adm_match_list
        return adm_match_list


    def compute_single_cost(self, adm_match_list, criterion=None):
        """
        Compute single cell - cell association cost

        Parameters
        ----------
        :param list adm_match_list: list of admissible matchings

        :param str criterion: optional, similarity metric.
            Default criterion is 'Euclidean distance'.
            criterion can be either:
                ['Euclidean distance', 'Jaccard coefficient', 'Mean overlap', 'Target overlap',
                 'Volume similarity', 'False negative error', 'False positive error',
                 'Sensitivity', 'Conformity']

        Returns
        ----------
        :return: single_cost_list (*list*) list of cell - cell association cost

        Example
        ----------
        >>> single_cost_list = self.compute_single_cost(adm_match_list)
        """
        possible_criteria = ['Euclidean distance', 'Jaccard coefficient',
                             'Mean overlap', 'Target overlap', 'Volume similarity',
                             'False negative error', 'False positive error',
                             'Sensitivity', 'Conformity']

        if ((criterion is None) or (criterion not in possible_criteria)):
            print('Possible criteria can be either:'), possible_criteria
            criterion = 'Euclidean distance'
            print('Setting criterion to:'), criterion
        else:
            criterion = str(criterion)

        segmentation_list, nodes_list = self.get_segmentation_list(), self.get_nodes_list()
        single_cost_list = []
        for glob_index, nodes_dict in enumerate(nodes_list):
            if glob_index < len(nodes_list)-1:
                segmentation_0, segmentation_1 = segmentation_list[glob_index], segmentation_list[glob_index+1]
                nodes_0, nodes_1 = nodes_list[glob_index], nodes_list[glob_index+1]
                single_cost_dict = {}
                for key in adm_match_list[glob_index]:
                    single_cost_dict[key] = {}
                    single_cost_dict[key]['Initial node'] = adm_match_list[glob_index][key]['Initial node']
                    single_cost_dict[key]['Criterion'] = criterion
                    single_cost_dict[key]['Admissible nodes'] = adm_match_list[glob_index][key]['Admissible nodes']
                    single_cost_dict[key]['Association cost'] = {}

                    if (criterion == 'Euclidean distance'):
                        for node in single_cost_dict[key]['Admissible nodes']:
                            if (segmentation_0.get_dim()==2):
                                eucl_dist = np_sqrt( pow((nodes_0[key]['Physical centroid'][0] - nodes_1[node]['Physical centroid'][0]),2) +
                                                     pow((nodes_0[key]['Physical centroid'][1] - nodes_1[node]['Physical centroid'][1]),2))
                            elif (segmentation_0.get_dim()==3):
                                eucl_dist = np_sqrt( pow((nodes_0[key]['Physical centroid'][0] - nodes_1[node]['Physical centroid'][0]),2) +
                                                     pow((nodes_0[key]['Physical centroid'][1] - nodes_1[node]['Physical centroid'][1]),2) +
                                                     pow((nodes_0[key]['Physical centroid'][2] - nodes_1[node]['Physical centroid'][2]),2) )
                            single_cost_dict[key]['Association cost'][node] = around_val(eucl_dist)

                    elif ( (criterion == 'Jaccard coefficient') or (criterion == 'Mean overlap')
                        or (criterion == 'Target overlap') or (criterion == 'Volume similarity')
                        or (criterion == 'False negative error') or (criterion == 'False positive error')
                        or (criterion == 'Sensitivity') or (criterion == 'Conformity')):
                        set_1 = compute_res_sets(segmentation_0, [key], nodes_0)
                        for node in single_cost_dict[key]['Admissible nodes']:
                            set_2 = compute_res_sets(segmentation_1, [node], nodes_1)
                            metric = sets_metrics(set_1, set_2, criterion)
                            single_cost_dict[key]['Association cost'][node] = around_val(metric)
                single_cost_list.append(single_cost_dict)
        self.single_cost_list = single_cost_list
        return single_cost_list


    def compute_pairwise_cost(self, adm_match_list, criterion=None):
        """
        Compute pairwise association cost

        Parameters
        ----------
        :param list adm_match_list: list of admissible matchings

        :param str criterion: optional, similarity metric.
            Default criterion is 'Euclidean distance'.
            criterion can be either:
                ['Euclidean distance', 'Jaccard coefficient', 'Mean overlap', 'Target overlap',
                 'Volume similarity', 'False negative error', 'False positive error',
                 'Sensitivity', 'Conformity']

        Returns
        ----------
        :return: pairwise_cost_list (*list*) -- list of pairwise association costs

        Example
        ----------
        >>> pairwise_cost_list = self.compute_pairwise_cost(adm_match_list, criterion)
        """
        possible_criteria = ['Euclidean distance', 'Jaccard coefficient',
                             'Mean overlap', 'Target overlap', 'Volume similarity',
                             'False negative error', 'False positive error',
                             'Sensitivity', 'Conformity']

        if ((criterion is None) or (criterion not in possible_criteria)):
            print('Possible criteria can be either:'), possible_criteria
            criterion = 'Euclidean distance'
            print('Setting criterion to:'), criterion
        else:
            criterion = str(criterion)

        segmentation_list, nodes_list = self.get_segmentation_list(), self.get_nodes_list()
        pairwise_cost_list = []
        for glob_index, node_dict in enumerate(nodes_list):
            if glob_index < len(nodes_list)-1:
                nodes_0, nodes_1 = nodes_list[glob_index], nodes_list[glob_index+1]
                segmentation_0, segmentation_1 = segmentation_list[glob_index], segmentation_list[glob_index+1]
                poss_pairwise_dict = {}
                for key in adm_match_list[glob_index]:
                    poss_pairwise_dict[key] = {}
                    for n1 in adm_match_list[glob_index][key]['Admissible nodes']:
                        for n2 in adm_match_list[glob_index][key]['Admissible nodes']:
                            if n1!=n2:
                                if segmentation_0.get_dim()==2:
                                    dist = np_sqrt(pow((nodes_1[n1]['Physical centroid'][0] - nodes_1[n2]['Physical centroid'][0]), 2)
                                                    + pow((nodes_1[n1]['Physical centroid'][1] - nodes_1[n2]['Physical centroid'][1]), 2))
                                elif segmentation_0.get_dim()==3:
                                    dist = np_sqrt(pow((nodes_1[n1]['Physical centroid'][0] - nodes_1[n2]['Physical centroid'][0]), 2)
                                                    + pow((nodes_1[n1]['Physical centroid'][1] - nodes_1[n2]['Physical centroid'][1]), 2)
                                                    + pow((nodes_1[n1]['Physical centroid'][2] - nodes_1[n2]['Physical centroid'][2]), 2))

                                lab = (n1, n2)
                                poss_pairwise_dict[key][lab] = {}
                                poss_pairwise_dict[key][lab]['Node 1'], poss_pairwise_dict[key][lab]['Node 2'] = n1, n2
                                poss_pairwise_dict[key][lab]['Distance'] = around_val(dist)

                pairwise_cost_dict = {}
                for key_1 in poss_pairwise_dict:
                    pairwise_cost_dict[key_1] = {}
                    min_dist = 100.0
                    for key_2 in poss_pairwise_dict[key_1]:
                        if poss_pairwise_dict[key_1][key_2]['Distance'] < min_dist:
                            min_dist = poss_pairwise_dict[key_1][key_2]['Distance']
                    for key_2 in poss_pairwise_dict[key_1]:
                        if poss_pairwise_dict[key_1][key_2]['Distance'] == min_dist:
                            pairwise_cost_dict[key_1] = {}
                            pairwise_cost_dict[key_1]['Node 1'], pairwise_cost_dict[key_1]['Node 2'] = poss_pairwise_dict[key_1][key_2]['Node 1'], poss_pairwise_dict[key_1][key_2]['Node 2']
                            pairwise_cost_dict[key_1]['Distance'] = min_dist
                            break

                if glob_index==0:
                    mini_node = max(nodes_list[-1].keys())+1
                else:
                    mini_node=0
                    for key in pairwise_cost_list[glob_index-1]:
                        add_node = pairwise_cost_list[glob_index-1][key]['Admissible nodes'][0]
                        if add_node > mini_node:
                            mini_node = add_node
                    mini_node = mini_node + 1

                for key in pairwise_cost_dict:
                    pairwise_cost_dict[key]['Initial node'] = [key]
                    pairwise_cost_dict[key]['Initial label'] = nodes_0[key]['Label']
                    pairwise_cost_dict[key]['Admissible nodes'] = [mini_node]
                    n1, n2 = pairwise_cost_dict[key]['Node 1'], pairwise_cost_dict[key]['Node 2']
                    pairwise_cost_dict[key]['Final label 1'], pairwise_cost_dict[key]['Final label 2'] = nodes_1[n1]['Label'], nodes_1[n2]['Label']
                    pairwise_cost_dict[key]['Criterion'] = criterion
                    pairwise_cost_dict[key]['Association cost'] = {}

                    if (criterion == 'Euclidean distance'):
                        if segmentation_0.get_dim()==2:
                            pairwise_centroid = [(nodes_1[n1]['Physical centroid'][0] + nodes_1[n2]['Physical centroid'][0])/2.0,
                                                 (nodes_1[n1]['Physical centroid'][1] + nodes_1[n2]['Physical centroid'][1])/2.0]
                            dist = np_sqrt(pow((nodes_0[key]['Physical centroid'][0] - pairwise_centroid[0]), 2)
                                            + pow((nodes_0[key]['Physical centroid'][1] - pairwise_centroid[1]), 2))

                        elif segmentation_0.get_dim()==3:
                            pairwise_centroid = [(nodes_1[n1]['Physical centroid'][0] + nodes_1[n2]['Physical centroid'][0])/2.0,
                                                 (nodes_1[n1]['Physical centroid'][1] + nodes_1[n2]['Physical centroid'][1])/2.0,
                                                 (nodes_1[n1]['Physical centroid'][2] + nodes_1[n2]['Physical centroid'][2])/2.0]
                            dist = np_sqrt(pow((nodes_0[key]['Physical centroid'][0] - pairwise_centroid[0]), 2)
                                            + pow((nodes_0[key]['Physical centroid'][1] - pairwise_centroid[1]), 2)
                                            + pow((nodes_0[key]['Physical centroid'][2] - pairwise_centroid[2]), 2))

                        pairwise_cost_dict[key]['Association cost'][mini_node] = around_val(dist)

                    elif ( (criterion == 'Jaccard coefficient') or (criterion == 'Mean overlap')
                            or (criterion == 'Target overlap') or (criterion == 'Volume similarity')
                            or (criterion == 'False negative error') or (criterion == 'False positive error')
                            or (criterion == 'Sensitivity') or (criterion == 'Conformity')):

                        set_1 = compute_res_sets(segmentation_0, [key], nodes_0)
                        set_2 = compute_res_sets(segmentation_1, [n1, n2], nodes_1)
                        metric = sets_metrics(set_1, set_2, criterion)
                        pairwise_cost_dict[key]['Association cost'][mini_node] = around_val(metric)

                    del pairwise_cost_dict[key]['Distance']
                    mini_node = mini_node + 1

                init_keys = [key for key in pairwise_cost_dict]
                add_keys = [pairwise_cost_dict[key]['Admissible nodes'][0] for key in init_keys]
                for ind_1, ind_2 in zip(init_keys, add_keys):
                    pairwise_cost_dict[ind_2] = pairwise_cost_dict[ind_1]
                for key in init_keys:
                    del pairwise_cost_dict[key]

                pairwise_cost_list.append(pairwise_cost_dict)
        self.pairwise_cost_list = pairwise_cost_list
        return pairwise_cost_list


    def compute_normalized_cost(self, adm_match_list, criterion=None, min_cost=None, max_cost=None):
        """
        Normalization of costs (simple and double) within the range
                [min_cost, max_cost].

        Association costs :
                - simple (one cell is associated to one cell)
                - double (one cell is associated to two cells)

        Parameters
        ----------
        :param list adm_match_list: list of admissible matchings

        :param str criterion: optional, similarity metric.
            Default criterion is 'Euclidean distance'.
            criterion can be either:
                ['Euclidean distance', 'Jaccard coefficient', 'Mean overlap', 'Target overlap',
                 'Volume similarity', 'False negative error', 'False positive error',
                 'Sensitivity', 'Conformity']

        :param int min_cost: optional minimum cost (default min_cost = 0)

        :param int max_cost: optional, maximum cost (default max_cost = 100)

        Returns
        ----------
        :return: normalized_cost_list (*list*) list of costs normalized within the range [min_cost, max_cost]

        Example
        ----------
        >>> normalized_cost_list = self.compute_normalized_association_cost(adm_match_list, criterion,
                                                                            min_cost, max_cost)
        """
        possible_criteria = ['Euclidean distance', 'Jaccard coefficient',
                             'Mean overlap', 'Target overlap', 'Volume similarity',
                             'False negative error', 'False positive error',
                             'Sensitivity', 'Conformity']

        if ((criterion is None) or (criterion not in possible_criteria)):
            print('Possible criteria can be either:'), possible_criteria
            criterion = 'Euclidean distance'
            print('Setting criterion to:'), criterion
        else:
            criterion = str(criterion)

        if (min_cost is None):
            min_cost = 0
        else:
            min_cost = np_abs(int(min_cost))

        if (max_cost is None):
            max_cost = 100
        else:
            max_cost = np_abs(int(max_cost))
        self.min_cost, self.max_cost = min_cost, max_cost

        global_cost_list, self.single_cost_list, self.pairwise_cost_list = [], [], []
        single_cost_list = self.compute_single_cost(adm_match_list, criterion)
        pairwise_cost_list = self.compute_pairwise_cost(adm_match_list, criterion)
        self.single_cost_list, self.pairwise_cost_list = single_cost_list, pairwise_cost_list

        for glob_index, single_cost_dict in enumerate(single_cost_list):
            global_cost_dict, pairwise_cost_dict = single_cost_dict.copy(), pairwise_cost_list[glob_index]

            init_add_list = [(pairwise_cost_dict[key]['Initial node'][0], key) for key in pairwise_cost_dict]
            for tmp in init_add_list:
                additional_node = tmp[1]
                additional_cost = pairwise_cost_dict[additional_node]['Association cost'][additional_node]
                global_cost_dict[tmp[0]]['Admissible nodes'].append(additional_node)
                global_cost_dict[tmp[0]]['Association cost'][additional_node] = additional_cost

            global_cost_list.append(global_cost_dict)

        normalized_cost_list = []
        for global_idx, global_cost_dict in enumerate(global_cost_list):
            criterion = global_cost_dict[global_cost_dict.keys()[0]]['Criterion']
            if ((criterion == 'Jaccard coefficient') or (criterion == 'Mean overlap')
                    or (criterion == 'Sensitivity') or (criterion == 'Target overlap')):
                # Warning --- sign inversion for some criteria !!
                # A low criterion value must give a low cost value
                # A high criterion value must give a high cost value
                for key_1 in global_cost_dict:
                    for key_2 in global_cost_dict[key_1]['Association cost'].keys():
                        val = 1.0 - global_cost_dict[key_1]['Association cost'][key_2]
                        global_cost_dict[key_1]['Association cost'][key_2] = val
            normalized_cost_dict = {}
            val_max, val_min = 0, 100000
            # Detect initial min value and initial max value (prior to rescaling)
            for key_1 in global_cost_dict:
                normalized_cost_dict[key_1] = {}
                normalized_cost_dict[key_1]['Initial node'] = global_cost_dict[key_1]['Initial node']
                normalized_cost_dict[key_1]['Admissible nodes'] = global_cost_dict[key_1]['Admissible nodes']
                for key_2 in global_cost_dict[key_1]['Association cost'].keys():
                    if global_cost_dict[key_1]['Association cost'][key_2] >= val_max:
                        val_max = global_cost_dict[key_1]['Association cost'][key_2]
                    if global_cost_dict[key_1]['Association cost'][key_2] < val_min:
                        val_min = global_cost_dict[key_1]['Association cost'][key_2]

            for key_1 in global_cost_dict:
                normalized_cost_dict[key_1]['Association cost'] = {}
                normalized_cost_dict[key_1]['Criterion'] = criterion
                normalized_cost_dict[key_1]['Min cost'], normalized_cost_dict[key_1]['Max cost'] = min_cost, max_cost
                for key_2 in global_cost_dict[key_1]['Association cost'].keys():
                    normalized_val = (max_cost - min_cost) * ((global_cost_dict[key_1]['Association cost'][key_2] - val_min)/(val_max - val_min)) + min_cost
                    normalized_cost_dict[key_1]['Association cost'][key_2] = int(normalized_val)
                    # dist_rescaled = (max_cost - min_cost) * ((dist - min_dist)/(max_dist - min_dist)) + min_cost
                    # TODO add others criterion and invert some of them !!
            normalized_cost_list.append(normalized_cost_dict)
        self.normalized_cost_list = normalized_cost_list
        return normalized_cost_list


    def build_init_graph(self, normalized_cost_list, alpha_cost=None):
        """
        Build an initial directed graph

        Parameters
        ----------
        :param list normalized_cost_list: list of normalized costs

        :param int alpha_cost: optional, initial cost (default alpha_cost = int((5/100.0)*max_cost))

        Returns
        ----------
        :return: initial_directed_graph (*nx.DiGraph*) initial directed graph

        Example
        ----------
        >>> initial_directed_graph = self.build_graph(normalized_cost_list,alpha_cost)
        """
        try:
            import networkx as nx
        except ImportError:
            raise ImportError('NetworkX import failed! Please install it.')
            import sys
            sys.exit(0)

        max_cost = self.max_cost
        if (alpha_cost is None):
            alpha_cost = int((5/100.0)*max_cost)
        else:
            if alpha_cost < max_cost:
                alpha_cost = int(alpha_cost)
            else:
                print('alpha_cost must be lower than max_cost')
                return

        nodes_list, pairwise_cost_list = self.get_nodes_list(), self.get_pairwise_cost_list()
        sets_single = [nodes_dict.keys() for glob_idx, nodes_dict in enumerate(nodes_list)]
        sets_pairwise = [pairwise_cost_dict.keys()
                         for glob_idx, pairwise_cost_dict in enumerate(pairwise_cost_list)]

        self.appear, self.disappear = np_max(sets_pairwise[-1])+1, np_max(sets_pairwise[-1])+2
        self.source, self.sink = np_min(sets_single[0])-1, np_max(sets_pairwise[-1])+3

        # Building a graph of admissible matchings
        G = nx.Graph()
        for tmp in sets_single:
            G.add_nodes_from(tmp)
        for tmp in sets_pairwise:
            G.add_nodes_from(tmp)
        spec_nodes = [self.source, self.appear, self.disappear, self.sink]
        for tmp in spec_nodes:
            G.add_node(tmp)

        # Add edges from source node to the first set of nodes
        edge_list = [(self.source, i, {'capacity': 1, 'weight': 0}) for i in sets_single[0]]
        G.add_edges_from(edge_list)

        # Add edges from the last set of nodes to the sink node
        edge_list = [(i, self.sink, {'capacity': 1, 'weight': 0}) for i in sets_single[-1]]
        G.add_edges_from(edge_list)

        # --- add edges from normalized_cost_list
        # --- (single cell - cell association cost)
        for ind, norm_cost_dict in enumerate(normalized_cost_list):
            edge_list = []
            for key_1 in norm_cost_dict:
                for key_2 in norm_cost_dict[key_1]['Association cost'].keys():
                    if key_1 in nodes_list[ind] and key_2 in nodes_list[ind+1]:
                        var = (key_1, key_2, {'capacity': 1, 'weight': norm_cost_dict[key_1]['Association cost'][key_2]})
                        edge_list.append(var)
            G.add_edges_from(edge_list)

        # --- enables cell division (splitting)
        cap_appear_pairw = 0
        for ind, pairwise_cost_dict in enumerate(pairwise_cost_list):
            edge_list = []
            for key in pairwise_cost_dict:
                init_node = pairwise_cost_dict[key]['Initial node'][0]
                node_1, node_2 = pairwise_cost_dict[key]['Node 1'], pairwise_cost_dict[key]['Node 2']
                alpha = int(np_ceil((normalized_cost_list[ind][init_node]['Association cost'][key])/2.0))
                if (init_node in nodes_list[ind]
                    and node_1 in nodes_list[ind+1]
                    and node_2 in nodes_list[ind+1]):
                    var_1 = (self.appear, key, {'capacity': 1, 'weight': alpha})
                    cap_appear_pairw = cap_appear_pairw + 1
                    var_2 = (init_node, key, {'capacity': 1, 'weight': alpha})
                    var_3 = (key, node_1, {'capacity': 1, 'weight': 0})
                    var_4 = (key, node_2, {'capacity': 1, 'weight': 0})
                    edge_list.append(var_1)
                    edge_list.append(var_2)
                    edge_list.append(var_3)
                    edge_list.append(var_4)
            G.add_edges_from(edge_list)

        cap_appear_sing, cap_disapp = 0, 0
        for ind, nodes_dict in enumerate(nodes_list):
            if ind>0:
                cap_appear_sing = cap_appear_sing + len(nodes_dict.keys())
                # Add edges from appear node to sets[1:end]
                edge_list = [(self.appear, i, {'capacity': 1, 'weight': alpha_cost}) for i in nodes_dict]
                G.add_edges_from(edge_list)

        for ind, nodes_dict in enumerate(nodes_list):
            if ind < len(nodes_list)-1:
                cap_disapp = cap_disapp + len(nodes_dict.keys())
                # Add edges from sets[0:end-1] to disappear node
                edge_list = [(i, self.disappear, {'capacity': 1, 'weight': alpha_cost}) for i in nodes_dict]
                G.add_edges_from(edge_list)

        cap_appear = cap_appear_sing + cap_appear_pairw
        G.add_edge(self.source, self.appear, capacity=cap_appear, weight=0)
        G.add_edge(self.appear, self.disappear, capacity=cap_appear,weight=alpha_cost)
        G.add_edge(self.disappear, self.sink, capacity=(cap_appear+cap_disapp), weight=0)
        initial_directed_graph = nx.DiGraph(G)
        return initial_directed_graph


    def update_graph(self, graph, matching_dict, norm_cost_list, alpha_cost):
        """
        Update the directed graph according to matching results

        Parameters
        ----------
        :param *nx.DiGraph* graph: directed graph

        :param dict matching_dict: dictionary of matchings

        :param dict normalized_association_cost_dict: dictionary of normalized costs

        :param dict pairwise_dict: dictionary of one cell - two cells events

        :param int alpha_cost: appear/disappear value

        Returns
        ----------
        :return: graph (*nx.DiGraph*) updated directed graph

        :return: normalized_association_cost_dict (*dict*), updated normalized costs

        :return: pairwise_dict (*dict*), dictionary of one cell - two cells events

        Example
        ----------
        >>> graph, normalized_association_cost_dict, dict_pairwise
                            = self.update_graph(graph, matching_dict, dict_pairwise,
                                                normalized_association_cost_dict, alpha_cost)
        """
        source, sink, appear, disappear = self.get_part_nodes()
        nodes_list, pairwise_cost_list = self.get_nodes_list(), self.get_pairwise_cost_list()
        nodes = [sorted(node_dict.keys()) for ind, node_dict in enumerate(nodes_list)]
        pairw_nodes = [sorted(pairwise_cost_dict.keys())
                        for ind, pairwise_cost_dict in enumerate(pairwise_cost_list)]
        pairw_set = []
        for ind, pairw in enumerate(pairw_nodes):
            for tmp, val in enumerate(pairw):
                pairw_set.append(val)

        for ind, node_list in enumerate(nodes):
            if ind < len(nodes)-1:
                for i in node_list:
                    if graph.has_edge(i, disappear):
                        graph.edge[i][disappear]['weight'] = alpha_cost

        for ind, node_list in enumerate(nodes):
            if ind>0:
                for i in node_list:
                    if graph.has_edge(appear, i):
                        graph.edge[appear][i]['weight'] = alpha_cost

        if graph.has_edge(appear, disappear):
            graph.edge[appear][disappear]['weight'] = alpha_cost

        for key in matching_dict:
            #--- list management
            for ind, val in enumerate(nodes):
                if key in nodes[ind]:
                    ind_time = ind

            if matching_dict[key]['Event'] == 'Stay':
                node_set_1, node_set_2 = matching_dict[key]['Node set 1'], matching_dict[key]['Node set 2']
                neighbors_set_1 = graph.neighbors(node_set_1)
                for neigh in neighbors_set_1:
                    if (neigh == node_set_2):
                        graph.edge[node_set_1][neigh]['weight'] = 1
                        norm_cost_list[ind_time][key]['Association cost'][neigh] = 1

            elif matching_dict[key]['Event'] == 'Split':
                node_set_1 = matching_dict[key]['Node set 1']
                neighbors_set_1 = graph.neighbors(node_set_1)
                for neigh in neighbors_set_1:
                    if (neigh in pairw_set):
                        norm_cost_list[ind_time][key]['Association cost'][neigh] = 1
                        if graph.has_edge(node_set_1, neigh):
                            graph.edge[node_set_1][neigh]['weight'] = 1
                        if graph.has_edge(appear, neigh):
                            graph.edge[appear][neigh]['weight'] = 1

            elif matching_dict[key]['Event'] == 'Disappear':
                node_set_1 = matching_dict[key]['Node set 1']
                if graph.has_edge(node_set_1, disappear):
                    graph.edge[node_set_1][disappear]['weight'] = alpha_cost

            elif matching_dict[key]['Event'] == 'Appear':
                node_set_2 = matching_dict[key]['Node set 2']
                if graph.has_edge(appear, node_set_2):
                    graph.edge[appear][node_set_2]['weight'] = alpha_cost

        return graph, norm_cost_list


    def matching(self, norm_cost_list, alpha_cost):
        """
        Max flow min cost matching

        Parameters
        ----------
        :param list norm_cost_list: dictionary of normalized costs

        :param int alpha_cost: appear/disappear value

        Returns
        ----------
        :return: out_matching_dict (*dict*) dictionary of matchings
        """
        try:
            import networkx as nx
        except ImportError:
            raise ImportError('NetworkX import failed! Please install it.')
            import sys
            sys.exit(0)

        graph = self.build_init_graph(norm_cost_list, alpha_cost=alpha_cost)
        source, sink, appear, disappear = self.get_part_nodes()
        print('Max flow...')
        max_flow_value, max_flow_dict = nx.maximum_flow(graph, source, sink, capacity='capacity')
        print('Max flow value'), max_flow_value
        H = nx.DiGraph(graph)
        H.add_node(source, demand= -max_flow_value)
        H.add_node(sink, demand= max_flow_value)
        print('Min cost...')
        min_flow_cost, min_flow_dict = nx.network_simplex(H, demand='demand', capacity='capacity', weight='weight')
        print('Min flow value'), min_flow_cost
        matching_dict, numbers = self.process_flow_dict(min_flow_dict)
        return matching_dict


    def process_flow_dict(self, min_flow_dict):
        """
        Process minimum flow dictionnary - Internal function

        Parameters
        ----------
        :param dict min_flow_dict: minimum flow dictionary

        Returns
        ----------
        :return: matching_dict (*dict*) dictionary of matchings

        :return: numbers (*int*) number of matchings

        Example
        ----------
        >>> matching_dict, numb = self.process_flow_dict(min_flow_dict)
        """
        source, sink, appear, disappear = self.get_part_nodes()
        nodes_list, pairwise_cost_list = self.get_nodes_list(), self.get_pairwise_cost_list()
        norm_cost_list = self.normalized_cost_list
        nodes = [sorted(node_dict.keys()) for ind, node_dict in enumerate(nodes_list)]
        pairw_nodes = [sorted(pairwise_cost_dict.keys()) for ind, pairwise_cost_dict in enumerate(pairwise_cost_list)]
        stay_numb, split_numb = 0,0
        disapp_numb, app_numb = 0,0

        out_dict = {}
        for glob_ind, nd_list in enumerate(nodes):
            if glob_ind < len(nodes)-1:
                for ind, nd in enumerate(nd_list):
                    out_dict[nd] = {}
        for key in out_dict:
            for loc_key in min_flow_dict[key]:
                if (loc_key != source and loc_key!= disappear and loc_key!=appear
                    and min_flow_dict[key][loc_key] != 0):

                    for ind, nd_list in enumerate(nodes):
                        if loc_key in nd_list:
                            out_dict[key]['Event'] = 'Stay'
                            out_dict[key]['Node set 1'] = key
                            out_dict[key]['Node set 2'] = loc_key
                            out_dict[key]['Label set 1'] = nodes_list[ind-1][key]['Label']
                            out_dict[key]['Initial label set 2'] = nodes_list[ind][loc_key]['Label']
                            out_dict[key]['Final label set 2'] = nodes_list[ind-1][key]['Label']
                            out_dict[key]['Association cost'] = norm_cost_list[ind-1][key]['Association cost'][loc_key]
                            stay_numb += 1

                    for ind, pr_list in enumerate(pairw_nodes):
                        if loc_key in pr_list:
                            out_dict[key]['Event'] = 'Split'
                            out_dict[key]['Node set 1'] = key
                            out_dict[key]['Label set 1'] = nodes_list[ind][key]['Label']
                            node_1 = pairwise_cost_list[ind][loc_key]['Node 1']
                            node_2 = pairwise_cost_list[ind][loc_key]['Node 2']
                            out_dict[key]['Node 1 set 2'] = node_1
                            out_dict[key]['Node 2 set 2'] = node_2
                            out_dict[key]['Initial label 1 set 2'] = nodes_list[ind+1][node_1]['Label']
                            out_dict[key]['Initial label 2 set 2'] = nodes_list[ind+1][node_2]['Label']
                            out_dict[key]['Association cost'] = norm_cost_list[ind][key]['Association cost'][loc_key]
                            split_numb +=1

                elif (loc_key==disappear and min_flow_dict[key][loc_key] != 0):
                    out_dict[key]['Event'] = 'Disappear'
                    out_dict[key]['Node set 1'] = key
                    for ind, nd_list in enumerate(nodes):
                        if key in nd_list:
                            out_dict[key]['Label set 1'] = nodes_list[ind][key]['Label']
                    disapp_numb += 1

        val = min_flow_dict[appear]
        for loc_key in val:
            if (loc_key != source and loc_key!=disappear and
                min_flow_dict[appear][loc_key] != 0):
                for ind, nd_list in enumerate(nodes):
                    if loc_key in nd_list:
                        if loc_key not in out_dict:
                            out_dict[loc_key] = {}
                        out_dict[loc_key]['Event'] = 'Appear'
                        out_dict[loc_key]['Node set 2'] = loc_key
                        out_dict[loc_key]['Label set 2'] = nodes_list[ind][loc_key]['Label']
                        app_numb += 1

        numbers = [stay_numb, split_numb, disapp_numb, app_numb]
        return out_dict, numbers


    def iterative_matching(self, norm_cost_list, graph, tolerance=None):
        """
        Iterative max flow min cost matching

        Parameters
        ----------
        :param list norm_cost_list: dictionary of normalized costs

        :param *nx.DiGraph* graph: directed and weighted graph

        :param float tolerance: optional, percentage of unmatched cells (default 10.0/100)

        Returns
        ----------
        :return: out_matching_dict (*dict*) dictionary of matchings

        Example
        ----------
        >>> out_matching_dict = self.iterative_matching(norm_cost_list, graph)
        """
        try:
            import networkx as nx
        except ImportError:
            raise ImportError('NetworkX import failed! Please install it.')
            import sys
            sys.exit(0)

        if tolerance is None:
            tolerance = 10.0/100

        source, sink, appear, disappear = self.get_part_nodes()
        nodes_list, pairwise_cost_list = self.get_nodes_list(), self.get_pairwise_cost_list()
        nodes = [sorted(node_dict.keys()) for ind, node_dict in enumerate(nodes_list)]
        number = [len(nodes[ind]) for ind, val in enumerate(nodes) if ind<len(nodes)-1]
        tol = sum(number)
        pairw_nodes = [sorted(pairwise_cost_dict.keys()) for ind, pairwise_cost_dict in enumerate(pairwise_cost_list)]

        max_cost = self.max_cost
        alpha_cost = graph[appear][disappear]['weight']
        print('Max flow...')
        max_flow_value, max_flow_dict = nx.maximum_flow(graph, source, sink, capacity='capacity')
        print('Max flow value'), max_flow_value
        H = nx.DiGraph(graph)
        tot_numb_list = [0]

        while ((tot_numb_list[-1] < (1.0-tolerance)*tol) or (alpha_cost < max_cost)):
            H.add_node(source, demand= -max_flow_value)
            H.add_node(sink, demand= max_flow_value)
            print('Min cost...')
            min_flow_cost, min_flow_dict = nx.network_simplex(H, demand='demand', capacity='capacity', weight='weight')
            print('Min flow value'), min_flow_cost
            matching_dict, numbers = self.process_flow_dict(min_flow_dict)
            stay_numb, split_numb = numbers[0], numbers[1]
            tot_numb_list.append(stay_numb + split_numb)
            print('tot numb list'), tot_numb_list
            alpha_cost = int(alpha_cost + (5.0/100)*max_cost)
            print('alpha'), alpha_cost
            H, norm_cost_list = self.update_graph(H, matching_dict, norm_cost_list, alpha_cost)
            if ((tot_numb_list[-1] > (1.0-tolerance)*tol) or (alpha_cost==max_cost)):
                out_matching_dict = matching_dict.copy()
                break
        return out_matching_dict


    def res_images(self, matching_dict):
        """
        Show matched cells

        Parameters
        ----------
        :param dict matching_dict: dictionary of matchings

        Returns
        ----------
        :return: img_list (list), list of *SpatialImage* (matchings events),
                    [time_i, time_j_match, disapp_time_i, app_time_j]

        Example
        ----------
        >>> img_list = self.res_images(matching_dict)
        """
        segmentation_list, nodes_list = self.get_segmentation_list(), self.get_nodes_list()
        nd_idx = []
        for ind, val in enumerate(nodes_list):
            nd_idx.append(val.keys())
        img_list = []
        for glob_ind, glob_val in enumerate(nd_idx): # glob_ind ---> time, # glob_val ---> set of nodes for this time
            if glob_ind < len(segmentation_list)-1:
                init_img = self.back_id_list[glob_ind]*np.ones_like(segmentation_list[glob_ind])
                end_img = self.back_id_list[glob_ind+1]*np.ones_like(segmentation_list[glob_ind+1])
                disapp_img = self.back_id_list[glob_ind]*np.ones_like(segmentation_list[glob_ind])
                app_img = self.back_id_list[glob_ind+1]*np.ones_like(segmentation_list[glob_ind+1])
                for ind, node in enumerate(glob_val): # ind ne sert a rien, nds on itere sur les nodes de ce temps la
                    if node in matching_dict:
                        if matching_dict[node]['Event'] in ['Stay','Split']:
                            lab_set_1 = matching_dict[node]['Label set 1']
                            if matching_dict[node]['Event']=='Stay':
                                lab_set_2 = matching_dict[node]['Initial label set 2']
                                init_img[segmentation_list[glob_ind]==lab_set_1]=lab_set_1
                                end_img[segmentation_list[glob_ind+1]==lab_set_2]=lab_set_1
                            elif matching_dict[node]['Event']=='Split':
                                lab_1_set_2 = matching_dict[node]['Initial label 1 set 2']
                                lab_2_set_2 = matching_dict[node]['Initial label 2 set 2']
                                init_img[segmentation_list[glob_ind]==lab_set_1]=lab_set_1
                                end_img[segmentation_list[glob_ind+1]==lab_1_set_2]=lab_set_1
                                end_img[segmentation_list[glob_ind+1]==lab_2_set_2]=lab_set_1
                                val = lab_1_set_2 in np.unique(segmentation_list[glob_ind+1]).tolist()

                        elif matching_dict[node]['Event']=='Disappear': #,'Appear']:
                            if matching_dict[node]['Event']=='Disappear':
                                lab_set_1 = matching_dict[node]['Label set 1']
                                disapp_img[segmentation_list[glob_ind]==lab_set_1]=lab_set_1

                    for tmp in matching_dict:
                        if matching_dict[tmp]['Event']=='Appear':
                            lab_set_2 = matching_dict[tmp]['Label set 2']
                            app_img[segmentation_list[glob_ind+1]==lab_set_2]=lab_set_2

                init_sp_img = SpatialImage(init_img, voxelsize=segmentation_list[glob_ind].get_voxelsize())
                end_sp_img = SpatialImage(end_img, voxelsize=segmentation_list[glob_ind+1].get_voxelsize())
                disapp_sp_img = SpatialImage(disapp_img, voxelsize=segmentation_list[glob_ind].get_voxelsize())
                app_sp_img = SpatialImage(app_img, voxelsize=segmentation_list[glob_ind+1].get_voxelsize())
                img_list.append(init_sp_img)
                img_list.append(end_sp_img)
                img_list.append(disapp_sp_img)
                img_list.append(app_sp_img)
        return img_list


    def write_lineage_file(self, out_path, matching_dict, times=None):
        """
        Write lineage file

        Parameters
        ----------
        :param str out_path: output path

        :param dict matching_dict: dictionary of matchings

        :param list times: optional, list of times

        Example
        ----------
        >>> self.write_matching_results(out_path, matching_dict, times)
        """
        if (times is None):
            times = ['unknown','unknown']

        if (out_path[-1] != separator):
            out_path = out_path + separator
        filename = out_path + 'a_lineage_time_' + str(times[0]) + '_to_' + str(times[1]) + '.txt'
        outfile = open(filename, 'a')
        ent = 'Initial time: ' + str(times[0]) + ' - Final time: ' + str(times[1]) + '\n'
        outfile.write(ent)
        outfile.write('STAY \n')
        for key in matching_dict:
            if (matching_dict[key]['Event'] == 'Stay'):
                label_set_1 = matching_dict[key]['Label set 1']
                label_set_2 = matching_dict[key]['Initial label set 2']
                association_cost = matching_dict[key]['Association cost']
                match_str = ''.join(['[', str(label_set_1), '] [', str(label_set_2), '] [',
                                       str(association_cost), ']\n'])
                outfile.write(match_str)
        outfile.write('SPLIT \n')
        for key in matching_dict:
            if (matching_dict[key]['Event'] == 'Split'):
                label_set_1 = matching_dict[key]['Label set 1']
                label_1_set_2 = matching_dict[key]['Initial label 1 set 2']
                label_2_set_2 = matching_dict[key]['Initial label 2 set 2']
                association_cost = matching_dict[key]['Association cost']
                match_str = ''.join(['[', str(label_set_1), '] [',
                                       str(label_1_set_2), ' ', str(label_2_set_2), '] [' ,
                                       str(association_cost), ']\n'])
                outfile.write(match_str)
        outfile.write('DISAPPEAR \n')
        for key in matching_dict:
            if (matching_dict[key]['Event'] == 'Disappear'):
                label_set_1 = matching_dict[key]['Label set 1']
                match_str = ''.join(['[', str(label_set_1), '] []\n'])
                outfile.write(match_str)
        outfile.write('APPEAR \n')
        for key in matching_dict:
            if (matching_dict[key]['Event'] == 'Appear'):
                label_set_2 = matching_dict[key]['Label set 2']
                match_str = ''.join(['[] [', str(label_set_2), ']\n'])
                outfile.write(match_str)
        outfile.close()
        return


def exch_lab(img_1, img_2):
    if isinstance(img_1, SpatialImage) and isinstance(img_2, SpatialImage):
        tmp_1, tmp_2 = img_1.get_array(), img_2.get_array()
        labs, labs_dict = np_unique(tmp_1).tolist(), {}
        for ind, lab in enumerate(labs):
            labs_dict[lab] = list(np_unique(tmp_2[tmp_1==lab]))
    return labs_dict


if __name__ == '__main__':
    print('')
