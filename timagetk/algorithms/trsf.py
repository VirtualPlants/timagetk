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

import gzip
import numpy as np
import cPickle as pickle

from ctypes import POINTER
try:
    from ctypes import pointer
    from timagetk.wrapping.clib import libblockmatching, add_doc
    from timagetk.wrapping.balImage import BAL_IMAGE
    from timagetk.wrapping.balTrsf import BAL_TRSF
    from timagetk.wrapping.bal_image import BalImage
    from timagetk.wrapping.bal_image import init_c_bal_image
    from timagetk.wrapping.bal_image import allocate_c_bal_image
    from timagetk.wrapping.bal_image import spatial_image_to_bal_image_fields
    from timagetk.wrapping.bal_matrix import BalMatrix
    from timagetk.wrapping.bal_matrix import init_c_bal_matrix
    from timagetk.wrapping.bal_matrix import allocate_c_bal_matrix
    from timagetk.wrapping.bal_trsf import BalTransformation
    from timagetk.components.spatial_image import SpatialImage
except ImportError as e:
    raise ImportError('Import Error: {}'.format(e))

__all__ = [
    'INV_TRSF_DEFAULT', 'inv_trsf',
    'APPLY_TRSF_DEFAULT', 'apply_trsf',
    'COMPOSE_TRSF_DEFAULT', 'compose_trsf',
    'CREATE_TRSF_DEFAULT', 'create_trsf',
    'mean_trsfs'
]

INV_TRSF_DEFAULT = ''
APPLY_TRSF_DEFAULT = ''
COMPOSE_TRSF_DEFAULT = ''
CREATE_TRSF_DEFAULT = '-identity'


def inv_trsf(trsf, template_img=None,
             param_str_1=INV_TRSF_DEFAULT, param_str_2=None):
    """
    Inversion of a *BalTransformation* transformation

    Parameters
    ----------
    :param *BalTransformation* trsf: *BalTransformation*, input transformation

    :param template_img: optional, default is None. template_img is used for output image geometry
                                                    and can be either SpatialImage or a list of dimensions

    :param str param_str_1: INV_TRSF_DEFAULT, default = ''

    :param str param_str_2: optional, optional parameters

    Returns
    ----------
    :return: output *BalTransformation* transformation

    Example
    -------
    >>> trsf_output = inv_trsf(trsf_input)
    """
    try:
        assert isinstance(trsf, BalTransformation)
    except AssertionError:
        raise TypeError('Input transformation must be a BalTransformation!')

    if template_img:
        if isinstance(template_img, SpatialImage):
            if template_img.get_dim() == 2:
                template_img = template_img.to_3D()
            x, y, z = template_img.get_shape()
            vx, vy, vz = template_img.get_voxelsize()
            val_vox = ' -template-voxel {} {} {}'.format(vx, vy, vz)
        elif isinstance(template_img, list):
            if len(template_img) == 3:
                x, y, z = template_img[0], template_img[1], template_img[2]
            elif len(template_img) == 2:
                x, y, z = template_img[0], template_img[1], 1
            val_vox = ''
        val_dim = ' -template-dim {} {} {}'.format(x, y, z)
        param_str_1 += val_dim + val_vox

    trsf_out = BalTransformation(trsf_type=trsf.trsf_type,
                                 trsf_unit=trsf.trsf_unit, c_bal_trsf=trsf)
    libblockmatching.API_invTrsf(trsf.c_ptr, trsf_out.c_ptr,
                                 param_str_1, param_str_2)
    return trsf_out


