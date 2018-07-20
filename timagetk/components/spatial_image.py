# -*- coding: utf-8 -*-
# -*- python -*-
#
#       Copyright 2016 INRIA
#
#       File author(s):
#           Sophie Ribes <sophie.ribes@inria.fr>
#           Jerome Chopard <jerome.chopard@inria.fr>
#
#       See accompanying file LICENSE.txt
# -----------------------------------------------------------------------------

# ----
# SR : update - 08/2016
# numpy types, tests and conds, 2D and 3D management
# origin, voxelsize, extent, mean, min, max and metadata
# set and get methods
# to_2D(), to_3D() methods
# documentation and unit tests (see test_spatial_image.py)

from __future__ import division

import numpy as np

__all__ = ['SpatialImage']

# - Define default values:
EPS = 1e-9
DEC_VAL = 6
DEF_VXS_2D, DEF_VXS_3D = [1.0, 1.0], [1.0, 1.0, 1.0]
DEF_ORIG_2D, DEF_ORIG_3D = [0, 0], [0, 0, 0]

# - Define possible values for 'dtype':
DICT_TYPES = {0: np.uint8, 1: np.int8, 2: np.uint16, 3: np.int16,
              4: np.uint32,
              5: np.int32, 6: np.uint64, 7: np.int64, 8: np.float32,
              9: np.float64,
              10: np.float_, 11: np.complex64, 12: np.complex128,
              13: np.complex_,
              'uint8': np.uint8, 'uint16': np.uint16, 'ushort': np.uint16,
              'uint32': np.uint32,
              'uint64': np.uint64, 'uint': np.uint64,
              'ulonglong': np.uint64, 'int8': np.int8,
              'int16': np.int16, 'short': np.int16, 'int32': np.int32,
              'int64': np.int64,
              'int': np.int64, 'longlong': np.int64, 'float16': np.float16,
              'float32': np.float32,
              'single': np.float32, 'float64': np.float64,
              'double': np.float64, 'float': np.float64,
              'float128': np.float_, 'longdouble': np.float_,
              'longfloat': np.float_,
              'complex64': np.complex64, 'singlecomplex': np.complex64,
              'complex128': np.complex128,
              'cdouble': np.complex128, 'cfloat': np.complex128,
              'complex': np.complex128,
              'complex256': np.complex_, 'clongdouble': np.complex_,
              'clongfloat': np.complex_,
              'longcomplex': np.complex_}
POSS_TYPES = sorted([k for k in DICT_TYPES.keys() if isinstance(k, str)])
DEF_TYPE = DICT_TYPES[0]


def around_list(input_list, dec_val=DEC_VAL):
    if isinstance(input_list, list) and len(input_list) > 0:
        output_list = [np.around(input_list[ind], decimals=dec_val).tolist()
                       for ind, val in enumerate(input_list)]
        return output_list
    else:
        return []


def tuple_to_list(input_tuple):
    if isinstance(input_tuple, tuple):
        output_list = [input_tuple[ind] for ind, val in enumerate(input_tuple)]
        output_list = around_list(output_list)
        return output_list
    else:
        return []

def dimensionality_test(dim, list2test):
    """ Quick testing of dimensionality with print in case of error."""
    d = len(list2test)
    try:
        assert d == dim
    except:
        raise ValueError("Provided values dimensionality ({}) is not of the same than the array ({})!".format(d, dim))

def tuple_array_to_list(val):
    """ Returns a list if a tuple or array is provided, else raise Error message."""
    if isinstance(val, np.ndarray):
        val = val.tolist()
    elif isinstance(val, tuple):
        val = list(val)
    if not isinstance(val, list):
        raise TypeError("Accepted type are tuple, list and np.array!")
    else:
        return val


def dimensionality_test(dim, list2test):
    """ Quick testing of dimensionality with print in case of error."""
    d = len(list2test)
    try:
        assert d == dim
    except:
        raise ValueError(
            "Provided values dimensionality ({}) is not of the same than the array ({})!".format(
                d, dim))


def tuple_array_to_list(val):
    """ Returns a list if a tuple or array is provided, else raise Error message."""
    if isinstance(val, np.ndarray):
        val = val.tolist()
    elif isinstance(val, tuple):
        val = list(val)
    if not isinstance(val, list):
        raise TypeError("Accepted type are tuple, list and np.array!")
    else:
        return val


