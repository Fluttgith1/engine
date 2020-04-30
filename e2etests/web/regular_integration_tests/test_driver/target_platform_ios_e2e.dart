// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:html';
import 'package:flutter/services.dart';
import 'package:flutter_test/flutter_test.dart';
import 'package:regular_integration_tests/text_editing_main.dart' as app;
import 'package:flutter/material.dart';

import 'package:e2e/e2e.dart';

void main() {
  E2EWidgetsFlutterBinding.ensureInitialized() as E2EWidgetsFlutterBinding;

  testWidgets('Focused text field creates a native input element',
          (WidgetTester tester) async {
        app.main();
        await tester.pumpAndSettle();

        final Finder macOSFinder = find.byKey(const Key('macOSKey'));
        expect(macOSFinder, findsNothing);

        final Finder iOSFinder = find.byKey(const Key('iOSKey'));
        expect(iOSFinder, findsOneWidget);

        final Finder androidFinder = find.byKey(const Key('androidKey'));
        expect(androidFinder, findsNothing);
      });
}
