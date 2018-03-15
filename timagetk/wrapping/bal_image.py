# -*- coding: utf-8 -*-
# -*- python -*-
#
#
#       Copyright 2016 INRIA
#
#       File author(s):
#           Guillaume Baty <guillaume.baty@inria.fr>
#           Sophie Ribes <sophie.ribes@inria.fr>
#
#       See accompanying file LICENSE.txt
#------------------------------------------------------------------------------

from ctypes import pointer, c_void_p
import numpy as np
try:
    from timagetk.wrapping.clib import c_stdout, libblockmatching
    from timagetk.wrapping.type_conversion import vt_type_to_c_type, vt_type, vt_type_to_np_type
    from timagetk.wrapping.balImage import BAL_IMAGE
    from timagetk.wrapping.bal_common import bal_mystr_c_ptr, bal_mystr_c_struct
    from timagetk.components.spatial_image import SpatialImage
except ImportError:
    raise ImportError('Import Error')

__all__ = ['bal_image_c_ptr',
           'bal_image_c_struct',
           'free_bal_image',
           'spatial_image_to_bal_image_fields',
           'init_c_bal_image',
           'allocate_c_bal_image',
           'new_c_bal_image',
           'spatial_image_to_bal_image',
           'bal_image_to_spatial_image',
           'BalImage'
           ]
DEFAULT_NAME = 'anonymous_bal_image'
DEFAULT_VT_TYPE = vt_type(np_type=np.uint8)


def bal_image_c_ptr(c_or_bal_image):
    """
    Return a pointer to instance of BAL_IMAGE or BalImage.

    Parameters
    ----------
    c_or_bal_image : instance of BAL_IMAGE or BalImage

    Returns
    ----------
    output : pointer

    Example
    -------
    c_ptr = bal_image_c_ptr(c_or_bal_image)
    """
    return bal_mystr_c_ptr(c_or_bal_image, BAL_IMAGE, BalImage)


def bal_image_c_struct(c_or_bal_image):
    """
    Return an instance of BAL_IMAGE or BalImage.

    Parameters
    ----------
    c_or_bal_image : instance of BAL_IMAGE or BalImage

    Returns
    ----------
    output : c_struct

    Example
    -------
    c_struct = bal_image_c_struct(c_or_bal_image)
    """
    return bal_mystr_c_struct(c_or_bal_image, BAL_IMAGE, BalImage)


def free_bal_image(c_or_bal_image):
    """
    Memory deallocation.

    Parameters
    ----------
    c_or_bal_image : instance of BAL_IMAGE or BalImage

    Example
    -------
    free_bal_image(c_or_bal_image)
    """
    c_struct = bal_image_c_struct(c_or_bal_image)
    c_ptr = pointer(c_struct)
    if c_ptr:
        # data memory is managed in python side, set pointer to NULL to avoid to free it twice
        # (double free or corruption)
        c_struct.data = None
        libblockmatching.BAL_FreeImage(c_ptr)  # free structure
        del c_or_bal_image


def spatial_image_to_bal_image_fields(spatial_image):
    """
    From spatial_image to BalImage fields.

    Parameters
    ----------
    spatial_image : spatial_image, see timagetk.components.spatial_image

    Returns
    ----------
    output : BalImage fields, type dict

    Example
    -------
    bal_image_kwds = spatial_image_to_bal_image_fields(spatial_image)
    """
    # Get data information from spatial image
    if spatial_image.ndim == 2:
        xdim, ydim, zdim = spatial_image.shape + (1,)
        spatial_image.shape = (xdim, ydim, zdim)
        _vx, _vy, _vz = spatial_image.voxelsize + (1.,)
        spatial_image.voxelsize = (_vx, _vy, _vz)

    _vx, _vy, _vz = spatial_image.voxelsize
    _xdim, _ydim, _zdim = spatial_image.shape
    _vdim = 1

    shape = _xdim, _ydim, _zdim, _vdim
    resolution = _vx, _vy, _vz

    bal_image_kwds = dict(
        shape=shape,
        resolution=resolution,
        vx=_vx, vy=_vy, vz=_vz,
        xdim=_xdim, ydim=_ydim, zdim=_zdim, vdim=_vdim,
        np_type=spatial_image.dtype)

    return bal_image_kwds


def init_c_bal_image(c_bal_image, **kwds):
    """
    Initialization of a c_bal_image.

    Parameters
    ----------
    c_bal_image : BAL_IMAGE
    kwds:
        - vy, vy, vz: voxelsize
        - np_type OR vt_type: data type (np_type: np.dtype, np_type: vt_type)
        - xdim, ydim, zdim, vdim: image dim
        - shape: if defined, replace xdim, ydim, zdim, vdim
        - voxelsize: if defined, replace vx, vy, vz

    Example
    -------
    init_c_bal_image(c_bal_image, **kwds)
    """
    xdim, ydim, zdim, vdim = kwds.get('xdim', 1), kwds.get(
        'ydim', 1), kwds.get('zdim', 1), kwds.get('vdim', 1)

    valid_shape = [xdim, ydim, zdim, vdim]

    shape = kwds.get('shape', valid_shape)
    if len(shape) > 4:
        raise NotImplementedError
    valid_shape[:len(shape)] = shape

    xdim, ydim, zdim, vdim = valid_shape

    vx = kwds.get('vx', 1)
    vy = kwds.get('vy', 1)
    vz = kwds.get('vz', 1)
    vx, vy, vz = kwds.get('voxelsize', (vx, vy, vz))

    vt_type_kwds = dict(vt_type=kwds.get('vt_type'),
                        np_type=kwds.get('np_type'))
    vtype = vt_type(**vt_type_kwds)
    if not vtype:
        vtype = DEFAULT_VT_TYPE

    name = kwds.get('name', DEFAULT_NAME)

    libblockmatching.BAL_InitImage(
        pointer(c_bal_image), name, xdim, ydim, zdim, vdim, vtype)

    c_bal_image.vx, c_bal_image.vy, c_bal_image.vz = vx, vy, vz