def basic_metadata(obj, metadata_dict=None):
    """
    Build the metadata dictionary for basics image array infos.
    Can compare it to a existing one.

    Parameters
    ----------
    obj : SpatialImage
        a SpatialImage to use for metadata definition
    metadata_dict : dict, optional
        a metadata dictionary to compare to the object variables

    Returns
    -------
    metadata_dict : dict
        a verified metadata dictionary
    """
    if not metadata_dict:
        metadata_dict = {}

    # -- Check the most important object values against potentially
    # predefined values in the metadata_dict:
    try:
        assert np.alltrue(metadata_dict['voxelsize'] == obj.voxelsize)
    except KeyError:
        metadata_dict['voxelsize'] = obj.voxelsize
    except AssertionError:
        raise ValueError(
            "Metadata 'voxelsize' does not match the object voxelsize!")
    try:
        assert metadata_dict['shape'] == obj.shape
    except KeyError:
        metadata_dict['shape'] = obj.shape
    except AssertionError:
        print "WARNING: Metadata 'shape' {} do not match the array shape {},".format(
            metadata_dict['shape'], obj.shape),
        metadata_dict['shape'] = obj.shape
        print "it has been updated!"

    try:
        assert metadata_dict['dim'] == obj.ndim
    except KeyError:
        metadata_dict['dim'] = obj.ndim
    except AssertionError:
        raise ValueError("Metadata 'dim' does not match the array dim!")

    try:
        assert np.alltrue(metadata_dict['origin'] == obj.origin)
    except KeyError:
        metadata_dict['origin'] = obj.origin
    except AssertionError:
        raise ValueError(
            "Metadata 'origin' does not match the object origin!")

    try:
        assert np.alltrue(metadata_dict['extent'] == obj.extent)
    except KeyError:
        metadata_dict['extent'] = obj.extent
    except AssertionError:
        raise ValueError(
            "Metadata 'extent' does not match the array extent!")

    try:
        assert metadata_dict['type'] == str(obj.dtype)
    except KeyError:
        metadata_dict['type'] = str(obj.dtype)
    except AssertionError:
        print "WARNING: Metadata 'type' ({}) do not match the array dtype ({}),".format(
            metadata_dict['type'], obj.dtype),
        metadata_dict['dtype'] = str(obj.dtype)
        print "it has been updated!"

    # Next lines compute min, max and mean values of the array every time we
    # call SpatialImage, even when reading the file from disk! This slow down
    # the process!
    # metadata_dict['min'] = obj.min()
    # metadata_dict['max'] = obj.max()
    # metadata_dict['mean'] = obj.mean()
    return metadata_dict


