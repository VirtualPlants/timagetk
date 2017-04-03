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

from ctypes import Structure, c_int, c_float
try:
    from timagetk.wrapping.balMatrix import BAL_MATRIX
    from timagetk.wrapping.balImage import BAL_IMAGE
except ImportError:
    raise ImportError('Import Error')


__all__ = ['BAL_TRSF']


class BAL_TRSF(Structure):
    """
    typedef struct {

      enumTypeTransfo type;
      BAL_MATRIX mat;
      enumUnitTransfo transformation_unit;
      bal_image vx;
      bal_image vy;
      bal_image vz;

      /* for transformation averaging
       */
      float weight;
      float error;

    } bal_transformation;
    """
    _fields_ = [("type", c_int),  # enumTypeTransfo (see bal_matrix)
                ("mat", BAL_MATRIX), # BAL_MATRIX struct (see bal_stddef)
                # enumUnitTransfo struct (see bal_stddef)
                ("transformation_unit", c_int),
                ("vx", BAL_IMAGE),
                ("vy", BAL_IMAGE),
                ("vz", BAL_IMAGE),
                ("weight", c_float),
                ("error", c_float),
                ]
