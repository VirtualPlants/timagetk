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

"""
This module contains plugins for image algorithms: filtering, semgentation, ...
"""

from openalea.core.plugin import PluginDef
from openalea.oalab.mimedata.plugin import QMimeCodecPlugin

from openalea.core.authors import gbaty, sribes, gmalandain


VIRTUALPLANTS = [gbaty, sribes]
MORPHEME = [gmalandain]
MORHOGENETICS = VIRTUALPLANTS + MORPHEME
image = {'name': 'image', 'interface': 'IImage', 'label': 'Input image'}
max_radius = {'name': 'max_radius', 'default': 1, 'interface': 'IInt', 'label': 'Max radius'}
#trsf = {'name': 'trsf', 'interface': 'ITrsf', 'label': 'Input trsf'}


class MorphogeneticTeamPlugin(object):
    authors = MORHOGENETICS

class FilteringPlugin(MorphogeneticTeamPlugin):
    implement = 'filtering'
    modulename = 'timagetk.plugins.linear_filtering'
    inputs = [{'name': 'image', 'interface': 'IImage'}]
    outputs = [{'name': 'filtered', 'interface': 'IImage'}]

class MorphoPlugin(MorphogeneticTeamPlugin):
    implement = 'morphology'
    modulename = 'timagetk.plugins.morphology'
    inputs = [
        {'name': 'image', 'interface': 'IImage'},
        {'name': 'radius', 'default': 1, 'interface': 'IInt', 'label': 'Dilation Sphere Radius'},
        {'name': 'iterations', 'default': 1, 'interface': 'IInt', 'label': 'Number of iterations'}]
    outputs = [{'name': 'output', 'interface': 'IImage'}]

class RegionLabelingPlugin(MorphogeneticTeamPlugin):
    implement = 'region labeling'
    modulename = 'timagetk.plugins.region_labeling'
    inputs = [
        {'name': 'image', 'interface': 'IImage'},
        {'name': 'low_threshold', 'default': 1, 'interface': 'IInt', 'label': 'Low threshold'},
        {'name': 'high_threshold', 'default': 3, 'interface': 'IInt', 'label': 'High threshold'}]
    outputs = [{'name': 'output', 'interface': 'IImage'}]

class SegmentationPlugin(MorphogeneticTeamPlugin):
    implement = 'segmentation'
    modulename = 'timagetk.plugins.segmentation'
    inputs = [
        {'name': 'image', 'interface': 'IImage'},
        {'name': 'seeds', 'interface': 'IImage'}]
    outputs = [{'name': 'output', 'interface': 'IImage'}]

class AveragingPlugin(MorphogeneticTeamPlugin):
    implement = 'averaging'
    modulename = 'timagetk.plugins.averaging'
    inputs = [
        {'name': 'image', 'interface': 'IImage'}]
    outputs = [{'name': 'output', 'interface': 'IImage'}]

class LabelsPostProcessingPlugin(MorphogeneticTeamPlugin):
    implement = 'labels post processing'
    modulename = 'timagetk.plugins.labels_post_processing'
    inputs = [
        {'name': 'image', 'interface': 'IImage'}]
    outputs = [{'name': 'output', 'interface': 'IImage'}]

class HTransformPlugin(MorphogeneticTeamPlugin):
    implement = 'h_transforms'
    modulename = 'timagetk.plugins.h_transform'
    inputs = [
        {'name': 'image', 'interface': 'IImage'},
        {'name': 'h', 'default': 1, 'interface': 'IInt', 'label': 'Height'}]
    outputs = [{'name': 'output', 'interface': 'IImage'}]

class RegistrationPlugin(MorphogeneticTeamPlugin):
    implement = 'registration'
    modulename = 'timagetk.plugins.registration'
    inputs = [
        {'name': 'floating_img', 'interface': 'IImage'},
        {'name': 'reference_img', 'interface': 'IImage'}]
    outputs = [{'name': 'trsf_out', 'interface': 'ITrsf'},
                {'name': 'res_image', 'interface': 'IImage'}]

class SequenceRegistrationPlugin(MorphogeneticTeamPlugin):
    implement = 'sequence registration'
    modulename = 'timagetk.plugins.sequence_registration'
    inputs = [{'name': 'list_images', 'interface': 'IImage'}]
    outputs = [{'name': 'trsf_out', 'interface': 'ITrsf'},
               {'name': 'list_res_img', 'interface': 'IImage'}]

# --- Linear Filter plugins
@PluginDef
class GaussianSmoothingPlugin(FilteringPlugin):
    implement = 'Linear filtering'
    objectname = 'linear_filtering_gaussian_smoothing'
    label = 'Gaussian smoothing filter'
    inputs = FilteringPlugin.inputs + [
        {'name': 'std_deviation', 'default': 0.5, 'interface': 'IFloat', 'label': 'Standard Deviation'}
    ]