def apply_trsf(image, trsf=None, template_img=None,
               param_str_1=APPLY_TRSF_DEFAULT, param_str_2=None, dtype=None):
    """
    Apply a *BalTransformation* transformation to a *SpatialImage image
    To apply a transformation to a segmented SpatialImage, uses '-nearest' in
    param_str_2, default is '-linear'

    Parameters
    ----------
    :param *SpatialImage* image: input *SpatialImage*

    :param *BalTransformation* trsf: input *BalTransformation* transformation

    :param template_img: optional, default is None. template_img is used for output image geometry
                                                    and can be either SpatialImage or a list of dimensions.
                                                    If a list of dimension, voxelsize is defined by `image`.

    :param str param_str_1: APPLY_TRSF_DEFAULT

    :param str param_str_2: optional, optional parameters

    :param *np.dtype* dtype  optional, output image type. By default, the output type is equal to the input type.

    Returns
    ----------
    :return: *SpatialImage* output image

    Example
    -------
    >>> output_image = apply_trsf(input_image, input_trsf)
    """
    try:
        assert isinstance(image, SpatialImage)
    except AssertionError:
        raise TypeError('Input image must be a SpatialImage!')
    if trsf is not None:
        try:
            assert isinstance(trsf, BalTransformation)
        except AssertionError:
            raise TypeError('Input transformation must be a BalTransformation!')

    if image.get_dim() == 2:  # 2D management
        image = image.to_3D()

    if template_img is None:
        kwargs = spatial_image_to_bal_image_fields(image)
    else:
        if isinstance(template_img, SpatialImage):
            if template_img.get_dim() == 2:
                template_img = template_img.to_3D()
            x, y, z = template_img.get_shape()
            vx, vy, vz = template_img.get_voxelsize()
            val_vox = ' -template-voxel {} {} {}'.format(vx, vy, vz)
        elif isinstance(template_img, list):
            # create the template_img from the list of dimensions:
            if len(template_img) == 3:
                tmp_img = np.zeros(
                    (template_img[0], template_img[1], template_img[2]),
                    dtype=image.dtype)
            elif len(template_img) == 2:
                tmp_img = np.zeros((template_img[0], template_img[1]),
                                   dtype=image.dtype)
            template_img = SpatialImage(tmp_img, voxelsize=[1., 1., 1.],
                                        origin=[0., 0., 0.])
            if template_img.get_dim() == 2:
                template_img = template_img.to_3D()
            x, y, z = template_img.get_shape()
            # Get the voxelsize from the input image:
            vx, vy, vz = image.get_voxelsize()
            val_vox = ' -template-voxel {} {} {}'.format(vx, vy, vz)
        else:
            raise TypeError(
                "Input `template_img` must be a SpatialImage or a list instance!")
        val_dim = ' -template-dim {} {} {}'.format(x, y, z)
        param_str_1 += val_dim + val_vox
        kwargs = spatial_image_to_bal_image_fields(template_img)

    if dtype:
        kwargs['np_type'] = dtype

    c_img_res = BAL_IMAGE()
    init_c_bal_image(c_img_res, **kwargs)
    allocate_c_bal_image(c_img_res, np.ndarray(kwargs['shape'], kwargs['np_type']))
    bal_img_res = BalImage(c_bal_image=c_img_res)
    bal_image = BalImage(spatial_image=image)
    libblockmatching.API_applyTrsf(bal_image.c_ptr, bal_img_res.c_ptr,
                                   trsf.c_ptr if trsf else None,
                                   param_str_1, param_str_2)
    res = bal_img_res.to_spatial_image()
    if 1 in res.get_shape():
        res = res.to_2D()
    bal_image.free(), bal_img_res.free()
    return res


def compose_trsf(list_trsf, template_img=None,
                 param_str_1=COMPOSE_TRSF_DEFAULT, param_str_2=None):
    """
    Composition of *BalTransformation* transformations

    Parameters
    ----------
    :param list list_trsf: list of *BalTransformation* transformations

    :param *SpatialImage* template_img: optional, template_img is a *SpatialImage* specified for vectorfield composition

    :param str param_str_1: COMPOSE_TRSF_DEFAULT

    :param str param_str_2: optional, optional parameters

    Returns
    ----------
    :return: res_trsf, *BalTransformation* transformation

    Examples
    -------
    >>> res_trsf = compose_trsf([trsf_1, trsf_2, trsf_3])
    >>> res_trsf = compose_trsf([trsf_1, trsf_2, trsf_3], template_img=template_img)
    """
    # SR - 18/03
    trsf_types = [list_trsf[ind].trsf_type for ind, val in enumerate(list_trsf)]
    ref = list_trsf[trsf_types.index(max(trsf_types))]
    # if template_img is not None and 12 (vectorfield transformation)
    # not in trsf_types, template_img is ignored
    if 12 in trsf_types and template_img is None:
        print('Specify a template image for output image geometry')
        raise NotImplementedError
    elif 12 in trsf_types and template_img is not None:
        if template_img.get_dim() == 2:
            template_img = template_img.to_3D()
        x, y, z = template_img.get_shape()
        str_dims = ' -template-dim {} {} {}'.format(x, y, z)
        param_str_1 += str_dims
        trsf_out = BalTransformation(trsf_type=ref.trsf_type,
                                     trsf_unit=ref.trsf_unit,
                                     c_bal_trsf=ref.c_struct)
    else:
        trsf_out = BalTransformation(trsf_type=ref.trsf_type,
                                     trsf_unit=ref.trsf_unit)
    # end modif

    mat_out = trsf_out.mat
    init_c_bal_matrix(mat_out.c_struct, l=4, c=4)
    allocate_c_bal_matrix(mat_out.c_struct, np.zeros((4, 4), dtype=np.float64))
    ntrsf = len(list_trsf)
    lst = POINTER(BAL_TRSF) * ntrsf
    trsfs = lst(*[trsf.c_ptr for trsf in list_trsf])

    libblockmatching.API_composeTrsf(trsfs, ntrsf, trsf_out.c_ptr,
                                     param_str_1, param_str_2)
    return trsf_out


