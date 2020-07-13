#!/usr/bin/env python
#
# Copyright 2013 The Flutter Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
""" Merges the debug symbols and uploads them to cipd.
"""

import argparse
import collections
import json
import os
import platform
import shutil
import subprocess
import sys
import tarfile


def IsLinux():
  return platform.system() == 'Linux'


# out_dir here is of the format "/b/s/w/ir/k/recipe_cleanup/tmpIbWDdp"
# we need to palce the cipd definition in this directory.
def GetPackagingDir(out_dir):
  return os.path.abspath(out_dir)


def CreateCIPDDefinition(target_arch, out_dir, symbol_dirs):
  dir_name = os.path.basename(os.path.normpath(out_dir))
  pkg_def = """
package: flutter/fuchsia-debug-symbols-%s
description: Flutter and Dart runner debug symbols for Fuchsia. Target architecture %s.
install_mode: copy
data:
""" % (target_arch, target_arch)
  for symbol_dir in symbol_dirs:
    symbol_dir_name = os.path.basename(os.path.normpath(symbol_dir))
    data = '\n  - dir: %s' % (symbol_dir_name)
    pkg_def = pkg_def + data
  return pkg_def


# CIPD CLI needs the definition and data directory to be relative to each other.
def WriteCIPDDefinition(target_arch, out_dir, symbol_dirs):
  _packaging_dir = GetPackagingDir(out_dir)
  yaml_file = os.path.join(_packaging_dir, 'debug_symbols.cipd.yaml')
  with open(yaml_file, 'w') as f:
    cipd_def = CreateCIPDDefinition(target_arch, out_dir, symbol_dirs)
    f.write(cipd_def)
  return yaml_file


def CheckCIPDPackageExists(package_name, tag):
  '''Check to see if the current package/tag combo has been published'''
  command = [
    'cipd',
    'search',
    package_name,
    '-tag',
    tag,
  ]
  stdout = subprocess.check_output(command)
  match = re.search(r'No matching instances\.', stdout)
  if match:
    return False
  else:
    return True


def ProcessCIPDPackage(upload, cipd_yaml, engine_version, out_dir, target_arch):
  _packaging_dir = GetPackagingDir(out_dir)
  tag = 'git_revision:%s' % engine_version
  already_exists = CheckCIPDPackageExists(
    'flutter/fuchsia-debug-symbols-%s' % target_arch,
    tag)
  if already_exists:
    print('CIPD package already exists!')

  if upload and IsLinux() and not already_exists:
    command = [
        'cipd', 'create', '-pkg-def', cipd_yaml, '-ref', 'latest', '-tag',
        'git_revision:%s' % engine_version
    ]
  else:
    command = [
        'cipd', 'pkg-build', '-pkg-def', cipd_yaml, '-out',
        os.path.join(_packaging_dir,
                     'fuchsia-debug-symbols-%s.cipd' % target_arch)
    ]

  # Retry up to three times.  We've seen CIPD fail on verification in some
  # instances. Normally verification takes slightly more than 1 minute when
  # it succeeds.
  num_tries = 3
  for tries in range(num_tries):
    try:
      subprocess.check_call(command, cwd=_packaging_dir)
      break
    except subprocess.CalledProcessError as error:
      print('Failed %s times.\nError was: %s' % (tries + 1, error))
      if tries == num_tries - 1:
        raise

# Recursively hardlinks contents from one directory to another,
# skipping over collisions.
def HardlinkContents(dirA, dirB):
  internal_symbol_dirs = []
  for src_dir, _, filenames in os.walk(dirA):
    for filename in filenames:
      # if a file contains 'dbg_success' in its name, it is a stamp file.
      # An example of this would be
      # '._dart_jit_runner_dbg_symbols_unstripped_dbg_success' these
      # are generated by GN and have to be ignored.
      if 'dbg_success' in filename:
        continue
      src = os.path.join(src_dir, filename)
      dest_dir = os.path.join(dirB, os.path.relpath(src_dir, dirA))
      try:
        os.makedirs(dest_dir)
        internal_symbol_dirs.append(dest_dir)
      except:
        pass
      dest = os.path.join(dest_dir, filename)
      if os.path.exists(dest):
        # The last two path components provide a content address for a .build-id entry.
        tokens = os.path.split(dest)
        name = os.path.join(tokens[-2], tokens[-1])
        print('%s already exists in destination; skipping linking' % name)
        continue
      os.link(src, dest)
  return internal_symbol_dirs

def main():
  parser = argparse.ArgumentParser()

  parser.add_argument(
      '--symbol-dirs',
      required=True,
      nargs='+',
      help='Space separated list of directories that contain the debug symbols.'
  )
  parser.add_argument(
      '--out-dir',
      required=True,
      action='store',
      dest='out_dir',
      help='Output directory where the executables will be placed.')
  parser.add_argument(
      '--target-arch', type=str, choices=['x64', 'arm64'], required=True)
  parser.add_argument(
      '--engine-version',
      required=True,
      help='Specifies the flutter engine SHA.')

  parser.add_argument('--upload', default=False, action='store_true')

  args = parser.parse_args()

  symbol_dirs = args.symbol_dirs
  for symbol_dir in symbol_dirs:
    assert os.path.exists(symbol_dir) and os.path.isdir(symbol_dir)

  out_dir = args.out_dir

  if os.path.exists(out_dir):
    print 'Directory: %s is not empty, deleting it.' % out_dir
    shutil.rmtree(out_dir)
  os.makedirs(out_dir)

  internal_symbol_dirs = []
  for symbol_dir in symbol_dirs:
    internal_symbol_dirs += HardlinkContents(symbol_dir, out_dir)

  # make these unique
  internal_symbol_dirs = list(set(internal_symbol_dirs))

  arch = args.target_arch
  cipd_def = WriteCIPDDefinition(arch, out_dir, internal_symbol_dirs)
  ProcessCIPDPackage(args.upload, cipd_def, args.engine_version, out_dir, arch)
  return 0


if __name__ == '__main__':
  sys.exit(main())
