# -*- python -*-
#
#       image.serial: read/write spatial nd images
#
#       Copyright 2006 - 2018 INRIA - CIRAD - INRA
#
#       File author(s): Jerome Chopard <jerome.chopard@sophia.inria.fr>
#                       Eric Moscardi <eric.moscardi@sophia.inria.fr>
#                       Daniel Barbeau <daniel.barbeau@sophia.inria.fr>
#                       Gregoire Malandain
#                       Jonathan Legrand
#
#       Distributed under the Cecill-C License.
#       See accompanying file LICENSE.txt or copy at
#           http://www.cecill.info/licences/Licence_CeCILL-C_V1-en.html
#
#       Repository: git@gitlab.inria.fr:mosaic/timagetk.git
################################################################################
"""
This module implementes partially the I/O for the metaimage format
(see https://itk.org/Wiki/ITK/MetaIO/Documentation)
It deals with images defined by a single file
"""

__license__ = "Cecill-C"
__revision__ = " $Id$ "

import os
from os import path
import sys
import numpy as np

import gzip

try:
    from cStringIO import StringIO  # C version
except:
    from StringIO import StringIO  # Python version

try:
    from timagetk.components import SpatialImage
except ImportError:
    raise ImportError('Unable to import SpatialImage')

__all__ = ["read_mha_image", "write_mha_image"]

#
#
#
#
#


_specific_header_keys_ = ("ObjectType",
                          "NDims", "DimSize", "ElementNumberOfChannels",
                          "ElementSize", "ElementSpacing", "ElementType",
                          "CompressedData", "BinaryDataByteOrderMSB", "BinaryData",
                          "ElementDataFile")


#
#
#
#
#


def _open_mha_imagefile(filename):
    """

    :param filename:
    :return:
    """
    program = "open_mha_imagefile"
    if not os.path.isfile(filename) and os.path.isfile(filename + ".gz"):
        filename = filename + ".gz"
        print "%s: Warning: path to read image has been changed to %s." % (program, filename)
    if not os.path.isfile(filename) and os.path.isfile(filename + ".zip"):
        filename = filename + ".zip"
        print "%s: Warning: path to read image has been changed to %s." % (program, filename)
    if path.splitext(filename)[1] in (".gz", ".zip"):
        fzip = gzip.open(filename, 'rb')
        f = StringIO(fzip.read())
        fzip.close()
    else:
        f = open(filename, 'rb')

    return f


#
#
#
#
#


def _read_header(f):
    """

    :param f:
    :return:
    """

    prop = {}
    while True:
        key, val = f.readline().rstrip('\n\r').split(" = ")
        if key == 'ElementDataFile':
            if val == 'LOCAL':
                break
            else:
                msg = "unable to read that type of data: '" + str(key) + ' = ' + str(val) + "'"
                raise UserWarning(msg)
        else:
            prop[key] = val

    return prop


#
#
#
#
#


def read_mha_image(filename):
    """

    :param filename:
    :return:
    """
    f = _open_mha_imagefile(filename)

    # read header
    prop = _read_header(f)

    #
    # Jonathan: 14.05.2012
    #
    prop["filename"] = filename

    #
    # find dimensions
    #
    dim = prop.pop("DimSize").split(' ')
    if len(dim) == 2:
        xdim = int(dim[0])
        ydim = int(dim[1])
        zdim = 1
    elif len(dim) == 3:
        xdim = int(dim[0])
        ydim = int(dim[1])
        zdim = int(dim[2])
    else:
        msg = "unable to handle such dimensions: 'DimSize = " + str(len(dim)) + "'"
        raise UserWarning(msg)

    vdim = int(prop.pop("ElementNumberOfChannels", 1))

    #
    # find type
    #
    voxeltype = prop.pop("ElementType")
    if voxeltype == 'MET_UCHAR':
        ntyp = np.dtype(np.uint8)
    elif voxeltype == 'MET_USHORT':
        ntyp = np.dtype(np.uint16)
    elif voxeltype == 'MET_UINT':
        ntyp = np.dtype(np.uint32)
    elif voxeltype == 'MET_FLOAT':
        ntyp = np.dtype(np.float32)
    elif voxeltype == 'MET_DOUBLE':
        ntyp = np.dtype(np.float64)
    else:
        msg = "unable to handle such voxel type: 'ElementType = " + str(voxeltype) + "'"
        raise UserWarning(msg)

    #
    # find resolution
    #
    resolution = prop.pop("ElementSize").split(' ')
    res = []
    for i in range(0, len(resolution)):
        res.append(float(resolution[i]))

    # read datas
    size = ntyp.itemsize * xdim * ydim * zdim * vdim
    mat = np.fromstring(f.read(size), ntyp)
    if vdim != 1:
        mat = mat.reshape((vdim, xdim, ydim, zdim), order="F")
        mat = mat.transpose(1, 2, 3, 0)
    else:
        mat = mat.reshape((xdim, ydim, zdim), order="F")
        # mat = mat.transpose(2,1,0)

    # create SpatialImage

    if vdim > 1:
        msg = "unable to handle multi-channel images: 'ElementNumberOfChannels = " + str(vdim) + "'"
        raise UserWarning(msg)
    # img = SpatialImage(mat, voxelsize=res, vdim, prop)
    img = SpatialImage(mat, voxelsize=res, metadata_dict=prop)

    # return
    f.close()
    return img


