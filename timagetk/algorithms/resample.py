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

# --- Aug. 2016
import numpy as np

try:
    from timagetk.components import SpatialImage
    from timagetk.algorithms import apply_trsf
except ImportError:
    raise ImportError('Import Error')

__all__ = ['resample_isotropic', 'subsample']


def resample(image, voxelsize, option='gray'):
    """
    Resample an image to the given voxelsize.
    Use 'option' to control type of interpolation applied to the image.

    Parameters
    ----------
    image : SpatialImage
        an image to resample
    voxelsize : list
        the voxelsize to which the image should be resampled
    option : str, optional
        'gray' (default) indicate the image will be interpolated with a 'linear'
        function, when 'label' use the 'nearest' function.

    Returns
    -------
    out_img : SpatialImage
        the resampled image
    """
    dim = image.get_dim()
    try:
        assert len(voxelsize) == dim
    except AssertionError:
        raise ValueError("Parameter 'voxelsize' length ({}) does not match the dimension of the image ({}).".format(len(voxelsize), dim))

    extent = image.get_extent()
    new_shape = [int(round(extent[ind] / voxelsize[ind])) for ind in range(dim)]
    # - Initialise a template image:
    tmp_img = np.zeros((new_shape[0], new_shape[1], new_shape[2]),
                       dtype=image.dtype)
    tmp_img = SpatialImage(tmp_img, voxelsize=voxelsize,
                           origin=image.get_origin(),
                           metadata_dict=image.get_metadata())

    if option == 'gray':
        param_str_2 = '-resize -interpolation linear'
    elif option == 'label':
        param_str_2 = '-resize -interpolation nearest'

    # - Performs resampling:
    out_img = apply_trsf(image, trsf=None, template_img=tmp_img,
                         param_str_2=param_str_2)

    if 1 in out_img.get_shape():
        out_img = out_img.to_2D()

    return out_img


def resample_isotropic(image, voxelsize, option='gray'):
    """
    Resample into an isotropic dataset

    Parameters
    ----------
    :param *SpatialImage* sp_img: *SpatialImage*, 3D input image

    :param float voxelsize: voxelsize value

    :param str option: option can be either 'gray' or 'label'

    Returns
    ----------
    :return: ``SpatialImage`` instance -- output image and metadata

    Example
    -------
    >>> output_image = resample_isotropic(input_image, voxelsize=0.4)
    """
    # Check the input parameters:
    poss_opt = ['gray', 'label']
    if option not in poss_opt:
        raise ValueError("Possible options are: {}".format(poss_opt))
    if not (isinstance(image, SpatialImage) and image.get_dim() == 3):
        raise TypeError("'image' must be a SpatialImage instance")

    if not isinstance(voxelsize, float):
        voxelsize = float(voxelsize)

    new_vox = [voxelsize, voxelsize, voxelsize]
    out_img = resample(image, new_vox, option)
    return out_img


# Similar to 'resample_isotropic':
# def resample_image(input_im, new_voxelsize=None, new_shape=None, resampling_factor=1.):
#     """
#     Allow to resample any image given a new shape, new voxelsize or resampling_factor.
#
#     If:
#      - resampling_factor < 1: over-sampling;
#      - resampling_factor > 1: down-sampling;
#      - resampling_factor = 1: unchanged.
#
#     Note: this is a work in progress!
#     """
#     print "Input image infos:"
#     # - Get input_im infos:
#     im_vxs = input_im.voxelsize
#     print "  -- voxelsize: {}".format(im_vxs)
#     im_dtype = input_im.dtype
#     print "  -- dtype: {}".format(im_dtype)
#     im_shape = input_im.shape
#     print "  -- shape: {}".format(im_shape)
#     # - `new_voxelsize` case:
#     if new_voxelsize is not None:
#         # - Check only one resampling method has been selected:
#         try:
#             assert (new_shape is None) and (resampling_factor==1)
#         except AssertionError:
#             raise AssertionError("You have defined to many parameters, please provide EITHER new_voxelsize, new_shape OR resampling_factor!")
#         else:
#             # -- Compute resampling factors for each direction:
#             interp_factor = np.divide(im_vxs, new_voxelsize)
#     # - `new_shape` case:
#     elif new_shape is not None :
#         # - Check only one resampling method has been selected:
#         raise NotImplementedError("This method is not implemented yet!")
#     # - `resampling_factor` case:
#     elif resampling_factor !=1 :
#         # - Check only one resampling method has been selected:
#         try:
#             assert (new_shape is None) and (resampling_factor==1)
#         except AssertionError:
#             raise AssertionError("You have defined to many parameters, please provide EITHER new_voxelsize, new_shape OR resampling_factor!")
#         else:
#             # -- Compute resampling factors for each direction:
#             interp_factor = np.repeat(resampling_factor, len(im_vxs))
#     else:
#         raise AssertionError("You need to define one resampling method by setting one of them!")
#
#     print "Resampling factor obtained: {}".format(interp_factor)
#     # -- Create a template image with isometric voselsize and the right shape:
#     template_im_shape = map(int, im_shape * interp_factor)
#     template_im_vxs = im_vxs / interp_factor
#     print "Isometric resampling to:"
#     print "  -- voxelsize {} (original: {})".format(template_im_vxs, input_im.voxelsize)
#     print "  -- shape {} (original: {})".format(template_im_shape, input_im.shape)
#     template_im = SpatialImage(np.zeros(template_im_shape),
#                                voxelsize=template_im_vxs.tolist(),
#                                dtype=im_dtype)
#     # -- Create the corresponding identity transformation:
#     identity_trsf = create_trsf(template_im, param_str_1='-identity')
#     # -- Apply it on `input_im` to interpolate:
#     interp_im = apply_trsf(input_im, trsf=identity_trsf,
#                            template_img=template_im, dtype=im_dtype)
#     return interp_im