def create_trsf(template_img=None, param_str_1=CREATE_TRSF_DEFAULT,
                param_str_2=None, trsf_type=None, trsf_unit=None):
    """
    Creation of classical *BalTransformation* transformations such as identity,
    translation, rotation,sinusoidal, random, etc.

    Parameters
    ----------
    :param *SpatialImage* template_img: optional, template_img is a *SpatialImage*

    :param str param_str_1: CREATE_TRSF_DEFAULT, default is identity

    :param str param_str_2: optional, optional parameters

    :param *BalTransformationType* trsf_type: type of *BalTransformation* transformation (see enumTypeTransfo in bal_stddef)
                    Default trsf_type is BalTransformation.AFFINE_3D

    :param *BalTransformationUnit* trsf_unit: unit of *BalTransformation* transformation (see enumUnitTransfo in bal_stddef)
            Default trsf_unit is BalTransformation.REAL_UNIT

    Returns
    ----------
    :return: *BalTransformation* trsf_out, output transformation

    Example
    -------
    >>> identity_trsf = create_trsf()
    """
    if template_img is not None and isinstance(template_img, SpatialImage):
        if template_img.get_dim() == 2:
            template_img = template_img.to_3D()

        x, y, z = template_img.get_shape()
        vx, vy, vz = template_img.get_voxelsize()
        val_dim = ' -template-dim {} {} {}'.format(x, y, z)
        val_vox = ' -template-voxel {} {} {}'.format(vx, vy, vz)
        param_str_1 = ''.join([param_str_1, val_dim, val_vox])
        bal_image = BalImage(template_img)
    else:
        template_img = None
        bal_image = None
        # raise NotImplementedError

    if trsf_type is None:
        trsf_type = BalTransformation.AFFINE_3D
    if trsf_unit is None:
        trsf_unit = BalTransformation.REAL_UNIT

    trsf_out = BalTransformation(trsf_type=trsf_type, trsf_unit=trsf_unit)
    mat_out = trsf_out.mat
    init_c_bal_matrix(mat_out.c_struct, l=4, c=4)
    allocate_c_bal_matrix(mat_out.c_struct, np.zeros((4, 4), dtype=np.float64))

    if template_img is None:
        libblockmatching.API_createTrsf(trsf_out.c_ptr, None, param_str_1,
                                        param_str_2)
    else:
        libblockmatching.API_createTrsf(trsf_out.c_ptr, bal_image.c_ptr,
                                        param_str_1, param_str_2)
    return trsf_out


def mean_trsfs(list_trsf):
    """
    Mean trsfs (vectorfield) *BalTransformation* transformation

    Parameters
    ----------
    :param list list_trsf: list of *BalTransformation* transformations (vectorfield type)

    Returns
    ----------
    :return: *BalTransformation* trsf_out, output mean transformation

    Example
    -------
    >>> trsf_out = mean_trsf([trsf_1, trsf_2, trsf_3])
    """
    # TODO: why only limited to 'VectorField' ?
    try:
        assert isinstance(list_trsf, list)
    except AssertionError:
        raise TypeError("Input list_trsf must be a list instance!")
    try:
        assert len(list_trsf) >= 2
    except AssertionError:
        raise TypeError(
            "Input list_trsf must contains a minimum of 2 trsfs!")

    try:
        assert np.alltrue(
            [isinstance(trsf, BalTransformation) for trsf in list_trsf])
    except AssertionError:
        raise TypeError(
            "All input trsf must be BalTransformation instances.")

    trsf_types = [trsf.trsf_type == 12 for trsf in list_trsf]
    try:
        assert np.alltrue(trsf_types)
    except AssertionError:
        raise NotImplementedError(
            "All input trsf must be VectorField (12), got: {}.".format(
                trsf_types))

    sp_img_vx = [trsf.vx.to_spatial_image() for trsf in list_trsf]
    sum_sp_img_vx = np.zeros_like(sp_img_vx[0])
    for ind, sp_img in enumerate(sp_img_vx):
        sum_sp_img_vx = sum_sp_img_vx + sp_img
    mean_sp_img_vx = sum_sp_img_vx / len(sp_img_vx)
    mean_sp_img_vx = SpatialImage(mean_sp_img_vx, origin=[0, 0, 0],
                                  voxelsize=sp_img_vx[0].voxelsize)
    bal_image_vx = BalImage(mean_sp_img_vx)

    sp_img_vy = [trsf.vy.to_spatial_image() for trsf in list_trsf]
    sum_sp_img_vy = np.zeros_like(sp_img_vy[0])
    for ind, sp_img in enumerate(sp_img_vy):
        sum_sp_img_vy = sum_sp_img_vy + sp_img
    mean_sp_img_vy = sum_sp_img_vy / len(sp_img_vy)
    mean_sp_img_vy = SpatialImage(mean_sp_img_vy, origin=[0, 0, 0],
                                  voxelsize=sp_img_vy[0].voxelsize)
    bal_image_vy = BalImage(mean_sp_img_vy)

    sp_img_vz = [trsf.vz.to_spatial_image() for trsf in list_trsf]
    sum_sp_img_vz = np.zeros_like(sp_img_vz[0])
    for ind, sp_img in enumerate(sp_img_vz):
        sum_sp_img_vz = sum_sp_img_vz + sp_img
    mean_sp_img_vz = sum_sp_img_vz / len(sp_img_vz)
    mean_sp_img_vz = SpatialImage(mean_sp_img_vz, origin=[0, 0, 0],
                                  voxelsize=sp_img_vz[0].voxelsize)
    bal_image_vz = BalImage(mean_sp_img_vz)

    out_bal_trsf = BAL_TRSF()  # --- BALTRSF instance
    out_bal_trsf_ptr = pointer(out_bal_trsf)
    libblockmatching.BAL_AllocTransformation(out_bal_trsf_ptr,
                                             list_trsf[0].trsf_type,
                                             pointer(list_trsf[0].vx.c_struct))
    out_bal_trsf.transformation_unit = list_trsf[0].trsf_unit
    out_bal_trsf.mat = list_trsf[0].mat.c_struct  # --- BAL_MATRIX instance
    out_bal_trsf.vx = bal_image_vx.c_struct  # --- BALIMAGE instance
    out_bal_trsf.vy = bal_image_vy.c_struct  # --- BALIMAGE instance
    out_bal_trsf.vz = bal_image_vz.c_struct  # --- BALIMAGE instance
    trsf_out = BalTransformation(trsf_type=list_trsf[0].trsf_type,
                                 c_bal_trsf=out_bal_trsf)
    for ind, trsf in enumerate(list_trsf):
        trsf.free()
    return trsf_out

