.. _ref_installation:

*************
Installation
*************

Requirements
************

* python 2.7 (to check the version, open a shell prompt and type: ``python --version``)
* `pip <https://pypi.python.org/pypi/pip>`_ and setuptools (extensions for python package installation)

* `scons <http://scons.org/>`_ (build tool)
* `zlib <http://www.zlib.net/>`_ (compression/decompression)
* `numpy <http://www.numpy.org/>`_ (array support for python)
* `networkx <https://networkx.github.io/>`_ (data structures for graphs)

To install pip, setuptools, scons, zlib and numpy:

* **Ubuntu and Debian:**
  ``sudo apt-get install python-pip python-setuptools scons libz-dev python-numpy``

* **Fedora and Redhat:**
  ``sudo yum install python-pip python-setuptools scons zlib-devel numpy``

* **Mac** (using `Macports <https://www.macports.org/>`_):
  ``sudo port install py27-pip py27-setuptools scons zlib-devel py27-numpy``

To install networks (all platforms): ``sudo pip install networkx``

Installing from source
**********************

Github
======

If you are interested in contributing to development or running the latest source code,
grab the git version. First, install the requirements and the development tools:

* **Ubuntu & Debian:** ``sudo apt-get install git gitk``

* **Fedora & Redhat:** ``sudo yum install git gitk``

* **Mac**: ``sudo port install git-core +doc +bash_completion +gitweb``

To install `nose <http://nose.readthedocs.io/en/latest/>`_ (testing, all platforms): ``sudo pip install nose``

Then:

#. Clone the timagetk repository ``git clone https://github.com/VirtualPlants/timagetk.git``
#. Change directory to timagetk
#. Run ``python setup.py develop --user``
#. Check that timagetk has been added to your .bashrc file
#. If it is not the case, add the following lines to the bottom of your .bashrc file:
    * timagetk_path=/path/to/timagetk/folder
    * export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:${timagetk_path}/timagetk/build-scons/lib
#. Open a new shell prompt
#. Type ``nosetests -v`` to execute the tests


*************
Documentation
*************

To build **timagetk**'s dynamic documentation (`sphinx <http://www.sphinx-doc.org/en/stable/>`_), open a shell prompt and type:

* ``sudo pip install -U Sphinx``

Go to the **timagetk/timagetk/doc/** folder and type:

* ``make html``

Open the file: **timagetk/timagetk/build/html/index.html**