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

"""
This class enables a computation of several geometric features of
labeled objects. Methods are 2D and 3D.
"""
# see for example: J.Zunic, "Shape descriptors for image analysis."

import numpy as np
import time
try:
    from timagetk.components import SpatialImage
except ImportError:
    raise ImportError('Import Error')
__all__ = ['GeometricalFeatures']

#--- from numpy
np_around = np.around
np_array, np_zeros, np_float = np.array, np.zeros, np.float
np_where, np_in1d, np_reshape, np_unique = np.where, np.in1d, np.reshape, np.unique
np_min, np_max, np_mean, np_sqrt, np_abs = np.min, np.max, np.mean, np.sqrt, np.abs
np_prod, np_sum = np.prod, np.sum
np_eig, np_argsort, np_trace = np.linalg.eig, np.argsort, np.trace


def around_val(input_val, DEC_VAL=3):
    val = np_around(input_val, decimals=DEC_VAL).tolist()
    return val

#--------------------------------------------
# code profiling -
#def do_profile(follow=[]):
#    from line_profiler import LineProfiler
#    def inner(func):
#        def profiled_func(*args, **kwargs):
#            try:
#                profiler = LineProfiler()
#                profiler.add_function(func)
#                for f in follow:
#                    profiler.add_function(f)
#                profiler.enable_by_count()
#                return func(*args, **kwargs)
#            finally:
#                profiler.print_stats()
#        return profiled_func
#    return inner
#--------------------------------------------


