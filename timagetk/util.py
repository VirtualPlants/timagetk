# -*- coding: utf-8 -*-
# -*- python -*-
#
#
#       Copyright 2016 INRIA
#
#       File author(s):
#           Guillaume Baty <guillaume.baty@inria.fr>
#           Sophie Ribes <sophie.ribes@inria.fr>
#           Jonathan Legrand <jonathan.legrand@ens-lyon.fr>
#
#       See accompanying file LICENSE.txt
# ------------------------------------------------------------------------------

import os

_ROOT = os.path.abspath(os.path.dirname(__file__))

import warnings
from timagetk.components import SpatialImage


def shared_folder():
    return os.path.join(_ROOT, 'share', 'data')


def data_path(filename):
    tmp = shared_folder()
    return os.path.join(tmp, str(filename))


def clean_warning(message, category, filename, lineno, file=None, line=None):
    return '%s: %s\n' % (category.__name__, message)


# TODO: write a `_connecitity_check` function working the same way than `_method_check`
def _method_check(method, valid_methods, default_index=0):
    """
    Check if a given string 'method' is valid and

    Parameters
    ----------
    method: str|None
        name of a method to check, if None returns valid_methods[default_index]
        as th default method
    valid_methods: list
        list of valid methods
    default_index: int, optional
        index of the deafault method to use in the list of 'valid_mathods'

    Returns
    -------
    methods: str
        a valid method
    """
    # - Set the default method if None:
    if method is None:
        method = valid_methods[default_index]
    # - Test the given method is valid:
    else:
        try:
            assert method in valid_methods
        except AssertionError:
            msg = "Unknown method '{}',".format(method)
            msg += "available methods are: {}".format(valid_methods)
            raise NotImplementedError(msg)

    return method


def _input_img_check(input_image, real=False):
    """
    Used to check `input_image` type and method units.
    If not real, check that the given input image is isometric.

    Parameters
    ----------
    input_image : SpatialImage
        tested input type
    real : bool, optional
        indicate if the method works on real or voxel units
    """
    # TODO: use function name for better 'warnings'
    # - Check the `input_image` is indeed a `SpatialImage`
    try:
        assert isinstance(input_image, SpatialImage)
    except AssertionError:
        raise TypeError('Input image must be a SpatialImage instance!')

    # - Check the isometry of the image when using voxel units:
    if not real and not input_image.is_isometric():
        warnings.formatwarning = clean_warning
        warnings.simplefilter("always")
        msg = "The image is NOT isometric, this method operates on voxels!"
        warnings.warn(msg)
    return


def _general_kwargs(param=False, verbose=False, time=False, debug=False, **kwargs):
    """
    Keyword argument parser for VT general parameters.

    Parameters
    ----------
    param: bool, optional
        if True, default False, print the used parameters
    verbose: bool, optional
        if True, default False, increase code verbosity
    time: bool, optional
        if True, default False, print the CPU & USER elapsed time
    debug: bool, optional
        if True, default False, print the debug parameters

    Returns
    -------
    str_param: str
        VT formatted general parameters
    """
    str_param = ""
    # - Providing 'param=True' will result in printing parameters
    if param:
        str_param += ' -param'
    # - Providing 'verbose=True' will result in increased verbosity of the code
    if verbose:
        str_param += ' -verbose'
    else:
        str_param += ' -noverbose'
    # - Providing 'time=True' will result in printing CPU & User elapsed time
    if time:
        str_param += ' -time'
    else:
        str_param += ' -notime'
    # - Providing 'debug=True' will result in printing debug log
    if debug:
        str_param += ' -debug'
    else:
        str_param += ' -nodebug'

    return str_param


PARALLEL_TYPE = ['openmp', 'omp', 'pthread', 'thread']
DEFAULT_PARALLEL = "default"
OMP_TYPE = ['default', 'static', 'dynamic-one', 'dynamic', 'guided']


def _parallel_kwargs(parallel=True, parallel_type=DEFAULT_PARALLEL, n_job=None,
                   omp_scheduling=None, **kwargs):
    """
    Keyword argument parser for VT parallelism parameters.

    Parameters
    ----------
    parallel: bool, optional
        if True (default), use parallelism
    parallel_type: str, optional
        type of parallelisation to use, can be in ['openmp', 'omp', 'pthread', 'thread']
    n_job: int, optional
        number of core to use in parallel, by default use the maximum number of
        available CPU cores
    omp_scheduling: str, optional
        change 'OpenMP' scheduling option (if available), can be in ['default',
        'static', 'dynamic-one', 'dynamic', 'guided']

    Returns
    -------
    str_param: str
        VT formatted parallelism parameters
    """
    str_param = ""
    # - Check OMP-scheduling:
    if omp_scheduling is not None and parallel_type != 'openmp' or parallel_type != 'omp':
        parallel_type = 'openmp'
    # - Providing 'parallel=True' will result in using parallelism options:
    if parallel:
        str_param += ' -parallel'
        str_param += ' -parallel-type %s' % parallel_type
        if n_job is not None:
            str_param += ' -max-chunks %d' % n_job
        if omp_scheduling is not None:
            str_param += ' -omp-scheduling %s' % omp_scheduling
    else:
        str_param += ' -no-parallel'

    return str_param
