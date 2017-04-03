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
#------------------------------------------------------------------------------

#--- Aug. 2016
import numpy as np
try:
    from timagetk.components import SpatialImage
    from timagetk.algorithms import apply_trsf
except ImportError:
    raise ImportError('Import Error')

__all__ = ['resample_isotropic','subsample']


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
    if isinstance(image, SpatialImage) and image.get_dim()==3:

        poss_opt = ['gray', 'label']
        if option not in poss_opt:
            option = 'gray'

        extent = image.get_extent()
        new_vox = [voxelsize, voxelsize, voxelsize]
        new_shape = [int(np.ceil(extent[ind]/new_vox[ind])) for ind in range(image.get_dim())]
        tmp_img = np.zeros((new_shape[0],new_shape[1],new_shape[2]),dtype=image.dtype)
        tmp_img = SpatialImage(tmp_img, voxelsize=new_vox)

        if option=='gray':
            param_str_2 = '-resize -interpolation linear'
        elif option=='label':
            param_str_2 = '-resize -interpolation nearest'

        out_img = apply_trsf(image, bal_transformation=None, template_img=tmp_img, param_str_2=param_str_2)
        if 1 in out_img.get_shape():
            out_img = out_img.to_2D()
        return out_img
    else:
        print('sp_img must be a SpatialImage instance')
        return


def subsample(image, factor=[2, 2, 1], option='gray'):
    """
    Subsample a *SpatialImage* (2D/3D, grayscale/label)

    Parameters
    ----------
    :param *SpatialImage* image: input *SpatialImage*

    :param list factor: list of dimensions or *SpatialImage*

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


    if isinstance(image, SpatialImage) and image.get_dim() in [2, 3]:
        if image.get_dim()==2:
            image = image.to_3D()
            factor.append(1)

        shape, extent = image.get_shape(), image.get_extent()
        new_shape = [int(np.ceil(shape[ind]/factor[ind])) for ind in range(image.get_dim())]
        new_vox = [extent[ind]/new_shape[ind] for ind in range(image.get_dim())]
        tmp_img = np.zeros((new_shape[0],new_shape[1],new_shape[2]),dtype=image.dtype)
        tmp_img = SpatialImage(tmp_img, voxelsize=new_vox)
        if option=='gray':
            param_str_2 = '-resize -interpolation linear'
        elif option=='label':
            param_str_2 = '-resize -interpolation nearest'

        out_img = apply_trsf(image, bal_transformation=None, template_img=tmp_img, param_str_2=param_str_2)
        if 1 in out_img.get_shape():
            out_img = out_img.to_2D()
        return out_img
    else:
        raise TypeError('Input image must be a SpatialImage')
        return
