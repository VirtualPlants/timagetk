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

import os
_ROOT = os.path.abspath(os.path.dirname(__file__))

def shared_folder():
    return os.path.join(_ROOT, 'share', 'data')

def data_path(filename):
    tmp = shared_folder()
    return os.path.join(tmp, str(filename))