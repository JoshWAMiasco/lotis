import 'package:intl/intl.dart';
import 'package:lotis_manager_app/data/keypad_counter.dart';

class Price with KeypadCounter {
  Price({this.value = 0.0});
  double value;

  String get display => NumberFormat("###,###,###.00").format(value);

  void increaseByKeypad(int number) {
    value = keypadInputDecimal(number, value, maxValue: 999_999_999.00);
  }

  void decreaseByKeypad() {
    value = keypadBackspaceDecimal(value);
  }
}
