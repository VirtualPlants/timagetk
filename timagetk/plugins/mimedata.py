# -*- coding: utf-8 -*-
# -*- python -*-
#
#
#       Copyright 2016 INRIA
#
#       File author(s):
#           Guillaume Cerutti <guillaume.cerutti@inria.fr>
#           Guillaume Baty <guillaume.baty@inria.fr>
#           Sophie Ribes <sophie.ribes@inria.fr>
#           Gregoire Malandain <gregoire.malandain@inria.fr>
#
#       See accompanying file LICENSE.txt
#------------------------------------------------------------------------------

import mimetypes
from openalea.core.path import path

from openalea.oalab.mimedata.qcodec import QMimeCodec
from openalea.oalab.mimedata.exception import MimeConversionError
from openalea.oalab.mimedata.builtin import BuiltinDataCodec

from timagetk.components import imread


def read_image_path(urls, mimetype_in, mimetype_out):

    if isinstance(urls, basestring):
        urls = [urls]
    for url in urls:
        url = path(url)
        if url.exists():
            kwds = {}
            try:
                print  "TimageTK imread!"
                data = imread(url)
                kwds['name'] = url.namebase
                if url.ext == ".gz":
                    kwds['name'] = url.stripext().namebase
            except Exception, e:
                e = MimeConversionError(url, mimetype_in, mimetype_out, e)
                raise e
            else:
                return data, kwds
    return None, {}


def is_image_path(path):
    mime, encoding = mimetypes.guess_type(path)
    # if mime and mime.startswith('image'):
    #     return True
    # elif path.ext in ('.lsm', '.inr'):
    if path.ext in ('.inr', '.tif', '.tiff'):
        return True
    elif path.ext == '.gz' and path.stripext().ext == '.inr':
        return True
    else:
        return False


class IImageCodec(QMimeCodec):

    def _raw_data(self, mimedata, mimetype_in, mimetype_out):
        """
        'text/uri-list' -> list of paths
        'openalealab/data' -> name
        """

        if mimetype_in == 'text/uri-list':
            return [path(url.toLocalFile()) for url in mimedata.urls()]
        elif mimetype_in == 'openalealab/data':
            return mimedata.data('openalealab/data')

    def quick_check(self, mimedata, mimetype_in, mimetype_out):
        raw_data = self._raw_data(mimedata, mimetype_in, mimetype_out)
        if not raw_data:
            return False
        if mimetype_in == 'openalealab/data':
            data = BuiltinDataCodec().decode(raw_data, mimetype_in, mimetype_out)
            url = data.path
        elif mimetype_in == 'text/uri-list':
            url = raw_data[0]
        else:
            return False

        return is_image_path(url)

    def qtdecode(self, mimedata, mimetype_in, mimetype_out):
        raw_data = self._raw_data(mimedata, mimetype_in, mimetype_out)
        if raw_data is None:
            return None, {}
        else:
            return self.decode(raw_data, mimetype_in, mimetype_out)

    def decode(self, raw_data, mimetype_in, mimetype_out, **kwds):
        if mimetype_in == 'text/uri-list':
            if mimetype_out == 'openalea/interface.IImage':
                return read_image_path(raw_data, mimetype_in, mimetype_out)
        elif mimetype_in == 'openalealab/data':
            data = BuiltinDataCodec().decode(raw_data, mimetype_in, mimetype_out)
            if mimetype_out == 'openalea/interface.IImage':
                return read_image_path(data.path, mimetype_in, mimetype_out)
        return None, {}
