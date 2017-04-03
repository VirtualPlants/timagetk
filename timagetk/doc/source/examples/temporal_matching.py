# imports
import numpy as np
try:
    from timagetk.util import data_path
    from timagetk.components import imread, imsave, SpatialImage
    from timagetk.plugins import labels_post_processing
    from timagetk.algorithms import GeometricalFeatures
    from timagetk.algorithms import TemporalMatching
except ImportError:
    raise ImportError('Import Error')
try:
    import cPickle as pickle
except:
    import pickle

out_path = './results/'
# We consider two different times
times = [3,4]
segmentation_list, feature_space_list, back_id_list = [], [], []

for ind, val in enumerate(times):
    img = imread(data_path('time_' + str(val) + '_seg.inr'))

    # subimage extraction
    shape = img.get_shape()
    indices = [0,shape[0]-1,0,shape[1]-1,0,5]
    img = img.get_region(indices=indices)

    # remove small cells
    labels = np.unique(img).tolist()
    img = labels_post_processing(img, method='labels_erosion', radius=2)
    img[img==0] = np.min(labels)
    img = SpatialImage(img, voxelsize=img.get_voxelsize())

    # save input labeled images
    res_name = 'example_track_time_' + str(val) + '_seg.inr'
    imsave(out_path+res_name, img)

    labels = np.unique(img).tolist() # list of labels
    back_id = np.min(labels) # background identifier
    back_id_list.append(back_id)
    labels.remove(back_id)

    # feature space computation
    obj_gf = GeometricalFeatures(img, label=labels)
    feature_space = obj_gf.compute_feature_space()
    segmentation_list.append(img), feature_space_list.append(feature_space)

# temporal matching
obj_tm = TemporalMatching(segmentation_list=segmentation_list,
                          feature_space_list=feature_space_list,
                          background_id_list=back_id_list)

# computation of admissible matching
adm_match_list = obj_tm.get_admissible_matching_list()

# matching criterion (default is None)
criterion='Jaccard coefficient'
# computation of normalized matching costs
norm_cost_list = obj_tm.compute_normalized_cost(adm_match_list,
                                                criterion=criterion)

poss_opt = ['matching','iterative_matching']
opt = 'matching'

if opt in poss_opt:
    if opt=='matching':
        # appear/disappear cost
        alpha_cost = int((80/100.0)*obj_tm.max_cost)
        # matching
        matching_dict = obj_tm.matching(norm_cost_list, alpha_cost)
    elif opt=='iterative_matching':
        # initial graph
        init_graph = obj_tm.build_init_graph(norm_cost_list)
        # iterative matching
        matching_dict = obj_tm.iterative_matching(norm_cost_list, init_graph)

    # write a dictionary of matching
    pkl_path = ''.join([out_path, 'example_track_matching_dict.pkl'])
    pkl_file = open(str(pkl_path), 'wb')
    # save as pickle object (serialization)
    pickle.dump(matching_dict, pkl_file)
    pkl_file.close()

    # resulting images
    img_list = obj_tm.res_images(matching_dict)
    # save results
    res_name = 'example_track_initial_cells.inr'
    imsave(out_path+res_name, img_list[0])
    res_name = 'example_track_final_cells.inr'
    imsave(out_path+res_name, img_list[1])
    res_name = 'example_track_disapp_cells.inr'
    imsave(out_path+res_name, img_list[2])
    res_name = 'example_track_app_cells.inr'
    imsave(out_path+res_name, img_list[3])
else:
    print('Poss. opt :'), poss_opt