def allocate_c_bal_image(c_bal_image, spatial_image, **kwds):
    """
    Memory allocation of a c_bal_image.

    Parameters
    ----------
    c_bal_image : c_bal_image
    spatial_image : spatial_image, see timagetk.components.spatial_image

    Example
    -------
    allocate_c_bal_image(c_bal_image, spatial_image)
    """
    # Data pointer cast to a particular c-types object
    _data = spatial_image.ctypes.data_as(c_void_p)
    # update data pointer: bal_image.data -> ctype numpy array
    c_bal_image.data = _data
    # Create ***array from data
    libblockmatching.BAL_AllocArrayImage(pointer(c_bal_image))
    # DO NOT FORGET TO FREE IT LATER


def new_c_bal_image(**kwds):
    """
    Initialization of a c_bal_image.

    Parameters
    ----------
    **kwds : contains all fields necessary to c_bal_image, type dict

    Example
    -------
    c_bal_image = new_c_bal_image(**kwds)
    """
    c_bal_image = BAL_IMAGE()
    init_c_bal_image(c_bal_image, **kwds)
    return c_bal_image


def spatial_image_to_bal_image(spatial_image, **kwds):
    """
    Create an instance of a BalImage from a spatial_image.

    Parameters
    ----------
    spatial_image : spatial_image, see timagetk.components.spatial_image
    kwds : contains all fields necessary to c_bal_image not found in spatial_image

    Returns
    ----------
    output : c_bal_image

    Example
    -------
    c_bal_image = spatial_image_to_bal_image(spatial_image, **kwds)
    """
    spatial_image_kwds = spatial_image_to_bal_image_fields(spatial_image)

    _kwds = {}
    _kwds.update(kwds)
    _kwds.update(spatial_image_kwds)

    c_bal_image = new_c_bal_image(**_kwds)
    allocate_c_bal_image(c_bal_image, spatial_image)

    return c_bal_image


def bal_image_to_spatial_image(c_or_bal_image, **kwds):
    """
    Create a spatial_image from a BalImage.

    Parameters
    ----------
    c_or_bal_image : c_or_bal_image
    kwds : contains all fields necessary to spatial_image not found in c_bal_image

    Returns
    ----------
    output : spatial_image, see timagetk.components.spatial_image

    Example
    -------
    spatial_image = bal_image_to_spatial_image(c_or_bal_image, **kwds)
    """
    b = bal_image_c_struct(c_or_bal_image)

    x, y, z, v = b.ncols, b.nrows, b.nplanes, b.vdim
    size = x * y * z * v
    resolution = (b.vx, b.vy, b.vz)

    _cdtype = vt_type_to_c_type(b.type)
    # SR 21/03
    _nptype = vt_type_to_np_type(b.type)

    if not b.data:
        _np_array = np.ndarray(size)
    else:
        _ct_array = (_cdtype * size).from_address(b.data)
        _np_array = np.ctypeslib.as_array(_ct_array)
    if v == 1:
        #--- SR 21/03
        #arr = np.array(_np_array.reshape(x, y, z, order="F"))
        arr = np.array(_np_array.reshape(x, y, z, order="F"), dtype=_nptype)
    else:
        #--- SR 21/03
        #arr = np.array(_np_array.reshape(x, y, z, v, order="F"))
        arr = np.array(_np_array.reshape(x, y, z, v, order="F"), dtype=_nptype)

    return SpatialImage(arr, voxelsize=resolution, origin=[0]*arr.ndim, dtype=arr.dtype)


class BalImage(object):

    def __init__(self, spatial_image=None, c_bal_image=None, **kwds):

        if spatial_image:
            self._spatial_image = spatial_image
            self._c_bal_image = spatial_image_to_bal_image(self._spatial_image, **kwds)
        elif c_bal_image:
            self._c_bal_image = c_bal_image
            self._spatial_image = self.to_spatial_image()
        else:
            self._c_bal_image = new_c_bal_image(**kwds)
            self._spatial_image = self.to_spatial_image()

    @property
    def c_ptr(self):
        return pointer(self._c_bal_image)

    @property
    def c_struct(self):
        return self._c_bal_image

    def to_spatial_image(self, **kwds):
        return bal_image_to_spatial_image(self._c_bal_image, **kwds)

    def free(self):
        if self._c_bal_image:
            free_bal_image(self._c_bal_image)
        self._c_bal_image = None

    def c_display(self, name=''):
        libblockmatching.BAL_PrintImage(c_stdout, self.c_ptr, name)

    def __del__(self):
        self.free()

    def __eq__(self, other):
        if not isinstance(other, self.__class__):
            return False
        sp_self = self.to_spatial_image()
        sp_other = other.to_spatial_image()

        elements_ok = np.array_equal(sp_self, sp_other)

        ix, iy, iz = sp_self.voxelsize
        nx, ny, nz = sp_other.voxelsize

        rx_ok = (round(abs(nx - ix), 7) == 0)
        ry_ok = (round(abs(ny - iy), 7) == 0)
        rz_ok = (round(abs(nz - iz), 7) == 0)

        dtype_ok = sp_self.dtype == sp_other.dtype

        return elements_ok and rx_ok and ry_ok and rz_ok and dtype_ok

    def __ne__(self, other):
        return not self.__eq__(other)
