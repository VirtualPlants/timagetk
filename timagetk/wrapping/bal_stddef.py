# -*- coding: utf-8 -*-
# -*- python -*-
#
#
#       Copyright 2016 INRIA
#
#       File author(s):
#           Guillaume Baty <guillaume.baty@inria.fr>
#           Sophie Ribes <sophie.ribes@inria.fr>
#
#       See accompanying file LICENSE.txt
#------------------------------------------------------------------------------

# see bal-stddef.h
__all__ = ['enumTypeTransfo',
           'enumUnitTransfo']


class enumTypeTransfo:
    """
    enumTypeTransfo: enumeration of the different types of transformations.
    """
    UNDEF_TRANSFORMATION = 0
    TRANSLATION_2D = 1
    TRANSLATION_3D = 2
    TRANSLATION_SCALING_2D = 3
    TRANSLATION_SCALING_3D = 4
    RIGID_2D = 5
    RIGID_3D = 6
    SIMILITUDE_2D = 7
    SIMILITUDE_3D = 8
    AFFINE_2D = 9
    AFFINE_3D = 10
    VECTORFIELD_2D = 11
    VECTORFIELD_3D = 12
    SPLINE = 13


class enumUnitTransfo:
    """
    enumUnitTransfo: enumeration of the different types of units.
    """
    UNDEF_UNIT = 0
    VOXEL_UNIT = 1
    REAL_UNIT = 2

#def enumTypeTransfo_dict = {0:UNDEF_TRANSFORMATION, 1:TRANSLATION_2D, 2:TRANSLATION_3D}
