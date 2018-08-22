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

try:
    from timagetk.plugins.linear_filtering import linear_filtering
except ImportError:
    raise ImportError("Import Error with 'linear_filtering' plugins!")
try:
    from timagetk.plugins.morphology import morphology
except ImportError:
    raise ImportError("Import Error with 'morphology' plugins!")
try:
    from timagetk.plugins.averaging import averaging
except ImportError:
    raise ImportError("Import Error with 'averaging' plugins!")
try:
    from timagetk.plugins.h_transform import h_transform
except ImportError:
    raise ImportError("Import Error with 'h_transform' plugins!")
try:
    from timagetk.plugins.region_labeling import region_labeling
except ImportError:
    raise ImportError("Import Error with 'region_labeling' plugins!")
try:
    from timagetk.plugins.segmentation import segmentation
except ImportError:
    raise ImportError("Import Error with 'segmentation' plugins!")
try:
    from timagetk.plugins.labels_post_processing import labels_post_processing
except ImportError:
    raise ImportError("Import Error with 'labels_post_processing' plugins!")
try:
    from timagetk.plugins.registration import registration
except ImportError:
    raise ImportError("Import Error with 'registration' plugins!")
try:
    from timagetk.plugins.sequence_registration import sequence_registration
except ImportError:
    raise ImportError("Import Error with 'sequence_registration' plugins!")