class GeometricalFeatures(object):


    def __init__(self, sp_img, label=[]):
        """
        ``GeometricalFeatures`` constructor (2D and 3D images)

        Parameters
        ----------
        :param *SpatialImage* sp_img: input ``SpatialImage``

        :param list label: labels of interest
        """
        if isinstance(sp_img, SpatialImage):
                self._segmentation  = np.array(sp_img, dtype=sp_img.dtype) #--- numpy array instance
                self._voxelsize, self._shape, self._dim = sp_img.get_voxelsize(), sp_img.get_shape(), sp_img.get_dim()
                self._label_range_max = np_unique(self._segmentation.ravel()).tolist()
                if len(label) is 0:
                    self.int_labels = None
                elif isinstance(label, list) and len(label)>0:
                    self.set_labels(label)
                self.int_indexes, self.int_iterators = None, None


    def set_labels(self, label):
        """
        Set labels of interest

        Parameters
        ----------
        :param list label: labels of interest
        """
        label_range_max = self._label_range_max
        val = 0
        for i in label:
            if i not in label_range_max:
                val += 1
        if val is 0:
            self.int_labels = label
        else:
            self.int_labels = None
        self.int_indexes, self.int_iterators = None, None
        return


    def get_labels(self):
        """
        Get labels of interest

        Parameters
        ----------
        :param list label: labels of interest

        Returns
        ----------
        :return: int_labels -- labels of interest
        """
        return self.int_labels


    def lab_idx(self):
        """
        Region/Volume (2D/3D) shape iterators (approximation by a rectangle/cuboid)
        """
        if self.int_labels:
            start_time = time.time()
            label = self.get_labels()
            dim, shape, segmentation = self._dim, self._shape, self._segmentation
            idx = [np_where(np_reshape(np_in1d(segmentation.ravel(),lab), shape)) for lab in label]
            index_list = []
            index_list_append = index_list.append
            if dim==2:
                for ind, val in enumerate(idx):
                    xmin, xmax, ymin, ymax = np_min(idx[ind][0]), np_max(idx[ind][0]), np_min(idx[ind][1]), np_max(idx[ind][1])
                    index_list_append((xmin, xmax, ymin, ymax))
            elif dim==3:
                for ind, val in enumerate(idx):
                    xmin, xmax, ymin, ymax = np_min(idx[ind][0]), np_max(idx[ind][0]), np_min(idx[ind][1]), np_max(idx[ind][1])
                    zmin, zmax = np_min(idx[ind][2]), np_max(idx[ind][2])
                    index_list_append((xmin, xmax, ymin, ymax, zmin, zmax))
            self.int_indexes = index_list
            end_time = time.time()
            print('Number of labels : '), len(label)
            print('Iterators, elapsed time : '), end_time - start_time
            return
        else:
            print('Error : invalid index')
            print('Index list :'), self._label_range_max
            return


    def lab_itrs(self):
        """
        Region/Volume (2D/3D) point iterators
        """
        if self.int_labels:
            start_time = time.time()
            label = self.get_labels()
            if not self.int_indexes:
                self.lab_idx()
            index_list = self.int_indexes
            dim, segmentation = self._dim, self._segmentation
            glob_list = []
            glob_list_append = glob_list.append
            if dim==2:
                for ind, val in enumerate(label):
                    xmin, xmax, ymin, ymax = index_list[ind]
                    range_x, range_y = xrange(xmin, xmax+1), xrange(ymin, ymax+1)
                    iter_list = [ [j, k] for j in range_x for k in range_y if segmentation[j, k]==val]
                    glob_list_append(iter_list)
            elif dim==3:
                for ind, val in enumerate(label):
                    xmin, xmax, ymin, ymax, zmin, zmax = index_list[ind]
                    range_x, range_y, range_z = xrange(xmin, xmax+1), xrange(ymin, ymax+1), xrange(zmin, zmax+1)
                    iter_list = [ [j, k, l] for j in range_x for k in range_y for l in range_z
                                        if segmentation[j, k, l]==val]
                    glob_list_append(iter_list)
            self.int_iterators = glob_list
            end_time = time.time()
            print('Number of labels : '), len(label)
            print('Iterators, elapsed time : '), end_time - start_time
            return
        else:
            print('Error : invalid index')
            print('Index list :'), self._label_range_max
            return


    def moment(self, iterators, order):
        """
        Moment value

        Parameters
        ----------
        :param list iterators: iterators

        :param list order: order

        Returns
        ----------
        :return: moment_value (*double*)
        """
        moment_value = 0
        pow_arr = np.power(iterators, order)
        for tmp in pow_arr:
            moment_value = moment_value + np_prod(tmp)
        return moment_value


    #@do_profile(follow=[lab_idx])
    def compute_bounding_box(self):
        """
        The bounding box is calculted as the minimum and maximum indices
        in each dimension of the object
        It is represented as a set of min/max pairs for each dimension
        In 2D, it is [min(X), max(X), min(Y), max(Y)]
        In 3D, it is [min(X), max(X), min(Y), max(Y), min(Z), max(Z)]

        Returns
        ----------
        :return: bounding_box_dict (*dict*) -- bounding box dictionary

        Example
        -------
        >>> bounding_box_dict = self.compute_bounding_box()
        """
        if self.int_labels:
            dim, label = self._dim, self.int_labels
            if self.int_indexes is None or len(self.int_indexes)!=len(label):
                self.lab_idx()
            start_time = time.time()
            index_list = self.int_indexes
            bounding_box_dict = {}
            if (dim==2 or dim==3):
                if dim==2:
                    for ind, val in enumerate(label):
                        xmin, xmax, ymin, ymax = index_list[ind]
                        bbox = (xmin, xmax, ymin, ymax)
                        bbox_size = (xmax-xmin+1, ymax-ymin+1)
                        bbox_area = (xmax-xmin+1)*(ymax-ymin+1)
                        bounding_box_dict[val] = {"Label":val, "Bounding box":bbox, "Bounding box size":bbox_size, "Bounding box area":bbox_area}
                elif dim==3:
                    for ind, val in enumerate(label):
                        xmin, xmax, ymin, ymax, zmin, zmax = index_list[ind]
                        bbox = (xmin, xmax, ymin, ymax, zmin, zmax)
                        bbox_size = (xmax-xmin+1, ymax-ymin+1, zmax-zmin+1)
                        bbox_volume = (xmax-xmin+1)*(ymax-ymin+1)*(zmax-zmin+1)
                        bounding_box_dict[val] = {"Label":val, "Bounding box":bbox, "Bounding box size":bbox_size, "Bounding box volume":bbox_volume}
            end_time = time.time()
            print('Number of labels : '), len(label)
            print('Bonding box, elapsed time : '), end_time - start_time
            return bounding_box_dict
        else:
            print('Index list :'), self._label_range_max
            return

    #@do_profile(follow=[lab_itrs, moment])
    def compute_volume(self):
        """
        Volume calculated from the image moments

        Returns
        ----------
        :return: volume_dict (*dict*) -- dictionary of area (2D) or volumes (3D)

        Example
        -------
        >>> volume_dict = self.compute_volume()
        """
        if self.int_labels:
            dim, voxelsize, label = self._dim, self._voxelsize, self.int_labels
            if self.int_iterators is None or len(self.int_iterators)!=len(label):
                self.lab_itrs()
            start_time = time.time()
            iterators = self.int_iterators
            if (dim==2 or dim==3):
                volume_dict={}
                mom_val = self.moment
                if dim==2:
                    order = [0, 0]
                    vox_prod = voxelsize[0]*voxelsize[1]
                    for ind, val in enumerate(label):
                        m_00 = mom_val(iterators[ind], order)
                        vol = around_val(m_00*vox_prod)
                        volume_dict[val] = {"Label":val, "Physical volume":vol}
                elif dim==3:
                    order = [0, 0, 0]
                    vox_prod = voxelsize[0]*voxelsize[1]*voxelsize[2]
                    for ind, val in enumerate(label):
                        m_000 = mom_val(iterators[ind], order)
                        vol = around_val(m_000*vox_prod)
                        volume_dict[val] = {"Label":val, "Physical volume":vol}
            end_time = time.time()
            print('Number of labels : '), len(label)
            print('Volume, elapsed time : '), end_time - start_time
            return volume_dict
        else:
            print('Index list :'), self._label_range_max
            return

    #@do_profile(follow=[lab_itrs, moment, around_val])
    def compute_centroid(self):
        """
        Unweighted centroid calculated from the image moments.

        Returns
        ----------
        :return: centroid_dict (*dict*) -- dictionary of centroids.

        Example
        -------
        >>> centroid_dict = self.compute_centroid()
        """
        if self.int_labels:
            dim, voxelsize, label = self._dim, self._voxelsize, self.int_labels
            if self.int_iterators is None or len(self.int_iterators)!=len(label):
                self.lab_itrs()
            start_time = time.time()
            iterators = self.int_iterators
            if (dim==2 or dim==3):
                centroid_dict={}
                mom_val = self.moment
                if dim==2:
                    order_00, order_01, order_10 = [0,0], [0,1], [1,0]
                    for ind, val in enumerate(label):
                        it = iterators[ind]
                        m_00, m_01, m_10 = mom_val(it, order_00), mom_val(it, order_01), mom_val(it, order_10)
                        centroid = (around_val(m_10/m_00), around_val(m_01/m_00))
                        centroid_phys = (around_val(voxelsize[0]*centroid[0]),
                                         around_val(voxelsize[1]*centroid[1]))
                        centroid_dict[val] = {"Label":val, "Physical centroid":centroid_phys, "Index centroid":centroid}
                elif dim==3:
                    order_000, order_100, order_010, order_001 = [0,0,0], [1,0,0], [0,1,0], [0,0,1]
                    for ind, val in enumerate(label):
                        it = iterators[ind]
                        m_000 = mom_val(it, order_000)
                        m_100, m_010, m_001 = mom_val(it, order_100), mom_val(it, order_010), mom_val(it, order_001)
                        centroid = (around_val(m_100/m_000), around_val(m_010/m_000), around_val(m_001/m_000))
                        centroid_phys = (around_val(voxelsize[0]*centroid[0]),
                                         around_val(voxelsize[1]*centroid[1]),
                                         around_val(voxelsize[2]*centroid[2]))
                        centroid_dict[val] = {"Label":val, "Physical centroid":centroid_phys, "Index centroid":centroid}
            end_time = time.time()
            print('Number of labels : '), len(label)
            print('Centroid, elapsed time : '), end_time - start_time
            return centroid_dict
        else:
            print('Index list :'), self._label_range_max
            return


    def compute_moment(self, order=[], option='raw'):
        """
        Image moments are useful to describe objects and form the
        building blocks of many useful features of the objects

        Parameters
        ----------
        :param list order: moment orders, default: [0,0] (2D), [0,0,0] (3D)

        :param str option: moment option, default:'raw'.
                           option can be either ['raw', 'centered', 'normalised']
                           - 'raw' moments are not translation invariant,
                           - 'central' moments are translation invariant,
                              and not scaling invariant (achieved by substracting the centroid),
                           - 'normalised' moments are translation invariant and scaling invariant

        Returns
        ----------
        :return: moment_dict (*dict*) -- dictionary of moments.

        Example
        -------
        >>> moment_dict = self.compute_moment(order, option)
        """
        if self.int_labels:
            dim, label = self._dim, self.int_labels
            if self.int_iterators is None or len(self.int_iterators)!=len(label):
                self.lab_itrs()
            start_time = time.time()
            iterators = self.int_iterators

            if order == []:
                if dim==2:
                    order = [0, 0]
                elif dim==3:
                    order = [0, 0, 0]
            else:
                if len(order) != dim:
                    print('Warning : incorrect specification')
                    if dim==2:
                        order = [0, 0]
                    elif dim==3:
                        order = [0, 0, 0]
                else:
                    order = order

            opt_list = ['raw', 'centered', 'normalised']
            if option not in opt_list:
                print('Option can be:'), opt_list
                print('Setting option to raw...')
                option = 'raw'

            if (dim==2 or dim==3):
                order_00, order_01, order_10 = [0,0], [0,1], [1,0]
                order_000, order_100, order_010, order_001 = [0,0,0], [1,0,0], [0,1,0], [0,0,1]
                mom_val = self.moment
                moment_dict = {}
                for ind, val in enumerate(label):
                    it = iterators[ind]
                    moment_val = mom_val(it, order)
                    if (option == 'raw'):
                        moment_dict[val] = {"Label":val, "Moment":moment_val}
                    elif ((option == 'centered') or (option == 'normalised')):
                        if dim==2:
                            m_00, m_01, m_10 = mom_val(it, order_00), mom_val(it, order_01), mom_val(it, order_10)
                            centroid = [around_val(-1.0*m_10/m_00), around_val(-1.0*m_01/m_00)]
                        elif dim==3:
                            m_000 = mom_val(it, order_000)
                            m_100, m_010, m_001 = mom_val(it, order_100), mom_val(it, order_010), mom_val(it, order_001)
                            centroid = [around_val(-1.0*m_100/m_000),
                                        around_val(-1.0*m_010/m_000),
                                        around_val(-1.0*m_001/m_000)]
                        iter_cent=[]
                        iter_cent_append = iter_cent.append
                        for tmp_it in it:
                            iter_cent_append(np_sum([tmp_it, centroid], axis=0).tolist())
                        moment_cent = mom_val(iter_cent, order)
                        if (option == 'centered'):
                            # translation invariant
                            moment_dict[val] = {"Label":val, "Moment":moment_cent}
                        elif (option == 'normalised'):
                            # scaling invariant
                            factor = 1.0 + np_mean(order)
                            if dim==2:
                                moment_norm = (1.0*moment_cent) / pow(m_00, factor)
                            elif dim==3:
                                moment_norm = (1.0*moment_cent) / pow(m_000, factor)
                            moment_norm = around_val(moment_norm)
                            moment_dict[val] = {"Label":val, "Moment":moment_norm}
            end_time = time.time()
            print('Number of labels : '), len(label)
            print('Elapsed time : '), end_time - start_time
            return moment_dict
        else:
            print('Index list :'), self._label_range_max
            return


    def compute_covariance_matrix(self):
        """
        Compute covariance matrix. Several features such as hyper-ellipsoid
        fitting are calculated using the eigenvalues/eigenvectors of
        the covariance matrix.

        Returns
        ----------
        :return: cov_dict (*dict*) -- dictionary of cov_mat, eigenvalues and
                                        eigenvectors

        Example
        -------
        >>> cov_dict = self.compute_covariance_matrix()
        """
        if self.int_labels:
            dim, label, voxelsize = self._dim, self.int_labels, self._voxelsize
            if self.int_iterators is None or len(self.int_iterators)!=len(label):
                self.lab_itrs()
            start_time = time.time()
            iterators = self.int_iterators

            if (dim==2 or dim==3):

                cov_dict = {}
                for ind, val in enumerate(label):

                    centroid_dict = self.compute_centroid()

                    it = iterators[ind]
                    samples = np_zeros((len(it),dim),dtype=np_float)
                    mean_vect = np_zeros((1,dim),dtype=np_float)
                    cov_mat = np_zeros((dim,dim),dtype=np_float)

                    if dim==2:
                        for index_it, tmp_it in enumerate(it):
                            samples[index_it,0:dim] = [tmp_it[0]*voxelsize[0],
                                                       tmp_it[1]*voxelsize[1]]

                        mean_vect[0,0] = np_mean(samples[:,0],dtype=np_float)
                        mean_vect[0,1] = np_mean(samples[:,1],dtype=np_float)

                        data = samples - mean_vect
                        cov_mat[1,0] = (1.0/len(it))*np_sum((data[:,1]*data[:,0]))
                        cov_mat[0,1] = cov_mat[1,0]

                    elif dim==3:
                        for index_it, tmp_it in enumerate(it):
                            samples[index_it,0:dim] = [tmp_it[0]*voxelsize[0],
                                                       tmp_it[1]*voxelsize[1],
                                                       tmp_it[2]*voxelsize[2]]

                        mean_vect[0,0] = np_mean(samples[:,0],dtype=np_float)
                        mean_vect[0,1] = np_mean(samples[:,1],dtype=np_float)
                        mean_vect[0,2] = np_mean(samples[:,2],dtype=np_float)

                        data = samples - mean_vect
                        cov_mat[1,0] = (1.0/len(it))*np_sum((data[:,1]*data[:,0]))
                        cov_mat[0,1] = cov_mat[1,0]
                        cov_mat[2,0] = (1.0/len(it))*np_sum((data[:,2]*data[:,0]))
                        cov_mat[0,2] = cov_mat[2,0]
                        cov_mat[2,1] = (1.0/len(it))*np_sum((data[:,2]*data[:,1]))
                        cov_mat[1,2] = cov_mat[2,1]

                    for j in range(dim):
                        cov_mat[j,j] = (1.0/len(it))*np_sum((data[:,j]**2))

                    tot_inertia = np_trace(cov_mat)
                    eigenval, eigenvect = np_eig(cov_mat)
                    idx = list(np_argsort(eigenval))
                    # eigenval in increasing order !
                    if idx!=[0,1,2] or id!=[0,1]:
                        eigenval_st = np_array(eigenval)[idx]
                        eigenvect_st = eigenvect[:,idx]
                    else:
                        eigenval_st = eigenval
                        eigenvect_st = eigenvect

                    #--- Check -- OK
                    # D, P = np.diag(eigenval_st), eigenvect_st
                    # out = cov_mat - np.dot(np.dot(P,D),np.linalg.inv(P))
                    # print('check :'), out

                    axes_lengths = 4*np_sqrt(eigenval_st)
                    radius = [axes_lengths[index]/2.0 for index, tmp in enumerate(axes_lengths)]

            cov_dict[val] = {"Label":val, "Cov matrix":cov_mat,
                             "Eigenvalues":eigenval_st,
                             "Eigenvectors":eigenvect_st,
                             "Physical center":centroid_dict[val]['Physical centroid'],
                             "Index Centroid":centroid_dict[val]['Index centroid'],
                             "Radius":radius,"Axes lengths":axes_lengths,
                             "Total inertia":tot_inertia}

            end_time = time.time()
            print('Number of labels : '), len(label)
            print('Elapsed time : '), end_time - start_time
            return cov_dict
        else:
            print('Index list :'), self._label_range_max
            return


    # @do_profile(follow=[])
    def compute_local_rag_approx(self, centroid_dict={}, background_id=1, radius=0):
        """
        Compute 2D/3D local region adjacency approximation
        Considered approximation: circular (2D), spherical (3D)

        Parameters
        ----------
        :param dict centroid_dict: dictionary of centroids

        :param int background_id: background label, default: background_id==1

        :param int radius: circle/sphere radius (pixel/voxel), if 0 an automatic radius is computed

        Returns
        ----------
        :return: neigh_dict (*dict*) -- dictionary of local adjacencies

        Example
        -------
        >>> neigh_dict = self.compute_local_rag_approx()
        """
        if self.int_labels:
            start_time = time.time()
            matrix, dim, shape, label = self._segmentation, self._dim, self._shape, self.int_labels

            if background_id in label:
                label.remove(background_id)

            if centroid_dict=={}:
                centroid_dict = self.compute_centroid()
            else:
                for key in centroid_dict:
                    if (centroid_dict[key]['Label']==background_id):
                        del centroid_dict[key]

            if radius==0:

                bounding_box_dict = self.compute_bounding_box()
                bbox_size_list, x_list, y_list, z_list = [], [], [], []
                bbox_size_list_append = bbox_size_list.append
                x_list_append, y_list_append, z_list_append = x_list.append, y_list.append, z_list.append
                for key in bounding_box_dict:
                    bbox_size_list_append(bounding_box_dict[key]['Bounding box size'])
                for ind, val in enumerate(bbox_size_list):
                    x_list_append(bbox_size_list[ind][0])
                    y_list_append(bbox_size_list[ind][1])
                    if dim==3:
                        z_list_append(bbox_size_list[ind][2])

                if dim==2:
                    inc = max(max(x_list), max(y_list))
                elif dim==3:
                    inc = max(max(x_list), max(y_list), max(z_list))
                radius = int(inc/2.0)

            else:
                radius = int(abs(radius))

            neigh_dict = {}
            for key in centroid_dict:
                neigh_dict[key] = {}
                neigh_dict[key]['Label'], neigh_dict[key]['Index centroid'], neigh_dict[key]['Physical centroid'] = centroid_dict[key]['Label'], centroid_dict[key]['Index centroid'], centroid_dict[key]['Physical centroid']
                lab = []
                lab_append, lab_remove, lab_sort = lab.append, lab.remove, lab.sort

                if dim==2:
                    index_x, index_y = neigh_dict[key]['Index centroid'][0], neigh_dict[key]['Index centroid'][1]
                    x_min, y_min = max(0, index_x-radius), max(0, index_y-radius)
                    x_max, y_max = min(shape[0]-1, index_x+radius), min(shape[1]-1, index_y+radius)
                    sub_range_x, sub_range_y = xrange(x_min, x_max+1), xrange(y_min, y_max+1)
                    conds_loc = [1 if (pow((j-index_x), 2) + pow((k-index_y), 2) <= pow(radius,2))
                                 else 0 for j in sub_range_x for k in sub_range_y]
                    conds_val = [matrix[j, k] if (matrix[j, k]!= background_id) else 0
                                 for j in sub_range_x for k in sub_range_y]
                    for ind_loc, val_loc in enumerate(conds_loc):
                        if conds_loc[ind_loc]==1 and conds_val[ind_loc]!=0:
                            if conds_val[ind_loc] not in lab and conds_val[ind_loc]!=neigh_dict[key]['Label']:
                                lab_append(conds_val[ind_loc])

                elif dim==3:
                    index_x, index_y, index_z = neigh_dict[key]['Index centroid'][0], neigh_dict[key]['Index centroid'][1], neigh_dict[key]['Index centroid'][2]
                    x_min, y_min, z_min = max(0, index_x-radius), max(0, index_y-radius), max(0, index_z-radius)
                    x_max, y_max, z_max = min(shape[0]-1, index_x+radius), min(shape[1]-1, index_y+radius), min(shape[2]-1, index_z+radius)
                    sub_range_x, sub_range_y, sub_range_z = xrange(x_min, x_max+1), xrange(y_min, y_max+1), xrange(z_min, z_max+1)
                    conds_loc = [1 if (pow((j-index_x), 2) + pow((k-index_y), 2) + pow((l-index_z), 2) <= pow(radius,2))
                                 else 0 for j in sub_range_x for k in sub_range_y for l in sub_range_z]
                    conds_val = [matrix[j, k, l] if (matrix[j, k, l]!= background_id) else 0
                                 for j in sub_range_x for k in sub_range_y for l in sub_range_z]
                    for ind_loc, val_loc in enumerate(conds_loc):
                        if conds_loc[ind_loc]==1 and conds_val[ind_loc]!=0:
                            if conds_val[ind_loc] not in lab and conds_val[ind_loc]!=neigh_dict[key]['Label']:
                                lab_append(conds_val[ind_loc])

                if neigh_dict[key]['Label'] in lab:
                    lab_remove(neigh_dict[key]['Label'])
                lab_sort()
                neigh_dict[key]['Neighbors'] = lab

            #--- symetric
            for key_1 in neigh_dict:
                tmp_lab, tmp_neigh = neigh_dict[key_1]['Label'], neigh_dict[key_1]['Neighbors']
                for neigh in tmp_neigh:
                    for key_2 in neigh_dict:
                        if neigh_dict[key_2]['Label'] == neigh:
                            tmp_neigh_2 = neigh_dict[key_2]['Neighbors']
                            if tmp_lab not in tmp_neigh_2:
                                tmp_neigh_2.append(tmp_lab)
                                tmp_neigh_2.sort()
                                neigh_dict[key_2]['Neighbors'] = tmp_neigh_2

            #--- Distances
            for key in neigh_dict:
                tmp_centroid, tmp_neigh = neigh_dict[key]['Physical centroid'], neigh_dict[key]['Neighbors']
                neigh_dict[key]['Distances'] = {}
                for neigh in tmp_neigh:
                    if neigh_dict[neigh]['Label']==neigh:
                        centroid = neigh_dict[neigh]['Physical centroid']
                        if dim==2:
                            val = np_sqrt( pow((tmp_centroid[0]-centroid[0]),2) + pow((tmp_centroid[1]-centroid[1]),2))
                        elif dim==3:
                            val = np_sqrt( pow((tmp_centroid[0]-centroid[0]),2) + pow((tmp_centroid[1]-centroid[1]),2)
                                            + pow((tmp_centroid[2]-centroid[2]),2))
                        val = around_val(val)
                        neigh_dict[key]['Distances'][neigh] = val

            end_time = time.time()
            print('Number of labels : '), len(label)
            print('Local rag approx, elapsed time : '), end_time - start_time
            return neigh_dict
        else:
            print('Index list :'), self._label_range_max
            return


    def compute_2D_local_rag(self, centroid_dict={}, background_id=1,
                             connectivity='8-neighbors', exploration_length=0):
        """
        Compute 2D local region adjacency

        Parameters
        ----------
        :param dict centroid_dict: dictionary of centroids, optional, default={}

        :param int background_id: background label, optional, default==1

        :param str connectivity: considered connectivity, optional, default: '8-neighbors'
                                 connectivity can be either : ['4-neighbors', '8-neighbors']

        :param int exploration_length: exploration_length (pixel), if 0 an automatic exploration_length is computed

        Returns
        ----------
        :return: neigh_dict (*dict*) -- dictionary of local adjacencies.

        Example
        -------
        >>> neigh_dict = self.compute_2D_local_rag(centroid_dict, background_id, connectivity, exploration_length)
        """
        if self.int_labels is not None and self._dim==2:
            start_time = time.time()
            matrix, shape, label = self._segmentation, self._shape, self.int_labels

            if background_id in label:
                label.remove(background_id)

            if centroid_dict=={}:
                centroid_dict = self.compute_centroid()
            else:
                for key in centroid_dict:
                    if (centroid_dict[key]['Label']==background_id):
                        del centroid_dict[key]

            connect = ['4-neighbors', '8-neighbors']
            if connectivity not in connect:
                print('Possible connectivity : '), connect
                connectivity = '8-neighbors'
                print('Setting connectivity to : '), connectivity
            else:
                connectivity = str(connectivity)

            if exploration_length==0:

                bbox_size_list, x_list, y_list = [], [], []
                bounding_box_dict = self.compute_bounding_box()
                bbox_size_list_append = bbox_size_list.append
                x_list_append, y_list_append = x_list.append, y_list.append

                for key in bounding_box_dict:
                    bbox_size_list_append(bounding_box_dict[key]['Bounding box size'])
                for ind, bbox in enumerate(bbox_size_list):
                    x_list_append(bbox_size_list[ind][0])
                    y_list_append(bbox_size_list[ind][1])

                inc = max(max(x_list), max(y_list))
                exploration_length = int(inc/2.0)

            else:
                exploration_length = int(abs(exploration_length))

            range_x, range_y = xrange(shape[0]), xrange(shape[1])
            dir_4N = [[0, 1], [-1, 0], [0, -1], [1, 0]]
            dir_8N = [[1, 1], [-1, 1], [-1, -1], [1, -1]]

            neigh_dict = {}
            for key in centroid_dict:
                neigh_dict[key] = {}
                neigh_dict[key]['Label'], neigh_dict[key]['Index centroid'], neigh_dict[key]['Physical centroid'] = centroid_dict[key]['Label'], centroid_dict[key]['Index centroid'], centroid_dict[key]['Physical centroid']
                index_x, index_y = neigh_dict[key]['Index centroid'][0], neigh_dict[key]['Index centroid'][1]

                lab = []
                lab_append, lab_remove, lab_sort = lab.append, lab.remove, lab.sort

                if ((index_x in range_x) and (index_y in range_y)):
                    #--- initialization
                    init_label = matrix[index_x, index_y]
                    if ((init_label != background_id) and (len(lab)==0)):
                        lab_append(init_label)
                    elif ((init_label == background_id) and (len(lab)==0)):
                        for tmp_dir in dir_4N:
                            step_x, step_y = 0, 0
                            init_label = matrix[index_x + step_x, index_y + step_y]
                            conds_loc = ((index_x + step_x) in range_x) and ((index_y + step_y) in range_y)
                            conds_expl = (np_abs(step_x) <= exploration_length) and (np_abs(step_y) <= exploration_length)
                            while ((init_label == background_id) and (len(lab)==0) and conds_expl and conds_loc):
                                step_x, step_y = step_x + tmp_dir[0], step_y + tmp_dir[1]
                                conds_loc = ((index_x + step_x) in range_x) and ((index_y + step_y) in range_y)
                                conds_expl = (np_abs(step_x) <= exploration_length) and (np_abs(step_y) <= exploration_length)
                                if conds_loc and conds_expl:
                                    init_label = matrix[index_x + step_x, index_y + step_y]
                                    if ((init_label != background_id) and (len(lab)==0)):
                                        lab_append(init_label)
                                        break
                                else:
                                    break

                    # 4-neighbors
                    for tmp_dir in dir_4N:
                        step_x, step_y = 0, 0
                        val = matrix[index_x + step_x, index_y + step_y]
                        while ((val == init_label) or (val == background_id)):
                            step_x, step_y = step_x + tmp_dir[0], step_y + tmp_dir[1]
                            conds_loc = ((index_x + step_x) in range_x) and ((index_y + step_y) in range_y)
                            conds_expl = (np_abs(step_x) <= exploration_length) and (np_abs(step_y) <= exploration_length)
                            if conds_loc and conds_expl:
                                val = matrix[index_x + step_x, index_y + step_y]
                                if ((init_label != val) and (val != background_id)):
                                    if val not in lab:
                                        lab_append(val)
                                        break
                            else:
                                break

                    # 8-neighbors
                    if (connectivity == '8-neighbors'):
                        for tmp_dir in dir_8N:
                            step_x, step_y = 0, 0
                            val = matrix[index_x + step_x, index_y + step_y]
                            while ((init_label == val) or (val == background_id)):
                                step_x, step_y = step_x + tmp_dir[0], step_y + tmp_dir[1]
                                conds_loc = ((index_x + step_x) in range_x) and ((index_y + step_y) in range_y)
                                conds_expl = (np_abs(step_x) <= exploration_length) and (np_abs(step_y) <= exploration_length)
                                if conds_loc and conds_expl:
                                    val = matrix[index_x + step_x, index_y + step_y]
                                    if ((init_label != val) and (val != background_id)):
                                        if val not in lab:
                                            lab_append(val)
                                            break
                                else:
                                    break

                    if neigh_dict[key]['Label'] in lab:
                        lab_remove(neigh_dict[key]['Label'])
                        lab_sort()
                    neigh_dict[key]['Neighbors'] = lab

            #--- symetric
            for key_1 in neigh_dict:
                tmp_lab, tmp_neigh = neigh_dict[key_1]['Label'], neigh_dict[key_1]['Neighbors']
                for neigh in tmp_neigh:
                    for key_2 in neigh_dict:
                        if neigh_dict[key_2]['Label'] == neigh:
                            tmp_neigh_2 = neigh_dict[key_2]['Neighbors']
                            if tmp_lab not in tmp_neigh_2:
                                tmp_neigh_2.append(tmp_lab)
                                tmp_neigh_2.sort()
                                neigh_dict[key_2]['Neighbors'] = tmp_neigh_2

            #--- Distances
            for key in neigh_dict:
                tmp_centroid, tmp_neigh = neigh_dict[key]['Physical centroid'], neigh_dict[key]['Neighbors']
                neigh_dict[key]['Distances'] = {}
                for neigh in tmp_neigh:
                    if neigh_dict[neigh]['Label']==neigh:
                        centroid = neigh_dict[neigh]['Physical centroid']
                        val = np_sqrt( pow((tmp_centroid[0]-centroid[0]),2) + pow((tmp_centroid[1]-centroid[1]),2))
                        val = around_val(val)
                        neigh_dict[key]['Distances'][neigh] = val

            end_time = time.time()
            print('Number of labels : '), len(label)
            print('2D local rag, elapsed time : '), end_time - start_time
            return neigh_dict
        else:
            print('Index list :'), self._label_range_max
            return


    def compute_3D_local_rag(self, centroid_dict={}, background_id=1,
                          connectivity='26-neighbors', exploration_length=0):
        """
        Compute 3D local region adjacency.

        Parameters
        ----------
        :param dict centroid_dict: dictionary of centroids, optional, default={}

        :param int background_id: background label, optional, default==1

        :param str connectivity: considered connectivity, default: '26-neighbors'
                                 connectivity can be either : ['6-neighbors', '18-neighbors', '26-neighbors']

        :param int exploration_length: exploration_length (voxel), if 0 an automatic exploration_length is computed

        Returns
        ----------
        :return: neigh_dict (*dict*) -- dictionary of local adjacencies

        Example
        -------
        >>> neigh_dict = self.compute_3D_local_rag(centroid_dict, background_id, connectivity, exploration_length)
        """
        if self.int_labels is not None and self._dim==3:
            start_time = time.time()
            matrix, shape, label = self._segmentation, self._shape, self.int_labels

            if background_id in label:
                label.remove(background_id)

            if centroid_dict=={}:
                centroid_dict = self.compute_centroid()
            else:
                for key in centroid_dict:
                    if (centroid_dict[key]['Label']==background_id):
                        del centroid_dict[key]

            connect = ['6-neighbors', '18-neighbors', '26-neighbors']
            if connectivity not in connect:
                print('Possible connectivity : '), connect
                connectivity = '26-neighbors'
                print('Setting connectivity to : '), connectivity
            else:
                connectivity = str(connectivity)

            if exploration_length==0:
                bbox_size_list, x_list, y_list, z_list = [], [], [], []
                bbox_size_list_append = bbox_size_list.append
                x_list_append, y_list_append, z_list_append = x_list.append, y_list.append, z_list.append
                bounding_box_dict = self.compute_bounding_box()
                for key in bounding_box_dict:
                    bbox_size_list_append(bounding_box_dict[key]['Bounding box size'])
                for ind, bbox in enumerate(bbox_size_list):
                    x_list_append(bbox_size_list[ind][0])
                    y_list_append(bbox_size_list[ind][1])
                    z_list_append(bbox_size_list[ind][2])
                inc = max(max(x_list), max(y_list), max(z_list))
                exploration_length = int(inc/2.0)
            else:
                exploration_length = int(abs(exploration_length))

            range_x, range_y, range_z = xrange(shape[0]), xrange(shape[1]), xrange(shape[2])
            dir_6N = [[1, 0, 0], [0, 1, 0], [-1, 0, 0], [0, -1, 0], [0, 0, -1], [0, 0, 1]]
            dir_18N = [[1, 1, 0], [-1, 1, 0], [0, 1, -1], [0, 1, 1], [-1, 0, 1], [1, 0, 1],
                           [-1, 0, -1], [1, 0, -1], [0, -1, 1], [0, -1, -1], [1, -1, 0], [-1, -1, 0]]
            dir_26N = [[1, 1, 1], [1, 1, -1], [-1, 1, 1], [-1, 1, -1], [1, -1, 1], [-1, -1, 1],
                           [1, -1, -1], [-1, -1, -1]]

            neigh_dict = {}
            for key in centroid_dict:
                neigh_dict[key] = {}
                neigh_dict[key]['Label'], neigh_dict[key]['Index centroid'], neigh_dict[key]['Physical centroid'] = centroid_dict[key]['Label'], centroid_dict[key]['Index centroid'], centroid_dict[key]['Physical centroid']
                index_x, index_y, index_z = neigh_dict[key]['Index centroid'][0], neigh_dict[key]['Index centroid'][1], neigh_dict[key]['Index centroid'][2]
                lab = []
                lab_append, lab_remove, lab_sort = lab.append, lab.remove, lab.sort

                if ((index_x in range_x) and (index_y in range_y) and (index_z in range_z)):
                    #--- initialization
                    init_label = matrix[index_x, index_y, index_z]
                    if ((init_label != background_id) and (len(lab)==0)):
                        lab_append(init_label)
                    elif ((init_label == background_id) and (len(lab)==0)):
                        for tmp_dir in dir_6N:
                            step_x, step_y, step_z = 0, 0, 0
                            init_label = matrix[index_x + step_x, index_y + step_y, index_z + step_z]
                            conds_loc = (((index_x + step_x) in range_x) and ((index_y + step_y) in range_y) and
                                            ((index_z + step_z) in range_z))
                            conds_expl = ((np_abs(step_x) <= exploration_length) and (np_abs(step_y) <= exploration_length) and
                                             (np_abs(step_z) <= exploration_length))
                            while ((init_label == background_id) and (len(lab)==0) and conds_expl and conds_loc):
                                step_x, step_y, step_z = step_x + tmp_dir[0], step_y + tmp_dir[1], step_z + tmp_dir[2]
                                conds_loc = (((index_x + step_x) in range_x) and ((index_y + step_y) in range_y) and
                                                ((index_z + step_z) in range_z))
                                conds_expl = ((np_abs(step_x) <= exploration_length) and (np_abs(step_y) <= exploration_length) and
                                                 (np_abs(step_z) <= exploration_length))
                                if conds_loc and conds_expl:
                                    init_label = matrix[index_x + step_x, index_y + step_y, index_z + step_z]
                                    if ((init_label != background_id) and (len(lab)==0)):
                                        lab_append(init_label)
                                        break
                                else:
                                    break

                    # 6-neighbors
                    for tmp_dir in dir_6N:
                        step_x, step_y, step_z = 0, 0, 0
                        val = matrix[index_x + step_x, index_y + step_y, index_z + step_z]
                        while ((val == init_label) or (val == background_id)):
                            step_x, step_y, step_z = step_x + tmp_dir[0], step_y + tmp_dir[1], step_z + tmp_dir[2]
                            conds_loc = (((index_x + step_x) in range_x) and ((index_y + step_y) in range_y) and
                                            ((index_z + step_z) in range_z))
                            conds_expl = ((np_abs(step_x) <= exploration_length) and (np_abs(step_y) <= exploration_length) and
                                             (np_abs(step_z) <= exploration_length))
                            if conds_loc and conds_expl:
                                val = matrix[index_x + step_x, index_y + step_y, index_z + step_z]
                                if ((init_label != val) and (val != background_id)):
                                    if val not in lab:
                                        lab_append(val)
                                        break
                            else:
                                break

                    # 18-neighbors
                    if (connectivity == '18-neighbors') or (connectivity == '26-neighbors'):
                        for tmp_dir in dir_18N:
                            step_x, step_y, step_z = 0, 0, 0
                            val = matrix[index_x + step_x, index_y + step_y, index_z + step_z]
                            while ((init_label == val) or (val == background_id)):
                                step_x, step_y, step_z = step_x + tmp_dir[0], step_y + tmp_dir[1], step_z + tmp_dir[2]
                                conds_loc = (((index_x + step_x) in range_x) and ((index_y + step_y) in range_y) and
                                                ((index_z + step_z) in range_z))
                                conds_expl = ((np_abs(step_x) <= exploration_length) and (np_abs(step_y) <= exploration_length) and
                                                 (np_abs(step_z) <= exploration_length))
                                if conds_loc and conds_expl:
                                    val = matrix[index_x + step_x, index_y + step_y, index_z + step_z]
                                    if ((init_label != val) and (val != background_id)):
                                        if val not in lab:
                                            lab_append(val)
                                            break
                                else:
                                    break

                    # 26-neighbors
                    if (connectivity == '26-neighbors'):
                        for tmp_dir in dir_26N:
                            step_x, step_y, step_z = 0, 0, 0
                            val = matrix[index_x + step_x, index_y + step_y, index_z + step_z]
                            while ((init_label == val) or (val == background_id)):
                                step_x, step_y, step_z = step_x + tmp_dir[0], step_y + tmp_dir[1], step_z + tmp_dir[2]
                                conds_loc = (((index_x + step_x) in range_x) and ((index_y + step_y) in range_y) and
                                                ((index_z + step_z) in range_z))
                                conds_expl = ((np_abs(step_x) <= exploration_length) and (np_abs(step_y) <= exploration_length) and
                                                 (np_abs(step_z) <= exploration_length))
                                if conds_loc and conds_expl:
                                    val = matrix[index_x + step_x, index_y + step_y, index_z + step_z]
                                    if ((init_label != val) and (val != background_id)):
                                        if val not in lab:
                                            lab_append(val)
                                            break
                                else:
                                    break

                    if neigh_dict[key]['Label'] in lab:
                        lab_remove(neigh_dict[key]['Label'])
                        lab_sort()
                    neigh_dict[key]['Neighbors'] = lab

            #--- symetric
            for key_1 in neigh_dict:
                tmp_lab, tmp_neigh = neigh_dict[key_1]['Label'], neigh_dict[key_1]['Neighbors']
                for neigh in tmp_neigh:
                    for key_2 in neigh_dict:
                        if neigh_dict[key_2]['Label'] == neigh:
                            tmp_neigh_2 = neigh_dict[key_2]['Neighbors']
                            if tmp_lab not in tmp_neigh_2:
                                tmp_neigh_2.append(tmp_lab)
                                tmp_neigh_2.sort()
                                neigh_dict[key_2]['Neighbors'] = tmp_neigh_2

            #--- Distances
            for key in neigh_dict:
                tmp_centroid, tmp_neigh = neigh_dict[key]['Physical centroid'], neigh_dict[key]['Neighbors']
                neigh_dict[key]['Distances'] = {}
                for neigh in tmp_neigh:
                    if neigh_dict[neigh]['Label']==neigh:
                        centroid = neigh_dict[neigh]['Physical centroid']
                        val = np_sqrt( pow((tmp_centroid[0]-centroid[0]),2) + pow((tmp_centroid[1]-centroid[1]),2)
                                        + pow((tmp_centroid[2]-centroid[2]),2))
                        val = around_val(val)
                        neigh_dict[key]['Distances'][neigh] = val

            end_time = time.time()
            print('Number of labels : '), len(label)
            print('3D local rag, elapsed time : '), end_time - start_time
            return neigh_dict
        else:
            print('Index list :'), self._label_range_max
            return


    def compute_local_rag(self, centroid_dict={}, background_id=1,
                          connectivity='26-neighbors', exploration_length=0):
        """
        Compute local region adjacency (2D or 3D).

        Parameters
        ----------
        :param dict centroid_dict: dictionary of centroids, optional, default={}

        :param int background_id: background label, optional, default==1

        :param str connectivity: considered connectivity, default: '26-neighbors'
                                 connectivity can be either : ['4-neighbors', '6-neighbors', '8-neighbors',
                                                               '18-neighbors', '26-neighbors']

        :param int exploration_length: exploration_length (voxel), if 0 an automatic exploration_length is computed

        Returns
        ----------
        :return: neigh_dict (*dict*) -- dictionary of local adjacencies

        Example
        -------
        >>> neigh_dict = self.compute_local_rag(centroid_dict, background_id, connectivity, exploration_length)
        """
        if self.int_labels is not None and self._dim in [2,3]:
            dim, label = self._dim, self.int_labels

            if background_id in label:
                label.remove(background_id)

            if centroid_dict=={}:
                centroid_dict = self.compute_centroid()
            else:
                for key in centroid_dict:
                    if (centroid_dict[key]['Label']==background_id):
                        del centroid_dict[key]

            connect_2D = ['4-neighbors', '8-neighbors']
            connect_3D = ['6-neighbors', '18-neighbors', '26-neighbors']
            if dim==2:
                if connectivity not in connect_2D:
                    print('Possible connectivity : '), connect_2D
                    connectivity = '8-neighbors'
                    print('Setting connectivity to : '), connectivity
                else:
                    connectivity = str(connectivity)
            elif dim==3:
                if connectivity not in connect_3D:
                    print('Possible connectivity : '), connect_3D
                    connectivity = '26-neighbors'
                    print('Setting connectivity to : '), connectivity
                else:
                    connectivity = str(connectivity)

            if exploration_length==0:

                bounding_box_dict = self.compute_bounding_box()
                bbox_size_list, x_list, y_list, z_list = [], [], [], []
                bbox_size_list_append = bbox_size_list.append
                x_list_append, y_list_append, z_list_append = x_list.append, y_list.append, z_list.append
                for key in bounding_box_dict:
                    bbox_size_list_append(bounding_box_dict[key]['Bounding box size'])
                for ind, val in enumerate(bbox_size_list):
                    x_list_append(bbox_size_list[ind][0])
                    y_list_append(bbox_size_list[ind][1])
                    if dim==3:
                        z_list_append(bbox_size_list[ind][2])

                if dim==2:
                    inc = max(max(x_list), max(y_list))
                elif dim==3:
                    inc = max(max(x_list), max(y_list), max(z_list))
                    #inc = max(np.median(x_list),np.median(y_list),np.median(z_list))

                exploration_length = int(inc/2.0)

            else:
                exploration_length = int(abs(exploration_length))

            if dim==2:
                neigh_dict = self.compute_2D_local_rag(centroid_dict, background_id, connectivity, exploration_length)
            elif dim==3:
                neigh_dict = self.compute_3D_local_rag(centroid_dict, background_id, connectivity, exploration_length)

            return neigh_dict
        else:
            print('Index list :'), self._label_range_max
            return


    def compute_feature_space(self, centroid_dict={}, volume_dict={},
                                    neigh_dict={}, bbox_dict={}, background_id=1):
        """
        Multi-component approach - build a feature space.

        Parameters
        ----------
        :param dict centroid_dict: dictionary of centroids

        :param dict volume_dict: dictionary of volumes

        :param dict neigh_dict: dictionary of local adjacencies

        :param dict bbox_dict: dictionary of local bounding boxes

        :param int background_id: background label, default==1

        Returns
        ----------
        :return: feature_space_dict (*dict*) -- dictionary of features

        Example
        ----------
        >>> feature_space_dict = self.compute_feature_space_label(centroid_dict, volume_dict,
                                                                  neigh_dict, bbox_dict, background_id)
        """
        if self.int_labels is not None and self._dim in [2,3]:
            start_time = time.time()
            feature_space_dict = {}
            label = self.int_labels

            if background_id in label:
                label.remove(background_id)
                self.set_labels(label)

            if bbox_dict=={}:
                bbox_dict = self.compute_bounding_box() ### --- bounding box (discrete space)
            else:
                for key in bbox_dict:
                    if (bbox_dict[key]['Label']==background_id):
                        del bbox_dict[key]

            if centroid_dict=={}:
                centroid_dict = self.compute_centroid() ### --- shape approximation (continuous space)
            else:
                for key in centroid_dict:
                    if (centroid_dict[key]['Label']==background_id):
                        del centroid_dict[key]

            if volume_dict=={}:
                volume_dict = self.compute_volume() ### --- volume approximation (continuous space)
            else:
                for key in volume_dict:
                    if (volume_dict[key]['Label']==background_id):
                        del volume_dict[key]

            if neigh_dict=={}:
                ### --- neighborhood information
                #neigh_dict = self.compute_local_rag_approx(centroid_dict=centroid_dict, background_id=background_id)
                neigh_dict = self.compute_local_rag(centroid_dict=centroid_dict, background_id=background_id)
            else:
                for key in neigh_dict:
                    if (neigh_dict[key]['Label']==background_id):
                        del neigh_dict[key]

            for key in centroid_dict:
                feature_space_dict[key] = centroid_dict[key].copy()
                feature_space_dict[key].update(bbox_dict[key])
                feature_space_dict[key].update(volume_dict[key])
                feature_space_dict[key].update(neigh_dict[key])
            end_time = time.time()
            print('Number of labels : '), len(label)
            print('Feature space, elapsed time : '), end_time - start_time
            return feature_space_dict
        else:
            print('Index list :'), self._label_range_max
            return


    def compute_3D_layers(self, centroid_dict={}, background_id=1,
                          connectivity='26-neighbors', exploration_length=0, opt=None):
        """
        Extract L1 and/or L2 and/or L3 layers (only 3D algorithm)

        Hypothesis : L1 is connected to background, L2 is connected to L1, etc.

        Parameters
        ----------
        :param dict centroid_dict: dictionary of centroids

        :param int background_id: background label, default==1

        :param str connectivity: considered connectivity, default: '26-neighbors'
                                 connectivity can be either : ['6-neighbors','18-neighbors', '26-neighbors']

        :param int exploration_length: exploration_length (voxel), if 0 an automatic exploration_length is computed

        :param str opt: layers option, default: 'L1'
                        opt can be either : ['L1','L2','L3','L1L2','L1L2L3']

        Returns
        ----------
        :return: layer_dict (*dict*) -- dictionary of layers and features

        Example
        ----------
        >>> layer_dict = self.compute_3D_layers()
        """

        if self.int_labels is not None and self._dim==3:
            start_time = time.time()
            matrix, shape, label = self._segmentation, self._shape, self.int_labels

            if background_id in label:
                label.remove(background_id)

            if centroid_dict=={}:
                centroid_dict = self.compute_centroid()
            else:
                for key in centroid_dict:
                    if (centroid_dict[key]['Label']==background_id):
                        del centroid_dict[key]

            connect = ['6-neighbors', '18-neighbors', '26-neighbors']
            if connectivity not in connect:
                print('Possible connectivity : '), connect
                connectivity = '26-neighbors'
                print('Setting connectivity to : '), connectivity
            else:
                connectivity = str(connectivity)

            if exploration_length==0:
                bbox_size_list, x_list, y_list, z_list = [], [], [], []
                bbox_size_list_append = bbox_size_list.append
                x_list_append, y_list_append, z_list_append = x_list.append, y_list.append, z_list.append
                bounding_box_dict = self.compute_bounding_box()
                for key in bounding_box_dict:
                    bbox_size_list_append(bounding_box_dict[key]['Bounding box size'])
                for ind, bbox in enumerate(bbox_size_list):
                    x_list_append(bbox_size_list[ind][0])
                    y_list_append(bbox_size_list[ind][1])
                    z_list_append(bbox_size_list[ind][2])
                inc = max(max(x_list), max(y_list), max(z_list))
                exploration_length = int(inc/2.0)
            else:
                exploration_length = int(abs(exploration_length))

            opts = ['L1','L2','L3','L1L2','L1L2L3']
            if opt is None or opt not in opts:
                print('Options :'), opts
                opt==['L1']

            range_x, range_y, range_z = xrange(shape[0]), xrange(shape[1]), xrange(shape[2])
            dir_6N = [[1, 0, 0], [0, 1, 0], [-1, 0, 0], [0, -1, 0], [0, 0, -1], [0, 0, 1]]
            dir_18N = [[1, 1, 0], [-1, 1, 0], [0, 1, -1], [0, 1, 1], [-1, 0, 1], [1, 0, 1],
                           [-1, 0, -1], [1, 0, -1], [0, -1, 1], [0, -1, -1], [1, -1, 0], [-1, -1, 0]]
            dir_26N = [[1, 1, 1], [1, 1, -1], [-1, 1, 1], [-1, 1, -1], [1, -1, 1], [-1, -1, 1],
                           [1, -1, -1], [-1, -1, -1]]

            neigh_dict = {}
            for key in centroid_dict:
                neigh_dict[key] = {}
                neigh_dict[key]['Label'], neigh_dict[key]['Index centroid'], neigh_dict[key]['Physical centroid'] = centroid_dict[key]['Label'], centroid_dict[key]['Index centroid'], centroid_dict[key]['Physical centroid']
                index_x, index_y, index_z = neigh_dict[key]['Index centroid'][0], neigh_dict[key]['Index centroid'][1], neigh_dict[key]['Index centroid'][2]
                lab = []
                lab_append, lab_remove, lab_sort = lab.append, lab.remove, lab.sort

                if ((index_x in range_x) and (index_y in range_y) and (index_z in range_z)):
                    #--- initialization
                    init_label = matrix[index_x, index_y, index_z]
                    if ((init_label != background_id) and (len(lab)==0)):
                        lab_append(init_label)
                    elif ((init_label == background_id) and (len(lab)==0)):
                        for tmp_dir in dir_6N:
                            step_x, step_y, step_z = 0, 0, 0
                            init_label = matrix[index_x + step_x, index_y + step_y, index_z + step_z]
                            conds_loc = (((index_x + step_x) in range_x) and ((index_y + step_y) in range_y) and
                                            ((index_z + step_z) in range_z))
                            conds_expl = ((np_abs(step_x) <= exploration_length) and (np_abs(step_y) <= exploration_length) and
                                             (np_abs(step_z) <= exploration_length))
                            while ((init_label == background_id) and (len(lab)==0) and conds_expl and conds_loc):
                                step_x, step_y, step_z = step_x + tmp_dir[0], step_y + tmp_dir[1], step_z + tmp_dir[2]
                                conds_loc = (((index_x + step_x) in range_x) and ((index_y + step_y) in range_y) and
                                                ((index_z + step_z) in range_z))
                                conds_expl = ((np_abs(step_x) <= exploration_length) and (np_abs(step_y) <= exploration_length) and
                                                 (np_abs(step_z) <= exploration_length))
                                if conds_loc and conds_expl:
                                    init_label = matrix[index_x + step_x, index_y + step_y, index_z + step_z]
                                    if ((init_label != background_id) and (len(lab)==0)):
                                        lab_append(init_label)
                                        break
                                else:
                                    break

                    # 6-neighbors
                    for tmp_dir in dir_6N:
                        step_x, step_y, step_z = 0, 0, 0
                        val = matrix[index_x + step_x, index_y + step_y, index_z + step_z]
                        while (val == init_label):
                            step_x, step_y, step_z = step_x + tmp_dir[0], step_y + tmp_dir[1], step_z + tmp_dir[2]
                            conds_loc = (((index_x + step_x) in range_x) and ((index_y + step_y) in range_y) and
                                            ((index_z + step_z) in range_z))
                            conds_expl = ((np_abs(step_x) <= exploration_length) and (np_abs(step_y) <= exploration_length) and
                                             (np_abs(step_z) <= exploration_length))
                            if conds_loc and conds_expl:
                                val = matrix[index_x + step_x, index_y + step_y, index_z + step_z]
                                if (init_label != val):
                                    if val not in lab:
                                        lab_append(val)
                                        break
                            else:
                                break

                    # 18-neighbors
                    if (connectivity == '18-neighbors') or (connectivity == '26-neighbors'):
                        for tmp_dir in dir_18N:
                            step_x, step_y, step_z = 0, 0, 0
                            val = matrix[index_x + step_x, index_y + step_y, index_z + step_z]
                            while (init_label == val):
                                step_x, step_y, step_z = step_x + tmp_dir[0], step_y + tmp_dir[1], step_z + tmp_dir[2]
                                conds_loc = (((index_x + step_x) in range_x) and ((index_y + step_y) in range_y) and
                                                ((index_z + step_z) in range_z))
                                conds_expl = ((np_abs(step_x) <= exploration_length) and (np_abs(step_y) <= exploration_length) and
                                                 (np_abs(step_z) <= exploration_length))
                                if conds_loc and conds_expl:
                                    val = matrix[index_x + step_x, index_y + step_y, index_z + step_z]
                                    if (init_label != val):
                                        if val not in lab:
                                            lab_append(val)
                                            break
                                else:
                                    break

                    # 26-neighbors
                    if (connectivity == '26-neighbors'):
                        for tmp_dir in dir_26N:
                            step_x, step_y, step_z = 0, 0, 0
                            val = matrix[index_x + step_x, index_y + step_y, index_z + step_z]
                            while (init_label == val):
                                step_x, step_y, step_z = step_x + tmp_dir[0], step_y + tmp_dir[1], step_z + tmp_dir[2]
                                conds_loc = (((index_x + step_x) in range_x) and ((index_y + step_y) in range_y) and
                                                ((index_z + step_z) in range_z))
                                conds_expl = ((np_abs(step_x) <= exploration_length) and (np_abs(step_y) <= exploration_length) and
                                                 (np_abs(step_z) <= exploration_length))
                                if conds_loc and conds_expl:
                                    val = matrix[index_x + step_x, index_y + step_y, index_z + step_z]
                                    if (init_label != val):
                                        if val not in lab:
                                            lab_append(val)
                                            break
                                else:
                                    break

                    if neigh_dict[key]['Label'] in lab:
                        lab_remove(neigh_dict[key]['Label'])
                        lab_sort()
                    neigh_dict[key]['Neighbors'] = lab

            #--- symetric
            for key_1 in neigh_dict:
                tmp_lab, tmp_neigh = neigh_dict[key_1]['Label'], neigh_dict[key_1]['Neighbors']
                for neigh in tmp_neigh:
                    for key_2 in neigh_dict:
                        if neigh_dict[key_2]['Label'] == neigh:
                            tmp_neigh_2 = neigh_dict[key_2]['Neighbors']
                            if tmp_lab not in tmp_neigh_2:
                                tmp_neigh_2.append(tmp_lab)
                                tmp_neigh_2.sort()
                                neigh_dict[key_2]['Neighbors'] = tmp_neigh_2

            layer_dict, l1_dict, l2_dict, l3_dict = {}, {}, {}, {}

            for key in neigh_dict:
                if background_id in neigh_dict[key]['Neighbors']:
                    l1_dict[key] = neigh_dict[key]

            for key in l1_dict:
                for neigh in l1_dict[key]['Neighbors']:
                    if neigh!=background_id and neigh not in l1_dict:
                        l2_dict[neigh]=neigh_dict[neigh]

            for key in l2_dict:
                for neigh in l2_dict[key]['Neighbors']:
                    if (neigh!=background_id and neigh not in l1_dict
                            and neigh not in l2_dict):
                        l3_dict[neigh]=neigh_dict[neigh]

            if opt=='L1':
                layer_dict = l1_dict.copy()
            elif opt=='L2':
                layer_dict = l2_dict.copy()
            elif opt=='L3':
                layer_dict = l1_dict.copy()
            elif opt=='L1L2':
                layer_dict = l1_dict.copy()
                layer_dict.update(l2_dict)
            elif opt=='L1L2L3':
                layer_dict = l1_dict.copy()
                layer_dict.update(l2_dict)
                layer_dict.update(l3_dict)

            end_time = time.time()
            print('Number of labels (layers) : '), len(layer_dict)
            print('Layers extraction, elapsed time : '), end_time - start_time
            return layer_dict
        else:
            print('Index list :'), self._label_range_max
            return
        return


if __name__ == '__main__':
    print('')