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
import numpy as np
from math import log10
np_square, np_array, np_reshape, np_sum = np.square, np.array, np.reshape, np.sum
try:
    from timagetk.components import SpatialImage
except ImportError:
    raise ImportError('Import Error')

__all__ = ['mean_squared_error','psnr']


def mean_squared_error(sp_img_1, sp_img_2):
    """
    Mean squared error - measure of image quality

    Parameters
    ----------
    :param *SpatialImage* sp_img_1: *SpatialImage* image

    :param *SpatialImage* sp_img_2: *SpatialImage* image

    Returns
    ----------
    :return: float mse -- mean sqared error value
    """
    if isinstance(sp_img_1, SpatialImage) and isinstance(sp_img_2, SpatialImage):
        if sp_img_1.shape==sp_img_2.shape:
            img_1 = sp_img_1.get_array().astype(np.float16) #--- np.ndarray instance
            img_2 = sp_img_2.get_array().astype(np.float16) #--- np.ndarray instance
            tmp = np_square((img_1 - img_2)) #--- np.ndarray instance
            tmp = np_array(np_reshape(tmp, (-1, 1))).tolist()
            mse = np_sum(tmp)/len(tmp)
            return mse
        else:
            print('sp_img_1 and sp_img_2 does not have the same shape')
            return
    else:
        print('sp_img_1 and sp_img_2 must be SpatialImage instances')
        return


def psnr(sp_img_1, sp_img_2):
    """
    Peak Signal To Noise Ratio - measure of image quality

    Parameters
    ----------
    :param *SpatialImage* sp_img_1: *SpatialImage* image

    :param *SpatialImage* sp_img_2: *SpatialImage* image

    Returns
    ----------
    :return: float psnr -- psnr value (dB)
    """
    if isinstance(sp_img_1, SpatialImage) and isinstance(sp_img_2, SpatialImage):
        if sp_img_1.itemsize==sp_img_2.itemsize:
            maxi =  2**(sp_img_1.itemsize*8) - 1
            mse = mean_squared_error(sp_img_1, sp_img_2)
            if mse!=0:
                psnr = 20.0*log10(maxi) - 10*log10(mse)
            elif mse==0:
                psnr = np.inf
            return psnr
        else:
            print('sp_img_1 and sp_img_2 does not have the same type')
            return
    else:
        print('sp_img_1 and sp_img_2 must be SpatialImage instances')
        return