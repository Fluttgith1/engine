#!/usr/bin/env python3
#
# Copyright 2013 The Flutter Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""Generates a shell or batch script to run a command."""

import argparse
import os
import string

def main():
  parser = argparse.ArgumentParser(description=__doc__)
  parser.add_argument('--output', required=True, help='Output file')
  parser.add_argument('--command', required=True, help='Command to run')
  parser.add_argument('--cwd', required=False, help='Working directory')
  parser.add_argument('rest', nargs='*', help='Arguments to pass to the command')
  
  # Rest of the arguments are passed to the command.
  args = parser.parse_args()

  out_path = os.path.dirname(args.output)
  if not os.path.exists(out_path):
    os.makedirs(out_path)

  script = string.Template('''#!/bin/sh

set -e

# Needed because if it is set, cd may print the path it changed to.
unset CDPATH

# Store the current working directory.
prev_cwd=$$(pwd)

# Set a trap to restore the working directory.
trap 'cd "$$prev_cwd"' EXIT

CD_PATH="$cwd"
if [ -n "$$CD_PATH" ]; then
  cd "$$CD_PATH"
fi

$command "$args"
''')

  # Convert args into an escaped string.
  escaped = [arg.replace('"', '\\"') for arg in args.rest]

  params = {
    'command': args.command,
    'args': '" "'.join(escaped),
    'cwd': args.cwd if args.cwd else '',
  }

  with open(args.output, 'w') as f:
    f.write(script.substitute(params))

  # Make the script executable.
  os.chmod(args.output, 0o755)

if __name__ == '__main__':
  main()
