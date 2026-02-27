import 'package:intl/intl.dart';
import 'package:lotis_manager_app/data/keypad_counter.dart';
import 'package:lotis_manager_app/data/agriproducts.dart';

class Egg extends AgriProducts with KeypadCounter {
  Egg({int quantity = 0}) : super(quantity, "Egg");

  void increaseByNumKeypad(int number, {int? maxValue}) {
    int value = keypadInput(number, quantity, maxValue: maxValue);
    quantity = value;
  }

  void decreaseByBackspaceKeypad() {
    quantity = keypadBackspace(quantity);
  }

  String get display => NumberFormat("###,###,###").format(quantity);

  @override
  Future<void> harvest() {
    // TODO: implement harvest
    throw UnimplementedError();
  }

  @override
  Future<void> sell() {
    // TODO: implement sell
    throw UnimplementedError();
  }
}