class SpatialImage(np.ndarray):
    """
    This class allows a management of ``SpatialImage`` objects (2D and 3D images).
    A ``SpatialImage`` gathers a numpy array and some metadata (such as voxelsize,
    physical extent, origin, type, etc.).
    Through a ``numpy.ndarray`` inheritance, all usual operations on
    `numpy.ndarray <http://docs.scipy.org/doc/numpy-1.10.1/reference/generated/numpy.ndarray.html>`_
    objects (sum, product, transposition, etc.) are available.
    All image processing operations are performed on this data structure, that
    is also used to solve inputs (read) and outputs (write).
    """

    def __new__(cls, input_array, origin=None, voxelsize=None, dtype=None,
                metadata_dict=None):
        """
        ``SpatialImage`` constructor (2D and 3D images)

        Parameters
        ----------
        :param *numpy.ndarray* input_array: image

        :param list origin: image origin, optional. Default: [0,0] or [0,0,0]

        :param list voxelsize: image voxelsize, optional. Default: [1.0,1.0] or [1.0,1.0,1.0]

        :param str dtype: image type, optional. Default: dtype=input_array.dtype

        :param dict metadata_dict: image metadata, optional. Default: metadata_dict={}
0
        Returns
        -------
        :return: ``SpatialImage`` instance -- image and metadata

        Notes
        -----
        To use 'input_array' type (from numpy), leave 'dtype' to None.

        Example
        -------
        >>> import numpy as np
        >>> from timagetk.components import SpatialImage
        >>> test_array = np.ones((5,5), dtype=np.uint8)
        >>> image_1 = SpatialImage(input_array=test_array)
        >>> image_2 = SpatialImage(input_array=test_array, voxelsize=[0.5,0.5])
        >>> isinstance(image_1, SpatialImage) and isinstance(image_2, SpatialImage)
        True
        """
        # TODO: SpatialImage should have 'filename' attribute or metadata!
        # - Test input array: should be a numpy array of dimension 2 or 3:
        try:
            assert isinstance(input_array, np.ndarray)
        except AssertionError:
            raise TypeError("Input array is not a numpy array!")
        try:
            assert input_array.ndim in [2, 3]
        except AssertionError:
            print ValueError(
                "Input array must have a dimensionality equal to 2 or 3")
            return None  # WEIRD behavior... seems required by some functions
            # Original behavior was to test if the dim was 2D or 3D but was not
            # doing anything otherwise (no else declared!).

        # - Create the metadata dictionary:
        if metadata_dict is None or metadata_dict == []:
            metadata_dict = {}
        else:
            try:
                assert isinstance(metadata_dict, dict)
            except:
                raise TypeError(
                    "Parameter 'metadata_dict' should be a dictionary!")

        # - Check or set the type of data from 'dtype':
        if dtype is not None:
            try:
                assert dtype in POSS_TYPES
            except AssertionError:
                raise ValueError(
                    "Unknown 'dtype' value '{}', available types are: {}".format(
                        dtype, POSS_TYPES))
            else:
                if isinstance(dtype, np.dtype):
                    dtype = str(dtype)
                dtype = DICT_TYPES[dtype]
        else:
            # Convert instance 'np.dtype' into a string:
            if input_array.dtype is not None:
                dtype = input_array.dtype
            else:
                print "Warning: undefined 'dtype',"
                print "set to default value:", DEF_TYPE
                dtype = DEF_TYPE

        # - Check input array 'flags' attribute, use 'dtype' value:
        if input_array.flags.f_contiguous:
            obj = np.asarray(input_array, dtype=dtype).view(cls)
        else:
            obj = np.asarray(input_array, dtype=dtype, order='F').view(cls)

        # - Check & set 'origins' value:
        if (origin is not None) and (input_array.ndim == len(origin)):
            orig = origin
            # - Update metadata in case voxelsize parameter is correctly
            # specified (to avoid potential missmatch against metadata_dict)
            metadata_dict['origin'] = orig
        else:
            # TODO: should check against metadata
            print "Warning: incorrect origin specification,",
            if input_array.ndim == 2:
                orig = DEF_ORIG_2D
            else:
                orig = DEF_ORIG_3D
            print "set to default value:", orig
        obj.origin = orig

        # - Check & set 'voxelsize' value:
        if isinstance(voxelsize, tuple):  # SR --- BACK COMPAT
            voxelsize = tuple_to_list(voxelsize)
        if isinstance(voxelsize, np.ndarray):
            voxelsize = voxelsize.tolist()
        if not isinstance(voxelsize, list):
            voxelsize = None
        try:
            assert input_array.ndim == len(voxelsize)
        except:
            print "Warning: incorrect voxelsize specification,",
            # TODO: should check against metadata
            if input_array.ndim == 2:
                vxs = DEF_VXS_2D
            else:
                vxs = DEF_VXS_3D
            print "set to default value:", vxs
        else:
            vxs = around_list(voxelsize)
            # - Update metadata in case voxelsize parameter is correctly
            # specified (to avoid potential missmatch against metadata_dict)
            metadata_dict['voxelsize'] = vxs
        obj.voxelsize = vxs

        # - Check & set 'extent' value:
        ext = [obj.voxelsize[ind] * input_array.shape[ind] for ind in
               xrange(input_array.ndim)]
        ext = around_list(ext)
        metadata_dict['extent'] = ext
        obj.extent = ext

        # - Save/update the metadata:
        obj.metadata = basic_metadata(obj, metadata_dict)

        # - Backward compatibility with 'openalea.image' `SpatiaImage`:
        obj.resolution = obj.voxelsize
        return obj

    def __array_finalize__(self, obj):
        """
        This is the mechanism that numpy provides to allow subclasses to handle
        the various ways that new instances get created.

        Parameters
        ----------
        obj : the object returned by the __new__ method.
        """
        if obj is not None:
            self.voxelsize = getattr(obj, 'voxelsize', [])
            self.origin = getattr(obj, 'origin', [])
            self.extent = getattr(obj, 'extent', [])
            self.min = getattr(obj, 'min', [])
            self.max = getattr(obj, 'max', [])
            self.mean = getattr(obj, 'mean', [])
            self.metadata = getattr(obj, 'metadata', {})
        else:
            return

    def __str__(self):
        print "SpatialImage object with following metadata:"
        print  self.get_metadata()
        return

    def is_isometric(self):
        """
        Test if the image is isometric, meaning the voxelsize value is the same
        in every direction.

        Returns
        -------
        is_iso : bool
            True is isometric, else False.
        """
        vxs = self.get_voxelsize()
        is_iso = np.alltrue([vxs_i == vxs[0] for vxs_i in vxs[1:]])
        return is_iso

    def resolution(self):
        """
        Ensure backward compatibility with older openalea.image package.
        """
        print DeprecationWarning(
            "Attribute 'resolution' is deprecated and 'voxelsize' or 'get_voxelsize()' should be used instead!")
        return self.voxelsize

    def equal(self, sp_img):
        """
        Equality test between two ``SpatialImage``

        Parameters
        ----------
        :param ``SpatialImage`` sp_img: ``SpatialImage`` instance

        Returns
        -------
        :returns: True/False (*bool*) -- if (array and metadata) are equal/or not

        Example
        -------
        >>> import numpy as np
        >>> from timagetk.components import SpatialImage
        >>> test_array = np.ones((5,5), dtype=np.uint8)
        >>> image_1 = SpatialImage(input_array=test_array)
        >>> image_1==image_1
        True
        >>> image_2 = SpatialImage(input_array=test_array, voxelsize=[0.5,0.5])
        >>> image_1==image_2
        False
        """
        val = False
        if not isinstance(sp_img, SpatialImage):
            raise TypeError("Parameter 'sp_img' is not a SpatialImage!")

        if self.get_shape() == sp_img.get_shape():
            # out_img = np.zeros_like(self, dtype=np.float)
            out_img = np.abs(self - sp_img)
            conds_arr = True if np.max(out_img < EPS) else False
            md_ref = self.get_metadata()
            md = sp_img.get_metadata()
            conds_met = all([True if md_ref[k] == md[k] else False for k in
                             self.get_metadata()])
            if conds_arr and conds_met:
                val = True
            elif not conds_arr:
                print "Max difference between arrays is greater than '{}' !".format(
                    EPS)
            elif not conds_met:
                print "SpatialImages metadata are different !"
            else:
                pass
        else:
            err = "Image 'sp_img' has a different shape than the current one!"
            raise ValueError(err)
        return val

    def get_array(self):
        """
        Get a ``numpy.ndarray`` from a ``SpatialImage``

        Returns
        ----------
        :returns: image_array (*numpy.ndarray*) -- ``SpatialImage`` array

        Example
        -------
        >>> import numpy as np
        >>> from timagetk.components import SpatialImage
        >>> test_array = np.ones((5,5), dtype=np.uint8)
        >>> image = SpatialImage(input_array=test_array)
        >>> image_array = image.get_array()
        >>> isinstance(image_array, SpatialImage)
        False
        >>> isinstance(image_array, np.ndarray)
        True
        """
        return np.array(self)

    def get_dim(self):
        """
        Get ``SpatialImage`` dimension (2D or 3D image)

        Returns
        ----------
        :returns: image_dim (*int*) -- ``SpatialImage`` dimension

        Example
        -------
        >>> import numpy as np
        >>> from timagetk.components import SpatialImage
        >>> test_array = np.ones((5,5), dtype=np.uint8)
        >>> image = SpatialImage(input_array=test_array)
        >>> image_dim = image.get_dim()
        >>> print image_dim
        2
        """
        return self.ndim

    def get_extent(self):
        """
        Get ``SpatialImage`` physical extent

        Returns
        ----------
        :returns: image_extent (*list*) -- ``SpatialImage`` physical extent

        Example
        -------
        >>> import numpy as np
        >>> from timagetk.components import SpatialImage
        >>> test_array = np.ones((5,5), dtype=np.uint8)
        >>> image = SpatialImage(input_array=test_array)
        >>> image_extent = image.get_extent()
        >>> print image_extent
        [5.0, 5.0]
        """
        return self.extent

    def get_metadata(self):
        """
        Get ``SpatialImage`` metadata

        Returns
        ----------
        :returns: image_metadata (*dict*) -- ``SpatialImage`` metadata

        Example
        -------
        >>> import numpy as np
        >>> from timagetk.components import SpatialImage
        >>> test_array = np.ones((5,5), dtype=np.uint8)
        >>> image = SpatialImage(input_array=test_array)
        >>> image_metadata = image.get_metadata()
        >>> print image_metadata
        {'dim': 2,
         'extent': [5.0, 5.0],
         'origin': [0, 0],
         'shape': (5, 5),
         'type': 'uint8',
         'voxelsize': [1.0, 1.0]
         }
        """
        met_dict = self.metadata
        try:
            assert met_dict.has_key('shape')
        except AssertionError:
            met_dict = basic_metadata(self, self.metadata)

        try:
            assert met_dict['shape'] == self.shape
        except AssertionError:
            old_shape = met_dict['shape']
            met_dict['shape'], met_dict['dim'], met_dict[
                'type'] = self.shape, self.ndim, str(self.dtype)
            if (self.ndim == 2 and old_shape[0] == self.shape[1] and old_shape[
                1] == self.shape[0]):  # --- transposition
                vox = [met_dict['voxelsize'][1], met_dict['voxelsize'][0]]
                ext = [met_dict['extent'][1], met_dict['extent'][0]]
                orig = [met_dict['origin'][1], met_dict['origin'][0]]
                met_dict['voxelsize'], met_dict['extent'], met_dict[
                    'origin'] = vox, ext, orig
                self.voxelsize, self.extent, self.origin = vox, ext, orig
            elif (self.ndim == 3 and old_shape[0] in self.shape and old_shape[
                1] in self.shape and old_shape[2] in self.shape):
                print(
                    'Warning : possibly incorrect voxelsize, extent and origin')
                vox, ext, orig = [], [], []
                for ind in range(0, self.ndim):
                    tmp = old_shape.index(self.shape[ind])
                    vox.append(met_dict['voxelsize'][tmp])
                    ext.append(met_dict['extent'][tmp])
                    orig.append(met_dict['origin'][tmp])
                met_dict['voxelsize'], met_dict['extent'], met_dict[
                    'origin'] = vox, ext, orig
                self.voxelsize, self.extent, self.origin = vox, ext, orig
            else:
                print('Warning : incorrect voxelsize, extent and origin')
                vox, ext, orig = [], [], []
                met_dict['voxelsize'], met_dict['extent'], met_dict[
                    'origin'] = vox, ext, orig
                self.voxelsize, self.extent, self.origin = vox, ext, orig

        self.metadata = met_dict
        return self.metadata

    def get_min(self):
        """
        Get ``SpatialImage`` min value

        Returns
        ----------
        :returns: image_min (*val*) -- ``SpatialImage`` min

        Example
        -------
        >>> import numpy as np
        >>> from timagetk.components import SpatialImage
        >>> test_array = np.ones((5,5), dtype=np.uint8)
        >>> image = SpatialImage(input_array=test_array)
        >>> image_min = image.get_min()
        >>> print image_min
        1
        """
        try:
            return self._min
        except AttributeError:
            self._min = self.min()
            self.metadata.update({'min': self._min})
            return self._min

    def get_max(self):
        """
        Get ``SpatialImage`` max value

        Returns
        ----------
        :returns: image_max (*val*) -- ``SpatialImage`` max

        Example
        -------
        >>> import numpy as np
        >>> from timagetk.components import SpatialImage
        >>> test_array = np.ones((5,5), dtype=np.uint8)
        >>> image = SpatialImage(input_array=test_array)
        >>> image_max = image.get_max()
        >>> print image_max
        1
        """
        try:
            return self._max
        except AttributeError:
            self._max = self.max()
            self.metadata.update({'max': self._max})
            return self._max

    def get_mean(self):
        """
        Get ``SpatialImage`` mean

        Returns
        ----------
        :returns: image_mean (*val*) -- ``SpatialImage`` mean

        Example
        -------
        >>> import numpy as np
        >>> from timagetk.components import SpatialImage
        >>> test_array = np.ones((5,5), dtype=np.uint8)
        >>> image = SpatialImage(input_array=test_array)
        >>> image_mean = image.get_mean()
        >>> print image_mean
        1
        """
        try:
            return self._mean
        except AttributeError:
            self._mean = self.mean()
            self.metadata.update({'mean': self._mean})
            return self._mean

    def get_origin(self):
        """
        Get ``SpatialImage`` origin

        Returns
        ----------
        :returns: image_origin (*list*) -- ``SpatialImage`` origin

        Example
        -------
        >>> import numpy as np
        >>> from timagetk.components import SpatialImage
        >>> test_array = np.ones((5,5), dtype=np.uint8)
        >>> image = SpatialImage(input_array=test_array)
        >>> image_origin = image.get_origin()
        >>> print image_origin
        [0, 0]
        """
        return self.origin

    def get_pixel(self, indices):
        """
        Get ``SpatialImage`` pixel value

        Parameters
        ----------
        :param list indices: indices as list of integers

        Returns
        ----------
        :returns: pixel_value (*self.get_type()*) -- pixel value

        Example
        -------
        >>> import numpy as np
        >>> from timagetk.components import SpatialImage
        >>> test_array = np.ones((5,5), dtype=np.uint8)
        >>> image = SpatialImage(input_array=test_array)
        >>> indices = [1,1]
        >>> pixel_value = image.get_pixel(indices)
        >>> print pixel_value
        1
        """
        img_dim = self.get_dim()
        if isinstance(indices, list) and len(indices) == img_dim:
            img_shape = self.get_shape()
            if img_dim == 2:
                range_x, range_y = xrange(img_shape[0]), xrange(img_shape[1])
                conds_ind = indices[0] in range_x and indices[1] in range_y
            elif img_dim == 3:
                range_x, range_y, range_z = xrange(img_shape[0]), xrange(
                    img_shape[1]), xrange(img_shape[2])
                conds_ind = indices[0] in range_x and indices[1] in range_y and \
                            indices[2] in range_z
            if conds_ind:
                if img_dim == 2:
                    pix_val = self[indices[0], indices[1]]
                elif img_dim == 3:
                    pix_val = self[indices[0], indices[1], indices[2]]
            return pix_val
        else:
            print('Warning, incorrect specification')
            return

    def get_region(self, indices):
        """
        Extract a region using list of start & stop 'indices'.
        There should be two values per image dimension in 'indices'.
        If the image is 3D and in one dimension, the start and stop are differ
        by one (on layer of voxels), the image is transformed to 2D!

        Parameters
        ----------
        :param list indices: indices as list of integers

        Returns
        ----------
        :returns: out_sp_image (``SpatialImage``) -- output ``SpatialImage``

        Example
        -------
        >>> import numpy as np
        >>> from timagetk.components import SpatialImage
        >>> test_array = np.ones((5,5), dtype=np.uint8)
        >>> image = SpatialImage(input_array=test_array)
        >>> indices = [1,3,1,3]
        >>> out_sp_image = image.get_region(indices)
        >>> isinstance(out_sp_image, SpatialImage)
        True
        """
        # TODO: use slice instead of 'indices' list?
        try:
            assert isinstance(indices, list)
        except AssertionError:
            raise TypeError("Parameters 'indices' must be a list!")

        img_dim = self.get_dim()
        try:
            assert len(indices) == 2 * img_dim
        except AssertionError:
            raise TypeError(
                "Parameter 'indices' must have twice the number of dimension of the image!")

        sh = [(0, s) for s in self.get_shape()]
        conds_ind = all(
            [(indices[i] > sh[i]) & (indices[i + 1] < sh[i + 1] + 1) for i in
             range(0, 2 * img_dim, 2)])
        conds_val = all(
            [indices[i + 1] - indices[i] >= 1 for i in
             range(0, 2 * img_dim, 2)])

        if img_dim < 2 or img_dim > 3:
            raise ValueError(
                "SpatialImage can be 2D or 3D, check dimensionality!")

        if not conds_ind:
            err = "Given 'indices' are not within the image shape!"
            raise ValueError(err)
        elif not conds_val:
            err = "Given 'indices' are wrong in at least one direction!"
            raise ValueError(err)
        else:
            pass

        bbox = (slice(indices[i], indices[i + 1]) for i in
                range(0, 2 * img_dim, 2))
        tmp_arr, tmp_vox = self.get_array(), self.get_voxelsize()
        reg_val = tmp_arr[bbox]
        if img_dim == 3 & 1 in reg_val.shape:  # 3D --> 2D
            if reg_val.shape[0] == 1:
                reg_val = np.squeeze(reg_val, axis=(0,))
                tmp_vox = [tmp_vox[1], tmp_vox[2]]
            elif reg_val.shape[1] == 1:
                reg_val = np.squeeze(reg_val, axis=(1,))
                tmp_vox = [tmp_vox[0], tmp_vox[2]]
            elif reg_val.shape[2] == 1:
                reg_val = np.squeeze(reg_val, axis=(2,))
                tmp_vox = [tmp_vox[0], tmp_vox[1]]

        out_sp_img = SpatialImage(input_array=reg_val, voxelsize=tmp_vox)
        return out_sp_img

    def get_shape(self):
        """
        Get ``SpatialImage`` shape

        Returns
        ----------
        :returns: image_shape (*tuple*) -- ``SpatialImage`` shape

        Example
        -------
        >>> import numpy as np
        >>> from timagetk.components import SpatialImage
        >>> test_array = np.ones((5,5), dtype=np.uint8)
        >>> image = SpatialImage(input_array=test_array)
        >>> image_shape = image.get_shape()
        >>> print image_shape
        (5, 5)
        """
        return self.shape

    def get_type(self):
        """
        Get ``SpatialImage`` type

        Returns
        ----------
        :returns: image_type (*str*) -- ``SpatialImage`` type

        Example
        -------
        >>> import numpy as np
        >>> from timagetk.components import SpatialImage
        >>> test_array = np.ones((5,5), dtype=np.uint8)
        >>> image = SpatialImage(input_array=test_array)
        >>> image_type = image.get_type()
        >>> print image_type
        uint8
        """
        return str(self.dtype)

    def get_voxelsize(self):
        """
        Get ``SpatialImage`` voxelsize

        Returns
        ----------
        :returns: image_voxelsize (*list*) -- ``SpatialImage`` voxelsize

        Example
        -------
        >>> import numpy as np
        >>> from timagetk.components import SpatialImage
        >>> test_array = np.ones((5,5), dtype=np.uint8)
        >>> image = SpatialImage(input_array=test_array)
        >>> image_voxelsize = image.get_voxelsize()
        >>> print image_voxelsize
        [1.0, 1.0]
        """
        return self.voxelsize

    def set_extent(self, img_extent):
        """
        Set ``SpatialImage`` physical extent

        Parameters
        ----------
        :param list image_extent: ``SpatialImage`` physical extent.
                                Metadata are updated according to the new physical extent.

        Example
        -------
        >>> import numpy as np
        >>> from timagetk.components import SpatialImage
        >>> test_array = np.ones((5,5), dtype=np.uint8)
        >>> image = SpatialImage(input_array=test_array)
        >>> image_extent = [10.0, 10.0]
        >>> image.set_extent(image_extent)
        """
        dimensionality_test(self.get_dim(), img_extent)
        img_extent = tuple_array_to_list(img_extent)
        img_extent = around_list(img_extent)
        self.extent = img_extent
        vox = [img_extent[ind] / self.get_shape()[ind] for ind, val in
               enumerate(img_extent)]
        vox = around_list(vox)
        self.set_voxelsize(vox)
        meta_dict = self.get_metadata()
        meta_dict['extent'] = img_extent
        meta_dict['voxelsize'] = vox
        self.metadata = meta_dict
        return "Set extent to '{}'".format(self.extent)

    def set_metadata(self, img_metadata):
        """
        Set ``SpatialImage`` metadata

        Parameters
        ----------
        :param dict image_metadata: ``SpatialImage`` metadata

        Example
        -------
        >>> import numpy as np
        >>> from timagetk.components import SpatialImage
        >>> test_array = np.ones((5,5), dtype=np.uint8)
        >>> image = SpatialImage(input_array=test_array)
        >>> image_metadata = {'name':'img_test'}
        >>> image.set_metadata(image_metadata)
        """
        tmp_dict = self.get_metadata()
        if isinstance(img_metadata, dict) and isinstance(tmp_dict, dict):
            tmp_dict.update(img_metadata)
            self.metadata = tmp_dict
            self.origin = tmp_dict['origin']
            self.voxelsize = around_list(tmp_dict['voxelsize'])
            self.extent = around_list(tmp_dict['extent'])
            self.min = tmp_dict['min']
            self.max = tmp_dict['max']
            self.mean = tmp_dict['mean']

    def set_origin(self, img_origin):
        """
        Set ``SpatialImage`` origin

        Parameters
        ----------
        :param list image_origin: ``SpatialImage`` origin

        Example
        -------
        >>> import numpy as np
        >>> from timagetk.components import SpatialImage
        >>> test_array = np.ones((5,5), dtype=np.uint8)
        >>> image = SpatialImage(input_array=test_array)
        >>> image_origin = [2, 2]
        >>> image.set_origin(image_origin)
        """
        dimensionality_test(self.get_dim(), img_origin)
        img_origin = tuple_array_to_list(img_origin)
        self.origin = img_origin
        img_met = self.get_metadata()
        img_met['origin'] = self.origin
        self.metadata = img_met
        return "Set origin to '{}'".format(self.origin)

    def set_pixel(self, indices, val):
        """
        Set ``SpatialImage`` pixel value

        Parameters
        ----------
        :param list indices: indices as list of integers

        :param value: new value, type of ``SpatialImage`` array

        Example
        -------
        >>> import numpy as np
        >>> from timagetk.components import SpatialImage
        >>> test_array = np.ones((5,5), dtype=np.uint8)
        >>> image = SpatialImage(input_array=test_array)
        >>> indices = [1,1]
        >>> value = 2
        >>> image.set_pixel(indices, value)
        """
        img_dim = self.get_dim()
        if isinstance(indices, list) and len(indices) == img_dim:
            img_shape = self.get_shape()
            if img_dim == 2:
                range_x, range_y = xrange(img_shape[0]), xrange(img_shape[1])
                conds_ind = indices[0] in range_x and indices[1] in range_y
            elif img_dim == 3:
                range_x, range_y, range_z = xrange(img_shape[0]), xrange(
                    img_shape[1]), xrange(img_shape[2])
                conds_ind = indices[0] in range_x and indices[1] in range_y and \
                            indices[2] in range_z
            if conds_ind:
                if img_dim == 2:
                    self[indices[0], indices[1]] = val
                elif img_dim == 3:
                    self[indices[0], indices[1], indices[2]] = val
            return
        else:
            print('Warning, incorrect specification')
            return

    def set_region(self, indices, val):
        """
        Replace a region

        Parameters
        ----------
        :param list indices: indices as list of integers
        :param val: new values (*np.ndarray* or value)

        Returns
        ----------
        :returns: out_sp_image (``SpatialImage``) -- ``SpatialImage`` instance

        Example
        -------
        >>> import numpy as np
        >>> from timagetk.components import SpatialImage
        >>> test_array = np.ones((5,5), dtype=np.uint8)
        >>> image = SpatialImage(input_array=test_array)
        >>> indices = [1,3,1,3]
        >>> out_sp_image = image.set_region(indices, 3)
        """
        img_dim = self.get_dim()
        conds_type = isinstance(indices, list) and len(indices) == 2 * img_dim
        if conds_type:
            conds_type_2 = isinstance(val, np.ndarray)
            conds_type_3 = isinstance(val, int)
            tmp_arr, tmp_vox = self.get_array(), self.get_voxelsize()
            if conds_type_2:
                if img_dim == 2:
                    conds_shape = (((max(indices[0], indices[1]) - min(
                        indices[0], indices[1])) == val.shape[0])
                                   and ((max(indices[2], indices[3]) - min(
                                indices[2], indices[3])) == val.shape[1]))
                elif img_dim == 3:
                    conds_shape = (((max(indices[0], indices[1]) - min(
                        indices[0], indices[1])) == val.shape[0])
                                   and ((max(indices[2], indices[3]) - min(
                                indices[2], indices[3])) == val.shape[1])
                                   and ((max(indices[4], indices[5]) - min(
                                indices[4], indices[4])) == val.shape[2]))
                if conds_shape:
                    if img_dim == 2:
                        tmp_arr[
                        min(indices[0], indices[1]):max(indices[0], indices[1]),
                        min(indices[2], indices[3]):max(indices[2],
                                                        indices[3])] = val[:, :]
                    elif img_dim == 3:
                        tmp_arr[
                        min(indices[0], indices[1]):max(indices[0], indices[1]),
                        min(indices[2], indices[3]):max(indices[2], indices[3]),
                        min(indices[4], indices[5]):max(indices[4],
                                                        indices[5])] = val[:, :,
                                                                       :]
            elif conds_type_3:
                if img_dim == 2:
                    tmp_arr[
                    min(indices[0], indices[1]):max(indices[0], indices[1]),
                    min(indices[2], indices[3]):max(indices[2],
                                                    indices[3])] = val
                elif img_dim == 3:
                    tmp_arr[
                    min(indices[0], indices[1]):max(indices[0], indices[1]),
                    min(indices[2], indices[3]):max(indices[2], indices[3]),
                    min(indices[4], indices[5]):max(indices[4],
                                                    indices[5])] = val
        else:
            print('Warning, incorrect specification')
        out_sp_img = SpatialImage(input_array=tmp_arr, voxelsize=tmp_vox)
        return out_sp_img

    def set_type(self, val):
        """
        Set ``SpatialImage`` type

        Parameters
        ----------
        :param str image_type: image type (see numpy types).

        Returns
        ----------
        :returns: out_sp_image (``SpatialImage``) -- ``SpatialImage`` instance

        Example
        -------
        >>> import numpy as np
        >>> from timagetk.components import SpatialImage
        >>> test_array = np.ones((5,5), dtype=np.uint8)
        >>> image = SpatialImage(input_array=test_array)
        >>> image_type = np.uint16
        >>> out_sp_image = image.set_type(image_type)
        """
        if (val in DICT_TYPES.keys() or val in DICT_TYPES.values()):
            for key in DICT_TYPES:
                if (val == key or val == DICT_TYPES[key]):
                    new_type = DICT_TYPES[key]

            met_dict = self.get_metadata()
            self = self.astype(new_type)
            met_dict['type'] = str(self.dtype)
            self.metadata = met_dict
            return self

    def set_voxelsize(self, img_vxs):
        """
        Set ``SpatialImage`` voxelsize

        Parameters
        ----------
        :param list image_voxelsize: ``SpatialImage`` voxelsize

        Example
        -------
        >>> import numpy as np
        >>> from timagetk.components import SpatialImage
        >>> test_array = np.ones((5,5), dtype=np.uint8)
        >>> image = SpatialImage(input_array=test_array)
        >>> image_voxelsize = [0.5, 0.5]
        >>> image.set_voxelsize(image_voxelsize)
        """
        dimensionality_test(self.get_dim(), img_vxs)
        img_vxs = tuple_array_to_list(img_vxs)
        img_vxs = around_list(img_vxs)
        self.voxelsize = img_vxs
        ext = [img_vxs[ind] * self.shape[ind] for ind, v in
               enumerate(self.shape)]
        ext = around_list(ext)
        self.extent = ext
        img_met = self.get_metadata()
        img_met['voxelsize'] = self.voxelsize
        img_met['extent'] = self.extent
        self.metadata = img_met
        return "Set voxelsize to '{}'".format(self.voxelsize)

    def is2D(self):
        """
        Returns True if the SpatialImage is 2D, else False.
        """
        return self.get_dim() == 2

    def is3D(self):
        """
        Returns True if the SpatialImage is 3D, else False.
        """
        return self.get_dim() == 3

    def to_2D(self):
        """
        Convert, if possible, a 3D SpatiamImage with one "flat" dimension (ie.
        with only one slice in this dimension) to a 2D SpatialImage.

        Returns
        -------
        SpatialImage
            the 2D SpatialImage
        """
        if self.is3D() and 1 in self.get_shape():
            voxelsize, shape, array = self.get_voxelsize(), self.get_shape(), self.get_array()
            ori, md = self.get_origin(), self.get_metadata()
            if shape[0] == 1:
                new_arr = np.squeeze(array, axis=(0,))
                new_vox = [voxelsize[1], voxelsize[2]]
            elif shape[1] == 1:
                new_arr = np.squeeze(array, axis=(1,))
                new_vox = [voxelsize[0], voxelsize[2]]
            elif shape[2] == 1:
                new_arr = np.squeeze(array, axis=(2,))
                new_vox = [voxelsize[0], voxelsize[1]]
            out_sp_img = SpatialImage(input_array=new_arr, voxelsize=new_vox,
                                      origin=ori, metadata_dict=md)
            return out_sp_img
        else:
            print('3D SpatialImage can not be reshaped to 2D')
            return

    def to_3D(self):
        """
        Convert, a 2D SpatiamImage into a 3D SpatialImage with one "flat"
        dimension (ie. with only one slice in this dimension).

        Returns
        -------
        SpatialImage
            the 3D SpatialImage
        """
        if self.is2D():
            voxelsize, shape, array = self.get_voxelsize(), self.get_shape(), self.get_array()
            ori, md = self.get_origin(), self.get_metadata()
            new_arr = np.reshape(array, (shape[0], shape[1], 1))
            new_vox = [voxelsize[0], voxelsize[1], 1.0]
            out_sp_img = SpatialImage(input_array=new_arr, voxelsize=new_vox,
                                      origin=ori, metadata_dict=md)
            return out_sp_img
        else:
            print('SpatialImage is not 2D')
            return

    def get_available_types(self):
        """
        Print the availables bits type dictionary.
        """
        return DICT_TYPES

    def is_available_types(self, dtype):
        """
        Print the availables bits type dictionary.
        """
        return dtype in DICT_TYPES.keys()

    def to_8bits(self, unsigned=True):
        """
        Convert the array to a different bit depth.

        Parameters
        ----------
        unsigned : bool, optional
            if True (default), return as unsigned integer 'uint8', else signed
            integer 'int8'

        Returns
        -------
        img : SpatialImage
            the converted SpatialImage
        """
        vxs = self.get_voxelsize()
        ori = self.get_origin()
        md = self.get_metadata()
        if unsigned:
            dtype = 'uint8'
        else:
            dtype = 'int8'

        arr = self.get_array()
        if arr.dtype == "uint16" or arr.dtype == "int16":
            factor = 2**(16 - 8) - 1
        elif arr.dtype == "uint32" or arr.dtype == "int32":
            factor = 2**(32 - 8) - 1
        elif arr.dtype == "uint64" or arr.dtype == "int64":
            factor = 2**(64 - 8) - 1
        else:
            raise NotImplementedError('Could not find implementation to do just that!')

        arr = np.divide(arr, factor).astype(DICT_TYPES[dtype])
        return SpatialImage(arr, voxelsize=vxs, origin=ori, metadata_dict=md)

    def revert_axis(self, axis='z'):
        """
        Revert x, y, or z axis

        Parameters
        ----------
        :param str axis: can be either 'x', 'y' or 'z'

        Returns
        ----------
        :returns: out_sp_image (``SpatialImage``) -- ``SpatialImage`` instance

        Example
        -------
        >>> import numpy as np
        >>> from timagetk.components import SpatialImage
        >>> test_array = np.ones((5,5,5), dtype=np.uint8)
        >>> image = SpatialImage(input_array=test_array, voxelsize=[0.5, 0.5, 0.5])
        >>> image.revert_axis(axis='y')
        """
        if self.get_dim() == 2:
            self = self.to_3D()

        arr, vox = self.get_array(), self.get_voxelsize()
        if axis == 'x':
            new_arr = arr[::-1, :, :]
        if axis == 'y':
            new_arr = arr[:, ::-1, :]
        elif axis == 'z':
            new_arr = arr[:, :, ::-1]
        out_sp_image = SpatialImage(new_arr, voxelsize=vox)
        if 1 in out_sp_image.get_shape():
            out_sp_image = out_sp_image.to_2D()
        return out_sp_image
