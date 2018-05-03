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
Management of the .tiff format
"""

from __future__ import division  #--- SR:do not delete!
import os
import numpy as np

try:
    from timagetk.components import SpatialImage
except ImportError:
    raise ImportError('Unable to import SpatialImage')

try:
    from timagetk.components.tifffile import TiffFile
except ImportError:
    raise ImportError('TiffFile import Error')

try:
    from timagetk.components.tifffile import TiffWriter
except ImportError:
    raise ImportError('TiffWriter import Error')


__all__ = ["read_tiff_image", "write_tiff_image"]

POSS_EXT = ['.tif', '.tiff']


def read_tiff_image(tiff_file):
    """
    Read an '.tif' or '.tiff' file (2D or 3D grayscale images)
    --- RGB images are not supported.

    Parameters
    ----------
    tiff_file : type string, path to the file.
        Supported extensions : '.tif', '.tiff'

    Returns
    ----------
    out_sp_img : type SpatialImage, image and associated information

    Example
    -------
    tiff_file = '/home/you/Documents/my_tiff_file.tiff'
    out_sp_img = read_tiff_image(tiff_file)
    """
    try:
        os.path.exists(tiff_file)
    except:
        raise IOError("This file does not exists: {}".format(tiff_file))

    (filepath, filename) = os.path.split(tiff_file)
    (shortname, ext) = os.path.splitext(filename)
    try:
        assert ext in POSS_EXT
    except AssertionError:
        raise NotImplementedError(
            "Unknown file ext '{}', should be in {}.".format(ext,
                                                                   POSS_EXT))

    with TiffFile(tiff_file) as tif:
        vox, metadata_dict = [], {}
        if len(tif.asarray().shape)==2:
            tmp_arr = tif.asarray()
        elif len(tif.asarray().shape)==3:
            tmp_arr = np.transpose(tif.asarray(), (2,1,0))
        elif len(tif.asarray().shape)>3 and 1 in tif.asarray().shape:
            tmp_arr = np.transpose(np.squeeze(tif.asarray()), (2,1,0))
        else:
            raise ValueError("Could not understand tiff_file array structure, only grayscale image are supported!")

        metadata_dict['shape'], metadata_dict['dim'] = tmp_arr.shape, len(tmp_arr.shape)
        int_tags = ['x_resolution', 'y_resolution', 'image_width', 'image_length', 'image_description', 'location']
        for page in tif:
            for tag in page.tags.values():
                if tag.name in int_tags:
                    if 'resolution' in tag.name:
                        if (isinstance(tag.value, tuple) and len(tag.value)==2):
                            res = float(tag.value[0]/tag.value[1])    # SR : OK (due to from __future__ import division)
                            metadata_dict['resolution'] = res
                        elif (isinstance(tag.value, float) and len(tag.value)==1):
                            #--- todo float management ?
                            pass
                    else:
                        metadata_dict[str(tag.name)] = tag.value

        if 'location' in metadata_dict:
            metadata_dict['filepath'], metadata_dict['filename'] = os.split(metadata_dict['location'])

        if ('resolution' in metadata_dict and 'image_width' in metadata_dict and 'image_length' in metadata_dict):
            metadata_dict['width'] = metadata_dict['image_width']/metadata_dict['resolution']
            metadata_dict['length'] = metadata_dict['image_length']/metadata_dict['resolution']
            vox_x = metadata_dict['width']/metadata_dict['image_width']
            vox_y = metadata_dict['length']/metadata_dict['image_length']
            vox.append(vox_x)
            vox.append(vox_y)

        if metadata_dict['dim']==3: #--- 3D images
            if 'image_description' in metadata_dict:
                description = metadata_dict['image_description']
                if isinstance(description, str) and len(description)>0:
                    description = description.replace('\n', ',')
                if description.endswith(','):
                    description = description[0:-1]
                out_dict = dict(x.split('=') for x in description.split(',') if len(x.split('='))==2)
                for key in out_dict:
                    if key == 'spacing':
                        metadata_dict['vox_z'] = float(out_dict[key])
                        vox.append(float(out_dict[key]))
                    elif key == 'slices':
                        metadata_dict['shape_z'] = int(out_dict[key])
                    else:
                        pass
                del out_dict
        tif.close()

        # Seems that 'spacing' is not defined when equal to 1.0...
        if ('shape_z' in metadata_dict) and not ('vox_z' in metadata_dict):
            vox.append(1.0)

        if (len(vox)==metadata_dict['dim']):
            out_sp_img = SpatialImage(input_array=tmp_arr, voxelsize=vox, metadata_dict=metadata_dict)
        else:
            out_sp_img = SpatialImage(input_array=tmp_arr, voxelsize=[], metadata_dict=metadata_dict)
            #~ print('Warning, unknown voxelsize')
        #~ del metadata_dict
        return out_sp_img


def write_tiff_image(tiff_file, sp_img):
    """
    Write a '.tif' or a '.tiff' file (2D or 3D grayscale images)
    --- RGB images are not supported.

    Parameters
    ----------
    tiff_file : type string, path to the file.
        Supported extensions : '.tif', '.tiff'

    sp_img : type SpatialImage

    Example
    -------
    tiff_file = '/home/you/Documents/my_tiff_file.tiff'
    test_arr = np.ones((5,5), dtype=np.uint8)
    sp_img = SpatialImage(input_array=test_arr)
    write_tiff_image(tiff_file, sp_img)
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
    (filepath, filename) = os.path.split(tiff_file)
    (shortname, ext) = os.path.splitext(filename)
    try:
        assert ext in POSS_EXT
    except AssertionError:
        raise NotImplementedError(
            "Unknown file ext '{}', should be in {}.".format(ext, POSS_EXT))

    if sp_img.get_dim()==2:
        data = np.transpose(sp_img.get_array())
    elif sp_img.get_dim()==3:
        data = np.transpose(sp_img.get_array(), (2, 1, 0))

    with TiffWriter(tiff_file, bigtiff=False, imagej=True) as tif:
        voxelsize = sp_img.get_voxelsize()
        if sp_img.get_dim()==2:
            tif.save(data, compress=0, resolution=(1.0/voxelsize[0],1.0/voxelsize[1]))
        elif sp_img.get_dim()==3:
            metadata_dict={'spacing': voxelsize[2], 'slices':sp_img.get_shape()[2]}
            tif.save(data, compress=0, resolution=(1.0/voxelsize[0],1.0/voxelsize[1]), metadata=metadata_dict)
        else:
            pass
    tif.close()
    return