def isometric_resampling(input_im, method='min', option='gray', dry_run=False):
    """
    Transform the image to an isometric version according to a method or a given voxelsize.

    Parameters
    ----------
    input_im : SpatialImage
        image to resample
    method : str|float, optional
        change voxelsize to 'min' (default), 'max' of original voxelsize or to a
        given value.
    option : str, optional
        option can be either 'gray' or 'label'
    dry_run : bool, optional
        if True (default False), do not performs the resampling but return the size and voxelsize

    """
    POSS_METHODS = ['min', 'max']
    if method not in POSS_METHODS and not isinstance(method, float):
        raise ValueError(
            "Possible values for 'methods' are a float, 'min' or 'max'.")
    if method == 'min':
        vxs = np.min(input_im.voxelsize)
    elif method == 'max':
        vxs = np.max(input_im.voxelsize)
    else:
        vxs = method

    if dry_run:
        vxs = np.repeat(vxs, len(input_im.voxelsize)).tolist()
        extent = input_im.get_extent()
        dim = input_im.get_dim()
        size = [int(round(extent[ind] / vxs[ind])) for ind in range(dim)]
        return size, vxs
    return resample_isotropic(input_im, vxs, option)


def subsample(image, factor=[2, 2, 1], option='gray'):
    """
    Subsample a *SpatialImage* (2D/3D, grayscale/label)

    Parameters
    ----------
    :param *SpatialImage* image: input *SpatialImage*

    :param list factor: int|float or xyz-list of subsampling values

    :param str option: option can be either 'gray' or 'label'

    Returns
    ----------
    :return: *SpatialImage* output image

    Example
    -------
    >>> output_image = subsample(input_image)
    """
    poss_opt = ['gray', 'label']
    if option not in poss_opt:
        option = 'gray'

    try:
        assert isinstance(image, SpatialImage)
    except:
        raise ValueError("Input image must be a SpatialImage")
    try:
        assert image.get_dim() in [2, 3]
    except:
        raise ValueError("Image dimensionality is different than 2D or 3D")

    n_dim = image.get_dim()
    if isinstance(factor, int) or isinstance(factor, float):
        factor = [factor] * n_dim

    if n_dim == 2:
        image = image.to_3D()
        factor.append(1)

    shape, extent = image.get_shape(), image.get_extent()
    # new_shape = [int(np.ceil(shape[ind] / factor[ind])) for ind in
    #              range(image.get_dim())]
    # Smaller approximation error with round than np.ceil ?!
    new_shape = [int(round(shape[ind] / factor[ind])) for ind in
                 range(image.get_dim())]
    new_vox = [extent[ind] / new_shape[ind] for ind in
               range(image.get_dim())]
    tmp_img = np.zeros((new_shape[0], new_shape[1], new_shape[2]),
                       dtype=image.dtype)
    tmp_img = SpatialImage(tmp_img, voxelsize=new_vox,
                           origin=image.get_origin(),
                           metadata_dict=image.get_metadata())

    if option == 'gray':
        param_str_2 = '-resize -interpolation linear'
    elif option == 'label':
        param_str_2 = '-resize -interpolation nearest'

    out_img = apply_trsf(image, trsf=None,
                         template_img=tmp_img, param_str_2=param_str_2)
    if 1 in out_img.get_shape():
        out_img = out_img.to_2D()

    return out_img
