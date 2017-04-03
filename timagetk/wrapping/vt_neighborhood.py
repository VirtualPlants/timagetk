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


class Neighborhood:
    """
    Neighborhood enumeration:
    * - C_04, N04 = 4,  4-neighborhood (2D)
    * - C_06, N06 = 6,  6-neighborhood (3D)
    * - C_08, N08 = 8,  8-neighborhood (2D)
    * - C_10n N10 = 10,  10-neighborhood (?)
    * - C_18n N18 = 18,  18-neighborhood (3D)
    * - C_26, N26 = 26  26-neighborhood (3D)
    """
    C_04, C_06, C_08, C_10, C_18, C_26 = 4, 6, 8, 10, 18, 26
    N04, N06, N08, N10, N18, N26 = C_04, C_06, C_08, C_10, C_18, C_26
