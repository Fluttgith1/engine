#!/usr/bin/env python
# Copyright 2019 The Flutter Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import argparse
import os
import sys
import json

# Path constants. (All of these should be absolute paths.)
THIS_DIR = os.path.abspath(os.path.dirname(__file__))
FLUTTER_DIR = os.path.abspath(os.path.join(THIS_DIR, '..', '..', '..'))
INSTALL_DIR = os.path.join(FLUTTER_DIR, 'third_party', 'android_support')

# The template for the POM file.
POM_FILE_CONTENT = '''
<?xml version="1.0" encoding="UTF-8"?>
<project xsi:schemaLocation="http://maven.apache.org/POM/4.0.0 http://maven.apache.org/xsd/maven-4.0.0.xsd" xmlns="http://maven.apache.org/POM/4.0.0"
    xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance">
  <modelVersion>4.0.0</modelVersion>
  <groupId>io.flutter</groupId>
  <artifactId>{0}</artifactId>
  <version>{1}</version>
  <packaging>jar</packaging>
  <dependencies>
    {2}
  </dependencies>
</project>
'''

POM_DEPENDENCY = '''
    <dependency>
      <groupId>{0}</groupId>
      <artifactId>{1}</artifactId>
      <version>{2}</version>
      <scope>compile</scope>
    </dependency>
'''

def main():
  with open (os.path.join(THIS_DIR, 'files.json')) as f:
    dependencies = json.load(f)

  parser = argparse.ArgumentParser(description='Generate the POM file for the engine artifacts')
  parser.add_argument('--engine-artifact-id', type=str, required=True)
  parser.add_argument('--engine-version', type=str, required=True)

  args = parser.parse_args()
  engine_artifact_id = args.engine_artifact_id
  engine_version = args.engine_version
  artifact_version = '1.0.0-' + engine_version
  out_file_name = '%s-%s.pom' % (engine_artifact_id, engine_version)

  pom_dependencies = ''
  for dependency in dependencies:
    group_id, artifact_id, version = dependency['maven_dependency'].split(':')
    pom_dependencies += POM_DEPENDENCY.format(group_id, artifact_id, version)

  # Write the POM file.
  with open(os.path.join(INSTALL_DIR, out_file_name), 'w') as f:
    f.write(POM_FILE_CONTENT.format(engine_artifact_id, engine_version, pom_dependencies))

if __name__ == '__main__':
  sys.exit(main())
