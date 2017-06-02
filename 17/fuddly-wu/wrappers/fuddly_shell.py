#!/usr/bin/env python

import os, sys
v = str(sys.version_info[0])
os.execvp("bash", [
    'bash',
    '-c',
    (''
        + 'source ./fuddly/venv' + v + '/bin/activate ;'
        + 'HOME="$(pwd)" exec python' + v + ' ./fuddly/fuddly_shell_wrapped.py ' + ' '.join(sys.argv[1:])
        ),
    ])

