# imports
import os
try:
    from timagetk.util import data_path
    from timagetk.components import imread, imsave
    from timagetk.plugins import linear_filtering, morphology
    from timagetk.plugins import h_transform
    from timagetk.plugins import region_labeling, segmentation
    from timagetk.plugins import labels_post_processing
except ImportError:
    raise ImportError('Import Error')

out_path = './results/' # to save results
if not os.path.isdir(out_path):
    new_fold = os.path.join(os.getcwd(),'results')
    os.mkdir(new_fold)

# we consider an input image
# SpatialImage instance
input_img = imread(data_path('input.tif'))

# optional denoising block
smooth_img = linear_filtering(input_img, std_dev=2.0,
                              method='gaussian_smoothing')
asf_img = morphology(smooth_img, max_radius=3,
                     method='co_alternate_sequential_filter')

ext_img = h_transform(asf_img, h=150,
                      method='h_transform_min')
con_img = region_labeling(ext_img, low_threshold=1,
                          high_threshold=150,
                          method='connected_components')
seg_img = segmentation(smooth_img, con_img, control='first',
                       method='seeded_watershed')

# optional post processig block
pp_img = labels_post_processing(seg_img, radius=1,
                                iterations=1,
                                method='labels_erosion')

res_name = 'example_segmentation.tif'
imsave(out_path+res_name, pp_img)
