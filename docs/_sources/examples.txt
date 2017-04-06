.. _ref_examples:

*********
Examples
*********

All sample scripts can be found in the folder **timagetk/timagetk/examples/** and can be launched.

For example, to run the script ``input_output.py``, open a shell prompt, go the
the folder examples and type: ``python input_output.py``

.. _ref_examples_input_output:

Input/Output
************

This example illustrates input and output operations (see :ref:`ref_package_components`).

The resulting images (see folder **timagetk/timagetk/examples/results/**) can be checked against the
reference image by using any image visualization tool (such as **TissueLab** or **fiji**).

.. literalinclude:: ./examples/input_output.py

You can find a detailed description of:

* data structure ``SpatialImage``, see :ref:`ref_spatial_image`
* functions ``imread`` and ``imsave``, see :ref:`ref_input_output`


Rigid, affine and deformable registration
*****************************************

This example illustrates rigid, affine and deformable registration (see :ref:`ref_package_registration`).

A registration process requires two input images (``floating_img`` and ``reference_img``) and computes:

* a spatial transformation ``trsf_out`` between the two input images,
* a result image ``img_res`` corresponding to the floating image resampled into the reference frame.

The resulting images (see folder **timagetk/timagetk/examples/results/**) can be checked against the
reference image by using any image visualization tool (such as **TissueLab** or **fiji**).

.. image:: _static/registration.jpg
    :width: 500px
    :height: 420px
    :align: center

.. literalinclude:: ./examples/registration.py


Sequence Registration
*********************

This example illustrates rigid and affine registration along a whole sequence (see :ref:`ref_package_sequence_registration`).

This process requires a list of images (a sequence) and computes:

* a list of spatial transformations ``list_compo_trsf`` between successive images and the reference image,
* a list of result images ``list_res_img`` corresponding to the successive images resampled into the reference frame.

By default, the reference image is the last image of the sequence and all images are resampled into this frame.
The resulting images (see folder **timagetk/timagetk/examples/results/**) can be checked against the
reference image by using any image visualization tool (such as **TissueLab** or **fiji**).

.. image:: _static/seq_rigid_registration.jpg
    :width: 550px
    :height: 400px
    :align: center

.. literalinclude:: ./examples/sequence_registration.py


Seeded-watershed segmentation
*****************************

This example illustrates seeded-watershed segmentation.

.. literalinclude:: ./examples/segmentation.py


Computation of features
***********************

This example illustrates computation of geometrical features on labeled images.
This process requires a labeled image and computes several features such as moments,
spatial relationships, etc. This algorithm returns a dictionary that can be pickled
(see pickle module).

.. image:: _static/geometrical_features.jpg
    :width: 330px
    :height: 280px
    :align: center

.. literalinclude:: ./examples/geometrical_features.py

.. code-block:: python

   Label :  137 {'Neighbors': [32, 124, 149, 195, 282, 297, 298, 312],
                 'Distances': {32: 5.28, 195: 6.963, 297: 4.348, 298: 5.989,
                              149: 5.8, 312: 4.783, 282: 5.665, 124: 7.703},
                 'Bounding box volume': 12180,
                 'Physical centroid': (55.428, 62.878, 1.191),
                 'Label': 137, 'Bounding box size': (29, 30, 14),
                 'Physical volume': 117.329, 'Index centroid': (186, 211, 3),
                 'Bounding box': (172, 200, 197, 226, 0, 13)}


Temporal tracking
*****************

This example illustrates tracking of objects on labeled images.

.. literalinclude:: ./examples/temporal_matching.py