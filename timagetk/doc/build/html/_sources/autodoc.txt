.. _ref_documentation:

*************
Documentation
*************

.. _ref_package_components:

Package Components
******************

The package Components contains both a data structure (``SpatialImage``) and functionalities
dedicated to input/output operations (``imread``, ``imsave``).

It consists in two modules : SpatialImage (see :ref:`ref_spatial_image`) and Input/Ouput (see :ref:`ref_input_output`).

.. _ref_spatial_image:

Module Spatial Image
=====================

.. automodule:: components.spatial_image
   :members:
   :special-members:

.. _ref_input_output:

Module Input/Output
=====================

.. automodule:: components.io
   :members:

.. _ref_package_plugins:

Package Plugins
***************

**Tissue Image Toolkit** (timagetk) can be used with or without the plug-in architecture provided
by **OpenAleaLab** and implemented within the platform **TissueLab**. **TissueLab** will automatically
detect the presence of **Tissue Image Toolkit** as plugins. Note that these plugins can be used independently
of the **TissueLab** environment.

**Tissue Image Toolkit** is based on a set of X groups of plugins whose API
(*Application Programming Interface*) are standardized, and these groups provide an
access to a total of X methods. The groups of plugins are:

* Linear Filtering (9 methods, see :ref:`ref_package_linear_filtering`)
* Grayscale Morphology (12 methods, see :ref:`ref_package_morphology`)
* Averaging (5 methods, see :ref:`ref_package_averaging`)
* H-transform (2 methods, see :ref:`ref_package_h_transform`)
* Region Labeling (1 method, see :ref:`ref_package_region_labeling`)
* Segmentation (1 method, see :ref:`ref_package_segmentation`)
* Labels Post Processing (X methods, see :ref:`ref_package_labels_post_processing`)
* Registration (3 methods, see :ref:`ref_package_registration`)
* Sequence Registration (3 methods, see :ref:`ref_package_sequence_registration`)

.. _ref_package_linear_filtering:

Module Linear Filtering
========================

.. automodule:: plugins.linear_filtering
   :members:

.. _ref_package_morphology:

Module Grayscale Morphology
============================

.. automodule:: plugins.morphology
   :members:

.. _ref_package_averaging:

Module Averaging
==================

.. automodule:: plugins.averaging
   :members:

.. _ref_package_h_transform:

Module H-transform
===================

.. automodule:: plugins.h_transform
   :members:

.. _ref_package_region_labeling:

Module Region Labeling
========================

.. automodule:: plugins.region_labeling
   :members:

.. _ref_package_segmentation:

Module Segmentation
===================

.. automodule:: plugins.segmentation
   :members:

.. _ref_package_labels_post_processing:

Module Labels Post Processing
=============================

.. automodule:: plugins.labels_post_processing
   :members:

.. _ref_package_registration:

Module Registration
====================

.. automodule:: plugins.registration
   :members:

.. _ref_package_sequence_registration:

Module Sequence registration
=============================

.. automodule:: plugins.sequence_registration
   :members: