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

#--- see vt_connexe.h
from ctypes import Structure, c_int
try:
    from timagetk.wrapping.vt_typedefs import DimType
    from timagetk.wrapping.vt_neighborhood import Neighborhood
except ImportError as e:
    raise ImportError('Import Error: {}'.format(e))

VT_BINAIRE = 1
VT_GREY = 2
VT_SIZE = 3


class _VT_CONNEXE(Structure):
    """ vt_connexe structure: see vt_connexe.h """
    _fields_ = [("min_size", c_int),
                ("max_nbcc", c_int),
                ("type_connexite", c_int),
                ("type_output", c_int),
                ("dim", c_int),
                ("verbose", c_int),
                ("threshold", c_int)
                ]


class VT_Connexe (object):
    """ Definition of VT_Connexe object """

    def __init__(self):
        """ Initialization : """

    def set_default(self):
        """ Default """
        _min_size = 1
        _max_nbcc = -1
        _type_connexite = Neighborhood.N26
        _type_output = VT_BINAIRE
        _dim = DimType.VT_3D
        _verbose = 0
        _threshold = 0
        vt_connexe = _VT_CONNEXE(
            _min_size, _max_nbcc, _type_connexite, _type_output, _dim, _verbose, _threshold)
        return vt_connexe
