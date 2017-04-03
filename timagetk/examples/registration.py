# imports
import os
try:
    from timagetk.util import data_path
    from timagetk.components import imread, imsave
    from timagetk.plugins import registration
    #from timagetk.wrapping.bal_trsf import BalTransformation
except ImportError:
    raise ImportError('Import Error')

out_path = './results/' # to save results
if not os.path.isdir(out_path):
    new_fold = os.path.join(os.getcwd(),'results')
    os.mkdir(new_fold)

# we consider two different times
# time_1 is the floating image
# time_2 is the reference image
times = [1, 2]
# list of SpatialImage instances
list_images = [imread(data_path('time_' + str(time) + '.inr'))
               for time in times]
floating_img, reference_img = list_images[0], list_images[1]

# Rigid registration:
trsf_rig, res_rig = registration(floating_img,
                                 reference_img,
                                 method='rigid_registration')
# display the spatial transformation (4x4 matrix):
trsf_rig.c_display()
# save the spatial transformation:
res_name = 'example_trsf_rigid.trsf' # filename
trsf_rig.write(out_path+res_name)
# save the result image:
res_name = 'example_reg_rigid_1_2.tif' # filename
# SpatialImage to .tif format
imsave(out_path+res_name, res_rig)

# Affine registration:
trsf_aff, res_aff = registration(floating_img,
                                 reference_img,
                                 method='affine_registration')
res_name = 'example_reg_affine_1_2.tif' # filename
# SpatialImage to .tif format
imsave(out_path+res_name, res_aff)

# Deformable registration:
trsf_def, res_def = registration(floating_img,
                                 reference_img,
                                 method=
                                 'deformable_registration')
res_name = 'example_reg_deformable_1_2.tif' # filename
# SpatialImage to .tif format
imsave(out_path+res_name, res_def)

# Save the reference image:
res_name = 'example_reg_reference.tif' # filename
# SpatialImage to .tif format
imsave(out_path+res_name, reference_img)