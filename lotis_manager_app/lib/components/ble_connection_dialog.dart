import 'dart:async';
import 'dart:developer';
import 'dart:io';

import 'package:flutter/foundation.dart';
import 'package:flutter/material.dart';
import 'package:flutter_blue_plus/flutter_blue_plus.dart';
import 'package:flutter_riverpod/flutter_riverpod.dart';
import 'package:lotis_manager_app/components/primary_snackbar.dart';
import 'package:lotis_manager_app/constants/app_color.dart';
import 'package:lotis_manager_app/constants/app_font.dart';
import 'package:lotis_manager_app/providers/ble_device_provider.dart';
import 'package:lotis_manager_app/providers/ble_services_provider.dart';
import 'package:lotis_manager_app/services/ble_service.dart';
import 'package:permission_handler/permission_handler.dart';

class BleConnectionDialog extends StatefulWidget {
  const BleConnectionDialog({super.key});

  @override
  State<BleConnectionDialog> createState() => _BleConnectionDialogState();
}

class _BleConnectionDialogState extends State<BleConnectionDialog> {
  final bleService = BleService();

  StreamSubscription? _scanSubscription;
  List<BluetoothDevice> devices = [];
  List<String> devicesMacAddress = [];

  @override
  void initState() {
    super.initState();
    WidgetsBinding.instance.addPostFrameCallback((_) async {
      try {
        if (await Permission.bluetooth.isGranted == false) {
          await Permission.bluetooth.request();
        }
        if (await FlutterBluePlus.isSupported == false) {
          log("Bluetooth not supported by this device");
          return;
        }
        var subscription = FlutterBluePlus.adapterState.listen((
          BluetoothAdapterState state,
        ) {
          print(state);
          if (state == BluetoothAdapterState.on) {
            // usually start scanning, connecting, etc
          } else {
            // show an error to the user, etc
          }
        });

        // turn on bluetooth ourself if we can
        // for iOS, the user controls bluetooth enable/disable
        if (!kIsWeb && Platform.isAndroid) {
          await FlutterBluePlus.turnOn();
        }

        // cancel to prevent duplicate listeners
        subscription.cancel();
        _scanSubscription = FlutterBluePlus.scanResults.listen((scanResults) {
          for (var res in scanResults) {
            String macAddress = res.device.remoteId.str;
            log("Device: ${res.device.advName} | $macAddress");
            if (devicesMacAddress.contains(macAddress) == false &&
                res.device.advName.toLowerCase() == "lotis") {
              setState(() {
                devices.add(res.device);
                devicesMacAddress.add(macAddress);
              });
            }
          }
        });

        await FlutterBluePlus.startScan(
          timeout: const Duration(seconds: 10),
          androidScanMode: AndroidScanMode.balanced,
        );
        await FlutterBluePlus.isScanning.where((val) => val == false).first;
        FlutterBluePlus.cancelWhenScanComplete(_scanSubscription!);
      } catch (e) {
        PrimarySnackbar.showInfo(context, "Error Bluetooth");
      }
    });
  }

  @override
  Widget build(BuildContext context) {
    return AlertDialog(
      scrollable: true,
      shape: RoundedRectangleBorder(borderRadius: .circular(10)),
      title: Text('Connect'),
      content: Builder(
        builder: (context) {
          if (devices.isNotEmpty) {
            return Consumer(
              builder: (context, ref, child) {
                return Column(
                  spacing: 10,
                  children: List.generate(devices.length, (index) {
                    return GestureDetector(
                      onTap: () async {
                        await FlutterBluePlus.stopScan();
                        try {
                          Navigator.of(context).pop(devices[index]);
                        } catch (e) {
                          if (!mounted) return;
                          Navigator.of(context).pop(null);
                        }
                      },
                      child: SizedBox(
                        width: MediaQuery.of(context).size.width * 0.4,
                        child: Card(
                          child: Padding(
                            padding: EdgeInsets.all(10),
                            child: Row(
                              mainAxisAlignment: .spaceBetween,
                              children: [
                                Column(
                                  crossAxisAlignment: .start,
                                  mainAxisSize: .min,
                                  children: [
                                    Text(
                                      devices[index].advName,
                                      style: AppFont.h3DarkBold().copyWith(
                                        color: AppColor.darkGreen,
                                      ),
                                    ),
                                    Text(
                                      devices[index].remoteId.str,
                                      style: AppFont.h5DarkRegular(),
                                    ),
                                  ],
                                ),
                                Icon(Icons.bluetooth, color: Colors.blue),
                              ],
                            ),
                          ),
                        ),
                      ),
                    );
                  }),
                );
              },
            );
          }

          return Column(
            mainAxisSize: .min,
            spacing: 20,
            children: [
              Text(
                'Scanning for LoTIS Devices...',
                style: AppFont.h5DarkRegular(),
              ),
              CircularProgressIndicator(
                color: Colors.transparent,
                valueColor: AlwaysStoppedAnimation(AppColor.darkGreen),
              ),
            ],
          );
        },
      ),
      actions: [
        FilledButton(
          onPressed: () {
            Navigator.of(context).pop();
          },
          child: Text('Close'),
        ),
      ],
    );
  }
}