#
#
#
#
#


def _write_mha_image_to_stream(stream, img):
    assert img.ndim in (3, 4)

    # metadata
    info = dict(getattr(img, "info", {}))

    #
    #
    #
    info["ObjectType"] = "Image"

    #
    # image dimensions
    # won't support 2D vectorial images
    #
    if img.ndim == 2:
        info["NDims"] = "3"
        info["DimSize"] = str(img.shape[0]) + ' ' + str(img.shape[1]) + ' 1'
        info["ElementNumberOfChannels"] = "1"
    elif img.ndim == 3:
        info["NDims"] = "3"
        info["DimSize"] = str(img.shape[0]) + ' ' + str(img.shape[1]) + ' ' + str(img.shape[2])
        info["ElementNumberOfChannels"] = "1"
    elif img.ndim == 4:
        info["NDims"] = "3"
        info["DimSize"] = str(img.shape[0]) + ' ' + str(img.shape[1]) + ' ' + str(img.shape[2])
        info["ElementNumberOfChannels"] = str(img.shape[2])

    #
    # image resolutions
    #
    res = getattr(img, "resolution", (1, 1, 1))
    info["ElementSize"] = str(res[0]) + ' ' + str(res[1]) + ' ' + str(res[2])
    info["ElementSpacing"] = str(res[0]) + ' ' + str(res[1]) + ' ' + str(res[2])

    #
    # data type
    #
    if img.dtype == np.uint8:
        info["ElementType"] = "MET_UCHAR"
    elif img.dtype == np.uint16:
        info["ElementType"] = "MET_USHORT"
    elif img.dtype == np.uint32:
        info["ElementType"] = "MET_UINT"
    elif img.dtype == np.float32:
        info["ElementType"] = "MET_FLOAT"
    elif img.dtype == np.float64:
        info["ElementType"] = "MET_DOUBLE"
    # elif img.dtype == np.float128:
    #   info["TYPE"] = "float"
    #   info["PIXSIZE"] = "128 bits"
    else:
        msg = "unable to write that type of data: %s" % str(img.dtype)
        raise UserWarning(msg)

    #
    #
    #
    info["CompressedData"] = "False"
    info["BinaryDataByteOrderMSB"] = "False"
    info["BinaryData"] = "True"
    info["ElementDataFile"] = "LOCAL"

    #
    # fill header
    #
    header = ''
    for k in _specific_header_keys_:
        try:
            header += "%s = %s\n" % (k, info[k])
        except KeyError:
            pass

    #
    # write raw data
    #

    stream.write(header)
    if img.ndim == 2 or img.ndim == 3:
        stream.write(img.tostring("F"))

    # elif img.ndim == 4:
    #    mat = img.transpose(3,0,1,2)
    #    stream.write(mat.tostring("F") )

    else:
        raise Exception("Unhandled image dimension %d." % img.ndim)


#
#
#
#
#


def write_mha_image(filename, img):
    """Write an inrimage zipped or not according to the extension

    .. warning:: if img is not a |SpatialImage|, default values will be used
                 for the resolution of the image

   :Parameters:
     - `img` (|SpatialImage|) - image to write
     - `filename` (str) - name of the file to read
    """
    # open stream
    zipped = (path.splitext(filename)[1] in (".gz", ".zip"))

    if zipped:
        f = gzip.GzipFile(filename, "wb")
        # f = StringIO()
    else:
        f = open(filename, 'wb')

    try:
        _write_mha_image_to_stream(f, img)
    except:
        # -- remove probably corrupt file--
        f.close()
        if path.exists(filename) and path.isfile(filename):
            os.remove(filename)
        raise
    else:
        f.close()
