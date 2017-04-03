# imports
import numpy as np
try:
    from timagetk.util import data_path
    from timagetk.components import imread
    from timagetk.algorithms import GeometricalFeatures
except ImportError:
    raise ImportError('Import Error')
try:
    import cPickle as pickle
except:
    import pickle

out_path = './results/' # to save results
# We consider two different times
times = [3,4]
list_images = [imread(data_path('time_' + str(time) + '_seg.inr'))
                for time in times] # list of SpatialImage
feature_space_list = []
for ind, time in enumerate(times):
    segmentation = list_images[ind] # segmentations
    labels = np.unique(segmentation).tolist() # labels
    background_id = np.min(labels)
    labels.remove(background_id) # remove background
    int_seg = GeometricalFeatures(segmentation, label=labels) # GeometricalFeatures object
    feature_space_dict = int_seg.compute_feature_space(background_id=background_id) # compute feature space
    feature_space_list.append(feature_space_dict)

for ind, time in enumerate(times):
    pkl_path = ''.join([out_path, 'example_geom_feat_time_', str(times[ind]), '.pkl'])
    out = open(str(pkl_path), 'wb')
    # save as pickle object (serialization)
    pickle.dump(feature_space_list[ind], out)
    out.close()

print('')
sp_label = 137
print('Retrieve information for the label :'), sp_label
print('')
print('Label : '), str(sp_label), feature_space_list[0][sp_label]
print('')
print('Label : '), str(sp_label), feature_space_list[1][sp_label]