import 'package:flutter_blue_plus/flutter_blue_plus.dart';
import 'package:flutter_riverpod/flutter_riverpod.dart';

class BleServicesProvider extends StateNotifier<List<BluetoothService>> {
  BleServicesProvider() : super([]);

  void set(List<BluetoothService> services) {
    state = services;
  }
}

final bleServicesProvider =
    StateNotifierProvider<BleServicesProvider, List<BluetoothService>>((ref) {
      return BleServicesProvider();
    });
