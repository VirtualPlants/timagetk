# -*- coding: utf-8 -*-
# -*- python -*-
#
#       Copyright 2016-2018 INRIA
#
#       File author(s):
#           Sophie Ribes <sophie.ribes@inria.fr>
#           Jerome Chopard <jerome.chopard@inria.fr>
#           G. Malandain
#
#       See accompanying file LICENSE.txt
# -----------------------------------------------------------------------------

"""
This module allows a management of inputs and outputs (2D/3D images and metadata)
through the functions ``imread`` and ``imsave``.
Supported formats are : ['.tif', '.tiff', '.inr', '.inr.gz', '.inr.zip'].
"""

from __future__ import division
import os
try:
    from timagetk.components import SpatialImage
except ImportError:
    raise ImportError('Unable to import SpatialImage')

__all__ = ["imread", "imsave"]

poss_ext = ['.inr', '.inr.gz', '.inr.zip', '.mha', '.mha.gz', '.tiff', '.tif']

def imread(img_file):
    """
    Read an image (2D/3D).
    The supported formats are : ['.tif', '.tiff', '.inr', '.inr.gz', '.inr.zip']

    Parameters
    ----------
    :param str img_file: path to the image

    Returns
    ----------
    :returns: sp_image (*SpatialImage*) -- image and metadata (such as voxelsize, extent, type, etc.)

    Example
    ---------
    >>> from timagetk.util import data_path
    >>> from timagetk.components import imread, SpatialImage
    >>> image_path = data_path('filtering_src.inr')
    >>> sp_image = imread(image_path)
    >>> isinstance(sp_image, SpatialImage)
    True
    """
    conds = os.path.exists(img_file)

    if conds:
        (filepath, filename) = os.path.split(img_file)
        (shortname, extension) = os.path.splitext(filename)
        if (extension == '.gz') or (extension == '.zip'):
            zip_ext = extension
            (shortname, extension) = os.path.splitext(shortname)
            extension += zip_ext
        if extension in poss_ext:
            if (extension=='.inr' or extension=='.inr.gz' or extension=='.inr.zip'):
                try:
                    from timagetk.components.inr_image import read_inr_image
                except ImportError:
                    raise ImportError('Unable to import .inr functionalities')
                sp_img = read_inr_image(img_file)
                return sp_img
            elif extension == '.mha' or extension == '.mha.gz':
                try:
                    from timagetk.components.mha_image import read_mha_image
                except ImportError:
                    raise ImportError('Unable to import .mha functionalities')
                sp_img = read_mha_image(img_file)
                return sp_img
            elif extension == '.tiff' or extension == '.tif':
                try:
                    from timagetk.components.tiff_image import read_tiff_image
                except ImportError:
                    raise ImportError('Unable to import .tiff functionalities')
                sp_img = read_tiff_image(img_file)
                return sp_img
        else:
            print('Unknown extension')
            print('Extensions can be either :'), _possible_extensions_
            return
    else:
        print('This file does not exist : '), img_file
        return


def imsave(img_file, sp_img):
    """
    Save an image (2D/3D).
    The supported formats are : ['.tif', '.tiff', '.inr', '.inr.gz', '.inr.zip']

    Parameters
    ----------
    :param str img_file: save path
    :param SpatialImage sp_img: *SpatialImage* instance

    Example
    ---------
    >>> from timagetk.util import data_path
    >>> from timagetk.components import imsave, SpatialImage
    >>> test_array = np.ones((5,5), dtype=np.uint8)
    >>> sp_image = SpatialImage(input_array=test_array)
    >>> save_path = data_path('test_output.tif')
    >>> imsave(save_path, sp_image)
    """
    conds = isinstance(sp_img, SpatialImage) and sp_img.get_dim() in [2, 3]
    if conds:
        (filepath, filename) = os.path.split(img_file)
        (shortname, extension) = os.path.splitext(filename)
        if (extension == '.gz') or (extension == '.zip'):
            zip_ext = extension
            (shortname, extension) = os.path.splitext(shortname)
            extension += zip_ext
        if extension in poss_ext:
            if extension == '.inr' or extension == '.inr.gz' or extension == '.inr.zip':
                try:
                    from timagetk.components.inr_image import write_inr_image
                except ImportError:
                    raise ImportError('Unable to import .inr functionalities')
                write_inr_image(img_file, sp_img)
            elif extension == '.mha' or extension == '.mha.gz':
                try:
                    from timagetk.components.mha_image import write_mha_image
                except ImportError:
                    raise ImportError('Unable to import .mha functionalities')
                write_mha_image(img_file, sp_img)
            elif extension == '.tiff' or extension == '.tif':
                try:
                    from timagetk.components.tiff_image import write_tiff_image
                except ImportError:
                    raise ImportError('Unable to import .tiff functionalities')
                write_tiff_image(img_file, sp_img)
    else:
        print('sp_img is not a SpatialImage')
    return
