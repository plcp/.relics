# -*- coding: utf-8 -*-
import fenestre as ft

import struct
import inspect

def packets(protocol, max_size=65536, header=b'\xfe\xfe\xfe\xfe'):
    import fenestre.protocols.details as etc

    assert inspect.isclass(protocol)
    class _packets_details(etc.packets):
        pass

    _packets_details._protocol_class = protocol
    _packets_details.max_size = max_size
    _packets_details.header = header

    return _packets_details

def sep(protocol, separator=b'\r\n', max_size=65536):
    import fenestre.protocols.details as etc

    assert inspect.isclass(protocol)
    class _sep_details(etc.sep):
        pass

    _sep_details._protocol_class = protocol
    _sep_details.separator = separator
    _sep_details.max_size = max_size

    return _sep_details
