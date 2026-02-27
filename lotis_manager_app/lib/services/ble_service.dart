import 'dart:async';
import 'dart:developer';

import 'package:flutter_blue_plus/flutter_blue_plus.dart';
import 'package:lotis_manager_app/constants/app_utils.dart';

class BleService {
  BluetoothDevice? connectedDevice;
  List<BluetoothService> listOfServices = [];

  Future<void> init() async {
    // Initialize Bluetooth service
  }

  Future<List<ScanResult>> scanDevices({
    Duration timeout = const Duration(seconds: 4),
  }) async {
    List<ScanResult> results = [];

    // Start scanning
    StreamSubscription? _subs;

    // Listen to scan results
    _subs = FlutterBluePlus.scanResults.listen((scanResults) {
      results = scanResults;
      for (var res in results) {
        log("Device: ${res.device.advName} | ${res.device.remoteId.str}");
      }
    });

    await FlutterBluePlus.startScan(
      timeout: timeout,
      androidScanMode: AndroidScanMode.balanced,
    );
    await FlutterBluePlus.isScanning.where((val) => val == false).first;
    FlutterBluePlus.cancelWhenScanComplete(_subs);

    return results;
  }

  Future<void> connectToDevice(BluetoothDevice device) async {
    try {
      await device.connect();
      connectedDevice = device;
    } catch (e) {
      throw Exception('Failed to connect to device: $e');
    }
  }

  Future<void> disconnectDevice() async {
    if (connectedDevice != null) {
      await connectedDevice!.disconnect();
      connectedDevice = null;
    }
  }

  Future<String> readResponse() async {
    if (connectedDevice == null) {
      throw Exception('No device connected');
    }

    // Implement BLE response reading logic
    // This will depend on the specific service and characteristic UUIDs
    return '';
  }

  bool isConnected() {
    return connectedDevice != null;
  }

  void setup(BluetoothDevice device, List<BluetoothService> services) {
    connectedDevice = device;
    listOfServices = services;
  }

  void clear() {
    connectedDevice = null;
    listOfServices = [];
  }

  BluetoothCharacteristic? getNotifier(String serviceUid, String charUid) {
    BluetoothCharacteristic? char;
    for (var service in listOfServices) {
      if (service.uuid.str.toLowerCase() == serviceUid) {
        for (var charac in service.characteristics) {
          if (charac.uuid.str == charUid) {
            char = charac;
            break;
          }
        }
      }
    }
    return char;
  }

  Future<void> sendCommand(String command, {int timeout = 15}) async {
    try {
      BluetoothCharacteristic? char;
      for (var service in listOfServices) {
        if (service.uuid.str.toLowerCase() ==
            "00000030-1212-efde-1523-785feabcd122") {
          for (var charac in service.characteristics) {
            if (charac.uuid.str.toLowerCase() ==
                "00001525-1212-efde-1523-785feabcd124") {
              char = charac;
              break;
            }
          }
        }
      }
      if (char != null) {
        List<int> bytes = AppUtils.hexToBytes(command);
        await char.write(bytes, withoutResponse: false, timeout: timeout);
      }
    } catch (e) {
      throw "Failed to send command";
    }
  }
}
