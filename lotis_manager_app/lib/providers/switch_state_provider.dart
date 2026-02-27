import 'package:flutter_riverpod/flutter_riverpod.dart';

class SwitchStateProvider extends StateNotifier<List<int>> {
  SwitchStateProvider() : super([]);

  void update(List<int> newValues) {
    state = newValues;
  }

  void toggle(int index) {
    List<int> updated = state;
    updated[index] = updated[index] == 1 ? 0 : 1;
    state = updated;
  }
}

final switchStateProvider =
    StateNotifierProvider<SwitchStateProvider, List<int>>((ref) {
      return SwitchStateProvider();
    });