add_doc(inv_trsf, libblockmatching.API_Help_invTrsf)
add_doc(apply_trsf, libblockmatching.API_Help_applyTrsf)
add_doc(compose_trsf, libblockmatching.API_Help_composeTrsf)
add_doc(create_trsf, libblockmatching.API_Help_createTrsf)


def save_trsf(trsf, filename, compress=False):
    """
    Write a BalTransformation 'trsf' under given 'filename'.

    Parameters
    ----------
    trsf : BalTransformation
        BalTransformation object to save
    filename : str
        name of the file
    compress : bool, optional
        if True (default False) the saved transformation matrix is compressed
    """
    if trsf.is_vectorfield():
        if compress and not filename.endswith(".gz"):
            f = gzip.open(filename + ".gz", 'w')
        elif filename.endswith(".gz"):
            f = gzip.open(filename, 'w')
        else:
            f = open(filename, 'w')

        vx = trsf.vx.to_spatial_image()
        vy = trsf.vy.to_spatial_image()
        vz = trsf.vz.to_spatial_image()
        vectorfield = {'vx': vx, 'vy': vy, 'vz': vz,
                       'bal_image_fields': spatial_image_to_bal_image_fields(
                           vx)}
        pickle.dump(vectorfield, f)
        f.close()
    else:
        trsf.write(filename)

    return "Done saving transformation file '{}'".format(filename)


def read_trsf(filename):
    """
    Read a transformation file.

    Parameters
    ----------
    filename : str
        name of the file

    Returns
    -------
    tsrf : BalTransformation
        loaded BalTransformation object
    """
    c_bal_trsf = BAL_TRSF()
    trsf = BalTransformation()
    try:
        if filename.endswith(".gz"):
            f = gzip.open(filename, 'r')
        else:
            f = open(filename, 'r')
        obj = pickle.load(f)
        f.close()
        vx, vy, vz = obj['vx'], obj['vy'], obj['vz']
        metadata = obj['bal_image_fields']
        vx = SpatialImage(vx, voxelsize=metadata['resolution'],
                          origin=[0, 0, 0], dtype=metadata['np_type'])
        vy = SpatialImage(vy, voxelsize=metadata['resolution'],
                          origin=[0, 0, 0], dtype=metadata['np_type'])
        vz = SpatialImage(vz, voxelsize=metadata['resolution'],
                          origin=[0, 0, 0], dtype=metadata['np_type'])
    except:
        trsf.read(filename)
    else:
        trsf.mat = BalMatrix(c_bal_matrix=c_bal_trsf.mat)
        trsf.vx = BalImage(spatial_image=vx)
        trsf.vy = BalImage(spatial_image=vy)
        trsf.vz = BalImage(spatial_image=vz)
        trsf.trsf_unit = c_bal_trsf.transformation_unit
        trsf.trsf_type = c_bal_trsf.type

    return trsf
