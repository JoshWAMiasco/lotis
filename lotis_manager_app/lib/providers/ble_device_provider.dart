import 'package:flutter_blue_plus/flutter_blue_plus.dart';
import 'package:flutter_riverpod/flutter_riverpod.dart';

class BleDeviceProvider extends StateNotifier<BluetoothDevice?> {
  BleDeviceProvider() : super(null);

  void set(BluetoothDevice? newDevice) {
    state = newDevice;
  }
}

final bleDeviceProvider =
    StateNotifierProvider<BleDeviceProvider, BluetoothDevice?>((ref) {
      return BleDeviceProvider();
    });
