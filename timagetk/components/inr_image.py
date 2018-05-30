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
"""
Management of the .inr format
"""

import gzip
import os

import numpy as np

try:
    from cStringIO import StringIO  # C version
except:
    from StringIO import StringIO  # Python version
try:
    from timagetk.components import SpatialImage
except ImportError:
    raise ImportError('Unable to import SpatialImage')

__all__ = ["read_inr_image", "write_inr_image"]

POSS_EXT = ['.inr', '.inr.gz', '.inr.zip']


def read_inr_image(inr_file):
    """
    Read an '.inr' file (2D or 3D images)
    The supported formats are : ['.inr', '.inr.gz', '.inr.zip']

    Parameters
    ----------
    :param str inr_file: path to the image

    Returns
    ----------
    :returns: out_sp_img (*SpatialImage*) -- image and metadata (such as voxelsize, extent, type, etc.)

    Example
    -------
    >>> inr_file = '/home/X/Images/my_inr_file.inr'
    >>> out_sp_img = read_inr_image(inr_file)
    """
    try:
        os.path.exists(inr_file)
    except:
        raise IOError("This file does not exists: {}".format(inr_file))

    (filepath, filename) = os.path.split(inr_file)
    (shortname, ext) = os.path.splitext(filename)
    if (ext == '.gz') or (ext == '.zip'):
        zip_ext = ext
        (shortname, ext) = os.path.splitext(shortname)
        ext += zip_ext
    try:
        assert ext in POSS_EXT
    except AssertionError:
        raise NotImplementedError(
            "Unknown file ext '{}', should be in {}.".format(ext,
                                                                   POSS_EXT))

    if (ext == '.inr.gz' or ext == '.inr.zip'):
        with gzip.open(inr_file, 'rb') as fzip:
            f = StringIO(fzip.read())
            fzip.close()
    elif (ext == '.inr'):
        f = open(inr_file, 'rb')

    # --- header
    header = ""
    while (header[-4:] != "##}\n"):
        header += f.read(256)

    prop = {}
    head_start, head_end = header.find("{\n") + 1, header.find("##}")
    infos = [gr for gr in header[head_start:head_end].split("\n") if
             len(gr) > 0]

    for prop_def in infos:
        if not prop_def.strip().startswith('#'):
            key, val = prop_def.split("=")
            prop[key] = val

    shape_x, shape_y, shape_z = int(prop['XDIM']), int(prop['YDIM']), int(prop['ZDIM'])
    vox_x, vox_y, vox_z = float(prop['VX']), float(prop['VY']), float(prop['VZ'])
    vdim = int(prop['VDIM'])
    img_type, img_enc = prop['TYPE'], int(prop["PIXSIZE"].split()[0])

    poss_types, poss_enc = ['unsigned fixed', 'signed fixed', 'float'], [8, 16]
    conds = img_type in poss_types and img_enc in poss_enc
    if conds:
        if (img_type == 'unsigned fixed'):
            np_typ = eval("np.dtype(np.uint%d)" % img_enc)
        elif (img_type == 'signed fixed'):
            np_typ = eval("np.dtype(np.int%d)" % img_enc)
        elif (img_type == 'float'):
            np_typ = np.dtype(np.float16)
    else:
        if (img_type in poss_types and img_enc not in poss_enc):
            print('Warning, undetected encoding and possibly incorrect reading')
            if (img_type == 'unsigned fixed'):
                np_typ = np.dtype(np.uint)
            elif (img_type == 'signed fixed'):
                np_typ = np.dtype(np.int)
            elif (img_type == 'float'):
                np_typ = np.dtype(np.float)
        else:
            print('Unable to read this file...')
            return

    size = np_typ.itemsize * shape_x * shape_y * shape_z * vdim
    mat = np.fromstring(f.read(size), np_typ)
    if (vdim == 1):
        mat = mat.reshape((shape_x, shape_y, shape_z), order="F")
        img_vox = [vox_x, vox_y, vox_z]
        if 1 in mat.shape:  # --- 2D images management
            if mat.shape[0] == 1:
                mat = np.squeeze(mat, axis=(0,))
                img_vox = [vox_y, vox_z]
            elif mat.shape[1] == 1:
                mat = np.squeeze(mat, axis=(1,))
                img_vox = [vox_x, vox_z]
            elif mat.shape[2] == 1:
                mat = np.squeeze(mat, axis=(2,))
                img_vox = [vox_x, vox_y]
