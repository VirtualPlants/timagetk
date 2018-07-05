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

from ctypes import pointer
try:
    from timagetk.wrapping.clib import libblockmatching, c_stdout
    from timagetk.wrapping.balTrsf import BAL_TRSF
    from timagetk.wrapping.bal_image import BalImage
    from timagetk.wrapping.bal_matrix import BalMatrix
    from timagetk.wrapping.bal_stddef import enumTypeTransfo, enumUnitTransfo
except ImportError:
    raise ImportError('Import Error')


__all__ = ['bal_trsf_c_ptr',
           'bal_trsf_c_struct',
           'free_bal_trsf',
           'BalTransformation'
           ]

TRSF_TYPE_DICT = {
    0: "UNDEF_TRANSFORMATION",
    1: "TRANSLATION_2D",
    2: "TRANSLATION_3D",
    3: "TRANSLATION_SCALING_2D",
    4: "TRANSLATION_SCALING_3D",
    5: "RIGID_2D",
    6: "RIGID_3D",
    7: "SIMILITUDE_2D",
    8: "SIMILITUDE_3D",
    9: "AFFINE_2D",
    10: "AFFINE_3D",
    11: "VECTORFIELD_2D",
    12: "VECTORFIELD_3D",
    13: "SPLINE"
}


def bal_trsf_c_ptr(c_or_bal_trsf):
    """
    Return a pointer to instance of BAL_TRSF.

    Parameters
    ----------
    c_or_bal_trsf : instance of BAL_TRSF

    Returns
    ----------
    output : pointer

    Example
    -------
    c_ptr = bal_trsf_c_ptr(c_or_bal_trsf)
    """
    if isinstance(c_or_bal_trsf, BAL_TRSF):
        return pointer(c_or_bal_trsf)
    else:
        return


def bal_trsf_c_struct(c_or_bal_trsf):
    """
    Return an instance of BAL_TRANSFORMATION
    """
    if isinstance(c_or_bal_trsf, BAL_TRSF):
        return c_or_bal_trsf
    else:
        return


def free_bal_trsf(c_or_bal_trsf):
    """
    Free trsf static attributes, not dynamics vx, vy, vz ant mat

    Parameters
    ----------
    c_or_bal_trsf : instance of BAL_TRSF

    Example
    -------
    free_bal_trsf(c_or_bal_trsf)
    """
    c_bal_trsf = bal_trsf_c_struct(c_or_bal_trsf)
    if c_bal_trsf is None:
        return
    c_bal_trsf.mat = None
    c_bal_trsf.vx = None
    c_bal_trsf.vy = None
    c_bal_trsf.vz = None

    ptr = pointer(c_bal_trsf)
    libblockmatching.BAL_FreeTransformation(ptr)


class BalTransformation(object, enumTypeTransfo, enumUnitTransfo):
    """
    Warning: this class do not yet follow bal_naming_convention and behaviour.
    It is just a container without memory management.

    You can only pass pointers in constructors. So to create a transformation, create matrix, images, ...
    outside and pass pointers (mat.c_ptr, image.c_ptr) to constructor.
    """

    def __init__(self, trsf_type=None, trsf_unit=None, c_bal_trsf=None):
        """
        """
        self._c_bal_trsf = BAL_TRSF()
        if c_bal_trsf is None:
            # WARNING: BAL_InitTransformation set type to undef!
            libblockmatching.BAL_InitTransformation(self.c_ptr) # type to undef !
            if trsf_unit is not None:
                self._c_bal_trsf.transformation_unit = trsf_unit
            if trsf_type is not None:
                self._c_bal_trsf.type = trsf_type
        else:
            if isinstance(c_bal_trsf, BAL_TRSF):
                libblockmatching.BAL_AllocTransformation(self.c_ptr, c_bal_trsf.type,
                                                         pointer(c_bal_trsf.vx))
                libblockmatching.BAL_CopyTransformation(pointer(c_bal_trsf), self.c_ptr)
            elif isinstance(c_bal_trsf, BalTransformation):
                libblockmatching.BAL_AllocTransformation(self.c_ptr, c_bal_trsf._c_bal_trsf.type,
                                                         c_bal_trsf.vx.c_ptr)
                libblockmatching.BAL_CopyTransformation(pointer(c_bal_trsf._c_bal_trsf), self.c_ptr)
            else:
                raise TypeError("Unknown type '{}' for 'c_bal_trsf'...".format(type(c_bal_trsf)))

        # Set object attributes:
        self.trsf_unit = self._c_bal_trsf.transformation_unit
        self.mat = BalMatrix(c_bal_matrix=self._c_bal_trsf.mat) #---- BalMatrix instance
        # TODO: move this if/else behaviour to BalImage (avoid unecessary warnings raise when calling SpatialImage)!
        if self._c_bal_trsf.vx is not None:
            self.vx = BalImage(c_bal_image=self._c_bal_trsf.vx) #---- BalImage instance
            self.vy = BalImage(c_bal_image=self._c_bal_trsf.vy) #---- BalImage instance
            self.vz = BalImage(c_bal_image=self._c_bal_trsf.vz) #---- BalImage instance
        else:
            self.vx = None
            self.vy = None
            self.vz = None
        self.trsf_type = self._c_bal_trsf.type


    def read(self, path):
        libblockmatching.BAL_ReadTransformation(self.c_ptr, str(path))
        self.trsf_type = self._c_bal_trsf.type
        self.trsf_unit = self._c_bal_trsf.transformation_unit

    def write(self, path):
        libblockmatching.BAL_WriteTransformation(self.c_ptr, str(path))

    def free(self):
        if self._c_bal_trsf:
            libblockmatching.BAL_FreeTransformation(self.c_ptr)
            # Remove ref to dynamic fields.
            # If these fields are used outside, memory is managed outside
            # Else, there are no more references to theses fields, gc collect Bal* object and delete it
            # When BalImage and BalMatrix are deleted, memory is released
            self.mat = None
            self.vx = None
            self.vy = None
            self.vz = None

    def __del__(self):
        self.free()

    @property
    def c_ptr(self):
        return pointer(self._c_bal_trsf)

    @property
    def c_struct(self):
        return self._c_bal_trsf

    def c_display(self, name=""):
        libblockmatching.BAL_PrintTransformation(c_stdout, self.c_ptr, name)

    def isLinear(self):
        """
        Test if the transformation matrix is of type 'Linear'.
        Linear transformation matrix are obtained from those types:
          - TRANSLATION_2D, TRANSLATION_3D;
          - TRANSLATION_SCALING_2D, TRANSLATION_SCALING_3D;
          - RIGID_2D, RIGID_3D;
          - SIMILITUDE_2D, SIMILITUDE_3D;
          - AFFINE_2D, AFFINE_3D.

        Returns
        -------
        isLinear: bool
            True if of type 'Linear', else False
        """
        return libblockmatching.BAL_IsTransformationLinear(self.c_ptr) != 0

    def isVectorField(self):
        """
        Test if the transformation matrix is of type 'VectorField'.
        Non-linear transformation matrix are obtained from those types:
          - VECTORFIELD_2D, VECTORFIELD_3D;

        Returns
        -------
        isVectorField: bool
            True if of type 'VectorField', else False
        """
        return libblockmatching.BAL_IsTransformationVectorField(self.c_ptr) != 0

    def get_type(self):
        """
        Returns the type of transformation matrix as a string.
        """
        return TRSF_TYPE_DICT[self.trsf_type]
