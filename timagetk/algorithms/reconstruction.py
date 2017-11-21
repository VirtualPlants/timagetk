# -*- python -*-
# -*- coding: latin-1 -*-
#
#       timagetk.algorithms.reconstruction
#
#       Copyright 2006 - 2011 INRIA - CIRAD - INRA
#
#       File author(s): Eric MOSCARDI <eric.moscardi@gmail.com>
#                       Daniel BARBEAU <daniel.barbeau@inria.fr>
#
#       Distributed under the Cecill-C License.
#       See accompanying file LICENSE.txt or copy at
#       http://www.cecill.info/licences/Licence_CeCILL-C_V1-en.html
#
#       OpenAlea WebSite : http://openalea.gforge.inria.fr
################################################################################

__license__ = "Cecill-C"
__revision__ = " $Id$ "

# This is a revised copy of (deprecated?) package 'vplants.mars_alt.mars.reconstruction'
# It was using a library that is not available anymore (morpheme).

import numpy as np
import scipy.ndimage as ndimage
from openalea.image.algo.basic import stroke, end_margin
from openalea.image.algo.morpho import connectivity_4
from openalea.image.algo.morpho import connectivity_6
from openalea.image.algo.morpho import connectivity_8
from timagetk.algorithms import connexe
from timagetk.components import SpatialImage
from timagetk.plugins import linear_filtering


def im2surface(image, threshold_value=45, only_altitude=False,
               front_half_only=False):
    """
    This function computes a surfacic view of the meristem, according to a revisited version
    of the method described in Barbier de Reuille and al. in Plant Journal.

    Parameters
    ----------
    image : SpatialImage
        image to be masked
    threshold_value : int, float
        consider intensities superior to threshold.
    only_altitude : bool
        only return altitude map, not maximum intensity projection
    front_half_only : bool
        only consider the first half of all slices in the Z direction.

    Returns
    -------
    mip_img : SpatialImage
        maximum intensity projection. *None* if only_altitude in True
    alt_img : SpatialImage
        altitude of maximum intensity projection
    """
    resolution = image.resolution
    # Added gaussian smoothing to reduce
    img_th = linear_filtering(image, 'gaussian_smoothing', std_dev=2.0)
    img_th = img_th >= threshold_value
    img_th = SpatialImage(img_th.astype(np.uint8), voxelsize=resolution)

    # ~ labeling, n = component_labeling(img_th, connectivity_26, number_labels=1)
    labeling = connexe(img_th, param_str_1='-labels -connectivity 26 -debug')
    del img_th

    iterations = 15
    dilation1 = ndimage.binary_dilation(labeling, connectivity_8, iterations)
    del labeling

    iterations = 10
    erosion1 = ndimage.binary_erosion(dilation1, connectivity_8, iterations,
                                      border_value=1)
    del dilation1

    iterations = 15
    dilation2 = ndimage.binary_dilation(erosion1, connectivity_8, iterations)
    del erosion1

    iterations = 4
    erosion2 = ndimage.binary_erosion(dilation2, connectivity_4, iterations,
                                      border_value=1)
    del dilation2

    iterations = 15
    erosion3 = ndimage.binary_erosion(erosion2, connectivity_8, iterations,
                                      border_value=1)
    del erosion2

    iterations = 1
    erosion4 = ndimage.binary_erosion(erosion3, connectivity_6, iterations,
                                      border_value=1)

    # CONDITIONNER_POUR_ISOSURFACE
    mat1 = stroke(erosion4, 1)

    iterations = 3
    erosion5 = ndimage.binary_erosion(mat1, connectivity_6, iterations,
                                      border_value=1)
    del mat1

    iterations = 9
    erosion6 = ndimage.binary_erosion(erosion5, connectivity_8, iterations,
                                      border_value=1)
    del erosion5

    m_xor = np.logical_xor(erosion3, erosion6)
    del erosion3
    del erosion6

    # METTRE_A_ZERO_LES_DERNIERES_COUPES
    mat2 = end_margin(m_xor, 10, 2)
    del m_xor

    mat2 = np.ubyte(mat2)
    m_and = np.where(mat2 == 1, image, 0)
    del mat2

    if front_half_only:
        m_and[:, :, m_and.shape[2] / 2:] = 0

    # ALTITUDE_DU_MIP_DANS_MASQUE_BINAIRE
    x, y, z = m_and.shape
    m_alt = m_and.argmax(2).reshape(x, y, 1)

    del image
    if not only_altitude:
        m_mip = m_and.max(2).reshape(x, y, 1)
        return SpatialImage(m_mip, resolution), SpatialImage(m_alt, resolution)
    else:
        return None, SpatialImage(m_alt, resolution)


