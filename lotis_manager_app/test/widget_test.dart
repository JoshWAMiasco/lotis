// This is a basic Flutter widget test.
//
// To perform an interaction with a widget in your test, use the WidgetTester
// utility in the flutter_test package. For example, you can send tap and scroll
// gestures. You can also use WidgetTester to find child widgets in the widget
// tree, read text, and verify that the values of widget properties are correct.

import 'package:flutter_test/flutter_test.dart';
import 'package:lotis_manager_app/data/egg.dart';

void main() {
  test("Data", () {
    final egg = Egg(quantity: 0);
    egg.increaseByNumKeypad(3);
    print(egg.display);
    egg.increaseByNumKeypad(9);
    print(egg.display);
  });
}
