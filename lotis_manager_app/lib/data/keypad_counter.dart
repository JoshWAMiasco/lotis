mixin KeypadCounter {
  int keypadInput(int number, int currentValue, {int? maxValue}) {
    int newValue = (currentValue * 10) + number;
    if (maxValue != null) {
      if (maxValue > newValue) {
        return newValue;
      } else {
        return currentValue;
      }
    } else {
      return newValue;
    }
  }

  double keypadInputDecimal(
    int number,
    double currentValue, {
    double? maxValue,
  }) {
    int currentCents = (currentValue * 100).round();
    int newCents = (currentCents * 10) + number;
    double newValue = newCents / 100.0;

    if (maxValue != null && newValue > maxValue) {
      return currentValue;
    }

    return newValue;
  }

  int keypadBackspace(int currentValue) {
    if (currentValue <= 0) {
      return 0;
    }
    return (currentValue / 10).toInt();
  }

  double keypadBackspaceDecimal(double currentValue) {
    int currentCents = (currentValue * 100).round();
    if (currentCents <= 0) {
      return 0.0;
    }

    int newCents = currentCents ~/ 10;

    return newCents / 100.0;
  }
}
