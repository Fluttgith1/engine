#!/usr/bin/env python3
# Copyright 2013 The Flutter Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

'''
Sets up githooks.
'''

import os
import subprocess
import sys


SRC_ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__)))))
FLUTTER_DIR = os.path.join(SRC_ROOT, 'flutter')


def Main(argv):
  result = subprocess.run([
    'git',
    'config',
    'core.hooksPath',
    os.path.join(FLUTTER_DIR, 'tools', 'githooks'),
  ], cwd=FLUTTER_DIR)
  return result.returncode


if __name__ == '__main__':
    sys.exit(Main(sys.argv))