@PluginDef
class GradientModulusPlugin(FilteringPlugin):
    implement = 'Linear filtering'
    objectname = 'linear_filtering_gradient_modulus'
    label = 'Gradient modulus filter'

@PluginDef
class GradientPlugin(FilteringPlugin):
    implement = 'Linear filtering'
    objectname = 'linear_filtering_gradient'
    label = 'Gradient filter'

@PluginDef
class HessianPlugin(FilteringPlugin):
    implement = 'Linear filtering'
    objectname = 'linear_filtering_hessian'
    label = 'Hessian filter'

@PluginDef
class LaplacianPlugin(FilteringPlugin):
    implement = 'Linear filtering'
    objectname = 'linear_filtering_laplacian'
    label = 'Laplacian filter'

@PluginDef
class ZeroCrossingsHessianPlugin(FilteringPlugin):
    implement = 'Linear filtering'
    objectname = 'linear_filtering_zero_crossings_hessian'
    label = 'Zero crossings hessian filter'

@PluginDef
class ZeroCrossingsLaplacianPlugin(FilteringPlugin):
    implement = 'Linear filtering'
    objectname = 'linear_filtering_zero_crossings_laplacian'
    label = 'Zero crossings laplacian filter'

@PluginDef
class GradientHessianPlugin(FilteringPlugin):
    implement = 'Linear filtering'
    objectname = 'linear_filtering_gradient_hessian'
    label = 'Gradient hessian filter'

@PluginDef
class GradientLaplacianPlugin(FilteringPlugin):
    implement = 'Linear filtering'
    objectname = 'linear_filtering_gradient_laplacian'
    label = 'Gradient laplacian filter'

# --- Morphology plugins
@PluginDef
class DilationPlugin(MorphoPlugin):
    implement = 'Morphology'
    objectname = 'morphology_dilation'
    label = 'Dilation filter'

@PluginDef
class ErosionPlugin(MorphoPlugin):
    implement = 'Morphology'
    objectname = 'morphology_erosion'
    label = 'Erosion filter'

@PluginDef
class OpeningPlugin(MorphoPlugin):
    implement = 'Morphology'
    objectname = 'morphology_opening'
    label = 'Opening filter'

@PluginDef
class ClosingPlugin(MorphoPlugin):
    implement = 'Morphology'
    objectname = 'morphology_closing'
    label = 'Closing filter'

@PluginDef
class HatTransformPlugin(MorphoPlugin):
    implement = 'Morphology'
    objectname = 'morphology_hat_transform'
    label = 'Hat transform filter'

@PluginDef
class InverseHatTransformPlugin(MorphoPlugin):
    implement = 'Morphology'
    objectname = 'morphology_inverse_hat_transform'
    label = 'Inverse hat transform filter'

@PluginDef
class ContrastPlugin(MorphoPlugin):
    implement = 'Morphology'
    objectname = 'morphology_contrast'
    label = 'Contrast filter'

@PluginDef
class MorphoGradientPlugin(MorphoPlugin):
    implement = 'Morphology'
    objectname = 'morphology_gradient'
    label = 'Morphological gradient filter'

@PluginDef
class OcAlternateSequentialFilterPlugin(MorphoPlugin):
    implement = 'Morphology'
    objectname = 'morphology_oc_alternate_sequential_filter'
    label = 'OC alternate sequential filter'
    inputs = [image, max_radius]

@PluginDef
class CoAlternateSequentialFilterPlugin(MorphoPlugin):
    implement = 'Morphology'
    objectname = 'morphology_co_alternate_sequential_filter'
    label = 'CO alternate sequential filter'
    inputs = [image, max_radius]

@PluginDef
class CocAlternateSequentialFilterPlugin(MorphoPlugin):
    implement = 'Morphology'
    objectname = 'morphology_coc_alternate_sequential_filter'
    label = 'COC alternate sequential filter'
    inputs = [image, max_radius]

@PluginDef
class OcoAlternateSequentialFilterPlugin(MorphoPlugin):
    implement = 'Morphology'
    objectname = 'morphology_oco_alternate_sequential_filter'
    label = 'OCO alternate sequential filter'
    inputs = [image, max_radius]

# --- Region labeling plugins
@PluginDef
class ConnectedComponentsPlugin(RegionLabelingPlugin):
    implement = 'Region labeling'
    objectname = 'connected_components'
    label = 'Connected components labeling'

# --- Segmentation plugins
@PluginDef
class SeededWatershedPlugin(SegmentationPlugin):
    implement = 'Segmentation'
    objectname = 'seeded_watershed'
    label = 'Seeded watershed segmentation'

# --- Averaging plugins
@PluginDef
class MeanAveragingPlugin(AveragingPlugin):
    implement = 'Averaging'
    objectname = 'mean_averaging'
    label = 'Mean averaging'

@PluginDef
class RobustMeanAveragingPlugin(AveragingPlugin):
    implement = 'Averaging'
    objectname = 'robust_mean_averaging'
    label = 'Robust mean averaging'

