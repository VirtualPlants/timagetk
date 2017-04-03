# imports
try:
    from timagetk.components import SpatialImage
    from timagetk.components import imread, imsave
    from timagetk.util import data_path
except ImportError:
    raise ImportError('Import Error')

out_path = './results/' # to save results
# input image path
img_path = data_path('time_0_cut.inr')
# .inr format to SpatialImage
sp_img = imread(img_path)
# sp_img is now a SpatialImage instance
if isinstance(sp_img, SpatialImage):
    
    # get the SpatialImage metadata, accessor
    metadata = sp_img.get_metadata() # dictionary
    # print the SpatialImage metadata
    print('Metadata :'), metadata
    # metadata modification
    metadata['filename'] = img_path
    # set the SpatialImage metadata, mutator
    sp_img.set_metadata(metadata)
    # print the SpatialImage metadata
    print('New metadata :'), sp_img.get_metadata()
    
    # get the SpatialImage voxelsize (list of floating numbers)
    vox = sp_img.get_voxelsize()
    # print the SpatialImage voxelsize
    print('Voxelsize :'), vox
    # voxelsize modification
    new_vox = [] # empty list
    for ind, val in enumerate(vox):
        new_vox.append(2.0*val)
    # set the SpatialImage voxelsize, mutator
    sp_img.set_voxelsize(new_vox)
    print('New voxelsize :'), sp_img.get_voxelsize()

    # output
    # filename
    res_name = 'example_input_output.tif'
    # SpatialImage to .tif format
    imsave(out_path+res_name, sp_img)
    # filename
    res_name = 'example_input_output.inr'
    # SpatialImage to .inr format
    imsave(out_path+res_name, sp_img)
