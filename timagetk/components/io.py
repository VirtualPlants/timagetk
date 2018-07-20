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
Supported formats are : ['.tif', '.tiff', '.mha', '.mha.gz', '.inr', '.inr.gz', '.inr.zip'].
"""

from __future__ import division

import os

import numpy as np
import scipy.ndimage as nd

try:
    from timagetk.components import SpatialImage
except ImportError:
    raise ImportError('Unable to import SpatialImage')

__all__ = ["imread", "imsave", "apply_mask"]

POSS_EXT = ['.inr', '.inr.gz', '.inr.zip', '.mha', '.mha.gz', '.tiff', '.tif']


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
    # Check file exists:
    try:
        os.path.exists(img_file)
    except:
        raise IOError("This file does not exists: {}".format(img_file))

    # Get file path, name, root_name and extension:
    (filepath, filename) = os.path.split(img_file)
    (shortname, ext) = os.path.splitext(filename)
    # Check for possible compression of the file:
    if (ext == '.gz') or (ext == '.zip'):
        zip_ext = ext
        (shortname, ext) = os.path.splitext(shortname)
        ext += zip_ext
    try:
        assert ext in POSS_EXT
    except AssertionError:
        raise NotImplementedError(
            "Unknown file ext '{}', should be in {}.".format(ext, POSS_EXT))

    if (ext == '.inr' or ext == '.inr.gz' or ext == '.inr.zip'):
        try:
            from timagetk.components.inr_image import read_inr_image
        except ImportError:
            raise ImportError('Unable to import .inr functionalities')
        sp_img = read_inr_image(img_file)
    elif (ext == '.tiff' or ext == '.tif'):
        try:
            from timagetk.components.tiff_image import read_tiff_image
        except ImportError:
            raise ImportError('Unable to import .tiff functionalities')
        sp_img = read_tiff_image(img_file)
    elif (ext == '.mha' or ext == '.mha.gz'):
        try:
            from timagetk.components.mha_image import read_mha_image
        except ImportError:
            raise ImportError('Unable to import .mha functionalities')
        sp_img = read_mha_image(img_file)
    else:
        pass

    try:
        md_filename = sp_img.metadata['filename']
    except KeyError:
        sp_img.metadata['filepath'] = filepath
        sp_img.metadata['filename'] = filename
    else:
        (md_filepath, md_filename) = os.path.split(md_filename)
        # -- Check the given filepath and the one found in metadata (if any)
        if md_filepath == '':
            md_filepath = filepath
        elif md_filepath != filepath:
            print "WARNING: file path from 'filename' metadata differ from given path to reader.",
            print "Updated!"
            md_filepath = filepath
        else:
            pass
        sp_img.metadata['filepath'] = md_filepath
        # -- Check the given filename and the one found in metadata (if any)
        if md_filename == '':
            md_filename = filename
        elif md_filename != filename:
            print "WARNING: file name from 'filename' metadata differ from given name to reader.",
            print "Updated!"
        else:
            pass
        sp_img.metadata['filename'] = md_filename

    return sp_img


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
    (filepath, filename) = os.path.split(img_file)
    (shortname, ext) = os.path.splitext(filename)
    # Check for possible compression of the file:
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

    if (ext == '.inr' or ext == '.inr.gz' or ext == '.inr.zip'):
        try:
            from timagetk.components.inr_image import write_inr_image
        except ImportError:
            raise ImportError('Unable to import .inr functionalities')
        write_inr_image(img_file, sp_img)
    elif (ext == '.tiff' or ext == '.tif'):
        try:
            from timagetk.components.tiff_image import write_tiff_image
        except ImportError:
            raise ImportError('Unable to import .tiff functionalities')
        write_tiff_image(img_file, sp_img)
    elif extension == '.mha' or extension == '.mha.gz':
        try:
            from timagetk.components.mha_image import write_mha_image
        except ImportError:
            raise ImportError('Unable to import .mha functionalities')
        write_mha_image(img_file, sp_img)
    else:
        pass

    return 


def apply_mask(img, mask_filename, masking_value=0, crop_mask=False):
    """
    Load and apply a z-projection mask (2D) to a SpatialImage (2D/3D).
    In case of an intensity image, allows to remove unwanted signal intensities.
    If the SpatialImage is 3D, it is applied in z-direction.
    The mask should contain a distinct "masking value".

    Parameters
    ----------
    img : SpatialImage
        SpatialImage to modify with the mask
    mask_filename : str
        string giving the location of the mask file
    masking_value : int, optional
        value (default 0) defining the masked region
    crop_mask : bool, optional
        if True (default False), the returned SpatialImage is cropped around the
        non-masked region

    Returns
    -------
    masked_image : SpatialImage
        the masked SpatialImage.
    """
    try:
        from pillow import Image
    except ImportError:
        from PIL import Image

    xsh, ysh, zsh = img.get_shape()
    # Read the mask file:
    mask_im = Image.open(mask_filename)
    # Detect non-masked values positions
    mask_im = np.array(mask_im) != masking_value
    # Transform mask to 3D by repeating the 2D-mask along the z-axis:
    mask_im = np.repeat(mask_im[:, :, np.newaxis], zsh, axis=2)
    # Remove masked values from `img`:
    masked_im = img * mask_im
    # Crop the mask if required:
    if crop_mask:
        bbox = nd.find_objects(mask_im)
        masked_im = masked_im[bbox]

    return masked_im