#            elif (vdim!=1):
#                mat = mat.reshape((vdim,shape_x,shape_y,shape_z), order="F" )
#                mat = mat.transpose(1,2,3,0)

    out_sp_img = SpatialImage(input_array=mat, voxelsize=img_vox,
                              origin=[0, 0, 0], metadata_dict=prop)
    f.close()
    prop.clear()
    return out_sp_img


def write_inr_image(inr_file, sp_img):
    """
    Write an '.inr' file (2D or 3D images).
    The supported formats are : ['.inr', '.inr.gz', '.inr.zip']

    Parameters
    ----------
    :param str inr_file : path to the file.
    :param SpatialImage sp_img: *SpatialImage* instance

    Example
    -------
    >>> inr_file = '/home/you/Documents/my_inr_file.inr'
    >>> test_arr = np.ones((5,5), dtype=np.uint8)
    >>> sp_img = SpatialImage(input_array=test_arr)
    >>> write_inr_image(inr_file, sp_img)
    """
    # - Assert sp_img is a SpatialImage instance:
    try:
        assert isinstance(sp_img, SpatialImage)
    except AssertionError:
        raise TypeError("Parameter 'sp_img' is not a SpatialImage!")
    # - Assert SpatialImage is 2D or 3D:
    try:
        assert sp_img.get_dim() in [2, 3]
    except AssertionError:
        raise ValueError("Parameter 'sp_img' should have a dimensionality of 2 or 3, not '{}'.".format(sp_img.get_dim()))
    # - Get file extension and check its validity:
    (filepath, filename) = os.path.split(inr_file)
    (shortname, ext) = os.path.splitext(filename)
    if (ext == '.gz') or (ext == '.zip'):
        zip_ext = ext
        (shortname, ext) = os.path.splitext(shortname)
        ext += zip_ext
    try:
        assert ext in POSS_EXT
    except AssertionError:
        raise NotImplementedError(
            "Unknown file ext '{}', should be in {}.".format(
                ext, POSS_EXT))

    if (ext == '.inr.gz' or ext == '.inr.zip'):
        f = gzip.GzipFile(inr_file, 'wb')
    elif (ext == '.inr'):
        f = open(inr_file, 'wb')

    metadata = sp_img.get_metadata()
    info = {}
    info['XDIM'], info['YDIM'] = metadata['shape'][0], \
                                 metadata['shape'][1]
    info['VX'], info['VY'] = metadata['voxelsize'][0], \
                             metadata['voxelsize'][1]

    if (sp_img.get_dim() == 2):
        info['ZDIM'], info['VZ'] = 1, 1.0
    elif (sp_img.get_dim() == 3):
        info['ZDIM'], info['VZ'] = metadata['shape'][2], \
                                   metadata['voxelsize'][2]
    info['#GEOMETRY'] = 'CARTESIAN'
    info['CPU'] = 'decm'
    info['VDIM'] = '1'
    img_typ = str(sp_img.get_type())

    if (img_typ[0:4] == 'uint'):
        info['TYPE'] = 'unsigned fixed'
    elif (img_typ[0:5] == 'float'):
        info['TYPE'] = 'float'
    if '8' in img_typ:
        info['PIXSIZE'] = '8 bits'
    elif '16' in img_typ:
        info['PIXSIZE'] = '16 bits'
    elif '32' in img_typ:
        info['PIXSIZE'] = '32 bits'
    elif '64' in img_typ:
        info['PIXSIZE'] = '64 bits'

    # --- header
    head_keys = ['XDIM', 'YDIM', 'ZDIM', 'VDIM', 'TYPE', 'PIXSIZE',
                 'SCALE', 'CPU', 'VX', 'VY', 'VZ', 'TX', 'TY', 'TZ',
                 '#GEOMETRY']
    header = "#INRIMAGE-4#{\n"
    for key in head_keys:
        try:
            header += "%s=%s\n" % (key, info[key])
        except KeyError:
            pass

    for k in set(info) - set(head_keys):
        header += "%s=%s\n" % (k, info[k])

    header_size = len(header) + 4
    if (header_size % 256) > 0:
        header += "\n" * (256 - header_size % 256)
    header += "##}\n"

    f.write(header)
    f.write(sp_img.get_array().tostring("F"))
#            elif (sp_img._get_dim() == 4):
#                mat = img.transpose(3,0,1,2)
#                stream.write(mat.tostring("F") )
#            else:
#                raise Exception("Unhandled image dimension %d."%img.ndim)