@PluginDef
class MedianAveragingPlugin(AveragingPlugin):
    implement = 'Averaging'
    objectname = 'median_averaging'
    label = 'Median averaging'

@PluginDef
class MinimumAveragingPlugin(AveragingPlugin):
    implement = 'Averaging'
    objectname = 'min_averaging'
    label = 'Minimum averaging'

@PluginDef
class MaximumAveragingPlugin(AveragingPlugin):
    implement = 'Averaging'
    objectname = 'max_averaging'
    label = 'Maximum averaging'

# --- Labels post-processing plugins
@PluginDef
class LabelsErosionPlugin(LabelsPostProcessingPlugin):
    implement = 'Labels post processing'
    objectname = 'labels_erosion'
    label = 'Labels erosion'

@PluginDef
class LabelsDilationPlugin(LabelsPostProcessingPlugin):
    implement = 'Labels post processing'
    objectname = 'labels_dilation'
    label = 'Labels dilation'

@PluginDef
class LabelsOpeningPlugin(LabelsPostProcessingPlugin):
    implement = 'Labels post processing'
    objectname = 'labels_opening'
    label = 'Labels opening'

@PluginDef
class LabelsClosingPlugin(LabelsPostProcessingPlugin):
    implement = 'Labels post processing'
    objectname = 'labels_closing'
    label = 'Labels closing'

# --- H-transform plugins
@PluginDef
class HTransformMinPlugin(HTransformPlugin):
    implement = 'H transform'
    objectname = 'h_transform_min'
    label = 'Min h transform'

@PluginDef
class HTransformMaxPlugin(HTransformPlugin):
    implement = 'H transform'
    objectname = 'h_transform_max'
    label = 'Max h transform'

#--- Registration plugins
@PluginDef
class RigidRegistrationPlugin(RegistrationPlugin):
    implement = 'Registration'
    objectname = 'rigid_registration'
    label = 'Rigid registration'

@PluginDef
class AffineRegistrationPlugin(RegistrationPlugin):
    implement = 'Registration'
    objectname = 'affine_registration'
    label = 'Affine registration'

@PluginDef
class DeformableRegistrationPlugin(RegistrationPlugin):
    implement = 'Registration'
    objectname = 'deformable_registration'
    label = 'Deformable registration'

# --- Sequence registration plugins
@PluginDef
class SequenceRigidRegistrationPlugin(SequenceRegistrationPlugin):
    implement = 'Sequence Registration'
    objectname = 'sequence_registration_rigid'
    label = 'Sequence rigid registration'

@PluginDef
class SequenceAffineRegistrationPlugin(SequenceRegistrationPlugin):
    implement = 'Sequence Registration'
    objectname = 'sequence_registration_affine'
    label = 'Sequence affine registration'


# --- Mimetype codec plugin
@PluginDef
class IImageCodecPlugin(QMimeCodecPlugin):
    implement = 'IQMimeCodec'
    qtdecode = [
        ('openalealab/data', 'openalea/interface.IImage'),
        ('text/uri-list', 'openalea/interface.IImage'),
    ]

    def __call__(self):
        from timagetk.plugins.mimedata import IImageCodec
        return IImageCodec


###############################################################################
# Region plugins
###############################################################################

#from openalea.core.util import camel_case_to_lower

#class RegionPlugin(MorphogenticTeamPlugin):

#    def __call__(self):
#        funcname = self.implement + '_' + camel_case_to_lower(self.__class__.__name__.replace('Plugin', ''))
#        module = __import__('vplants.mars_alt.mars.region_segmentation', fromlist=[funcname])

#        return getattr(module, funcname)

#    label = property(fget=lambda self: ' '.join([e.capitalize() for e in self.name.split('_')]))

###############################################################################
# Region labeling
###############################################################################
#@PluginDef
#class SeedExtractionPlugin(RegionPlugin):
#    implement = 'region_labeling'

#@PluginDef
#class ConnectedComponentExtractionPlugin(RegionPlugin):
#    implement = 'region_labeling'

###############################################################################
# Region selection
###############################################################################
#@PluginDef
#class RemoveRegionLabelsGenericPlugin(RegionPlugin):
#    implement = 'region_selection'

#@PluginDef
#class RemoveSmallSizedCellsPlugin(RegionPlugin):
#    implement = 'region_selection'

#@PluginDef
#class RemoveBigSizedCellsPlugin(RegionPlugin):
#    implement = 'region_selection'

#@PluginDef
#class RemoveAnormalSizedCellsPlugin(RegionPlugin):
#    implement = 'region_selection'

###############################################################################
# Region extension
###############################################################################

#@PluginDef
#class WatershedPlugin(RegionPlugin):
#    implement = 'region_extension'


#@PluginDef
#class LevelsetPlugin(RegionPlugin):
#    implement = 'region_extension'


###############################################################################
# Region segmentation
###############################################################################
#@PluginDef
#class ExtensionBasedPlugin(RegionPlugin):
#    implement = 'region_segmentation'
