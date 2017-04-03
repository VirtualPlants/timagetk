# imports
import os
try:
    from timagetk.util import data_path
    from timagetk.components import imread, imsave
    from timagetk.plugins import sequence_registration
except ImportError:
    raise ImportError('Import Error')

out_path = './results/' # to save results
if not os.path.isdir(out_path):
    new_fold = os.path.join(os.getcwd(),'results')
    os.mkdir(new_fold)

# we consider three different times
# time_0 is the floating image
# time_1 is first the local reference image,
# and becames the floating image
# time_2 is the global reference image
times = [0, 1, 2]
# list of SpatialImage instances
list_images = [imread(data_path('time_' + str(time) + '.inr'))
               for time in times]

# Rigid registration along the whole sequence:
# list_compo_trsf : list of transformations
# list_res_img : list of resulting images
# the primitive embeds an iterative registration
list_compo_trsf, list_res_img = sequence_registration(list_images,
                          method='sequence_rigid_registration')
for ind, img in enumerate(list_res_img):
    # filenames
    res_name = ''.join(['example_seq_reg_rigid_', str(ind), '_',
                        str(times[-1]), '.inr'])
    # SpatialImage to .inr format
    imsave(out_path+res_name, img)

# Affine registration alog the whole sequence:
list_compo_trsf, list_res_img = sequence_registration(list_images,
                          method='sequence_affine_registration')
for ind, img in enumerate(list_res_img):
    # filenames
    res_name = ''.join(['example_seq_reg_affine_', str(ind), '_',
                        str(times[-1]), '.inr'])
    # SpatialImage to .inr format
    imsave(out_path+res_name, img)