def surface2im(points, altitude):
    """
    This function is used to convert points from maximum intensity projection
    to the real world.

    Parameters
    ----------
    points : list
        list of points from the maximum intensity projection
    altitude : |SpatialImage|
        altitude of maximum intensity projection

    Returns
    -------
    coord : list
        list of points in the real world
    """
    assert isinstance(altitude, SpatialImage)
    coord = list()

    vx = altitude.resolution[0]
    vy = altitude.resolution[1]
    vz = altitude.resolution[2]

    for pt in points:
        c = (pt[0] * vx, pt[1] * vy, (altitude[pt[0], pt[1], 0]) * vz)
        coord.append(c)
    return coord


def spatialise_matrix_points(points, image, mip_thresh=45):
    """
    Given a list of points in matrix coordinates (i.e. i,j,k - but k is ignored anyway),
    and a spatial image, returns a list of points in real space (x,y,z) with the Z coordinate
    recomputed from an altitude map extracted from image. This implies that `points` were placed
    on the mip/altitude map result of `im2surface` applied to `image` with `mip_thresh`.

    Parameters
    ----------
    points : list[ of tuple[of float,float,float],
        file 2D points to spatialise.
        Can also be a filename pointing to a numpy-compatible list of points.
    image : |SpatialImage|, str
        image or path to image to use to spatialise the points.
    mip_thresh : int, float
        threshold used to compute the original altitude map for points.

    Returns
    -------
    points3d : list [of tuple [of float, float, float]]
        3D points in REAL coordinates.
    """
    image, was_path = lazy_image_or_path(image)
    if isinstance(points, (str, unicode)):
        points = np.loadtxt(points)
    return surface2im(points, im2surface(image, threshold_value=mip_thresh,
                                         only_altitude=True)[1])


def surface_landmark_matching(ref, ref_pts, flo, flo_pts,
                              ref_pts_already_spatialised=False,
                              flo_pts_already_spatialised=False,
                              mip_thresh=45):
    """ Computes the registration of "flo" to "ref" by minimizing distances between ref_pts and flo_pts.

    .. note::

    If `ref_pts_already_spatialised` and `flo_pts_already_spatialised` are True and `ref_pts` and
    `flo_pts` are indeed in real 3D coordinates, then this is exactly a landmark matching registration

    This function is implemented on top of
    :func:`~openalea.image.registration.registration.pts2transfo` .

    Parameters
    ----------
    ref : |SpatialImage|, str
        image or path to image to use to reference image.
    ref_pts : list
        ordered sequence of 2D/3D points to use as reference landmarks.
    flo : |SpatialImage|, str
        image or path to image to use to floating image
    flo_pts : list
        ordered sequence of 2D/3D points to use as floating landmarks.
    ref_pts_already_spatialised : bool
        If True, consider reference points are already in REAL 3D space.
    flo_pts_already_spatialised : bool
        If True, consider floating points are already in REAL 3D space.
    mip_thresh : int, float
        used to recompute altitude map to project points in 3D if they aren't spatialised.

    Returns
    -------
    trs : numpy.ndarray
        The result is a 4x4 **resampling voxel matrix** (*i.e.* from ref to flo,
         from ref_space to flo_space and NOT from real_space to real_space).
    """
    ref, was_path = lazy_image_or_path(ref)
    flo, was_path = lazy_image_or_path(flo)

    if isinstance(ref_pts, (str, unicode)):
        ref_pts = np.loadtxt(ref_pts)

    if isinstance(flo_pts, (str, unicode)):
        flo_pts = np.loadtxt(flo_pts)

    if not ref_pts_already_spatialised:
        print "spatialising reference"
        ref_spa_pts = spatialise_matrix_points(ref_pts, ref,
                                               mip_thresh=mip_thresh)
    else:
        print "not spatialising reference"
        ref_spa_pts = ref_pts

    if not flo_pts_already_spatialised:
        print "spatialising floating"
        flo_spa_pts = spatialise_matrix_points(flo_pts, flo,
                                               mip_thresh=mip_thresh)
    else:
        print "not spatialising floating"
        flo_spa_pts = flo_pts

    trs = pts2transfo(ref_spa_pts, flo_spa_pts)

    # -- trs is from ref to flo, in other words it is T-1,
    # a resampling matrix to put flo into ref space. ref_pts and
    # flo_pts are in real coordinates so the matrix is also in
    # real coordinates and must be converted to voxels --
    trs_vox = matrix_real2voxels(trs, flo.resolution, ref.resolution)
    return trs_vox
