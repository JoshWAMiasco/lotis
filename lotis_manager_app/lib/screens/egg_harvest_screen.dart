import 'dart:async';
import 'dart:developer';

import 'package:flutter/material.dart';
import 'package:flutter_blue_plus/flutter_blue_plus.dart';
import 'package:flutter_riverpod/flutter_riverpod.dart';
import 'package:lotis_manager_app/app_instance.dart';
import 'package:lotis_manager_app/components/primary_snackbar.dart';
import 'package:lotis_manager_app/data/egg.dart';
import 'package:lotis_manager_app/constants/app_color.dart';
import 'package:lotis_manager_app/constants/app_font.dart';
import 'package:lotis_manager_app/constants/app_image.dart';
import 'package:lotis_manager_app/constants/app_utils.dart';
import 'package:lotis_manager_app/providers/ble_device_provider.dart';
import 'package:responsive_framework/responsive_framework.dart';

class EggHarvestScreen extends ConsumerStatefulWidget {
  const EggHarvestScreen({super.key});

  @override
  ConsumerState<EggHarvestScreen> createState() => _EggHarvestScreenState();
}

class _EggHarvestScreenState extends ConsumerState<EggHarvestScreen> {
  final keypad = ['1', '2', '3', '4', '5', '6', '7', '8', '9', '*', '0', '#'];
  bool isTablet = false;
  Egg goodEgg = Egg(quantity: 0);
  Egg crackEgg = Egg(quantity: 0);
  String goodEggType = "good";
  String crackEggType = "crack";
  String? currentSelectedType;
  bool isLoading = false;
  StreamSubscription? _dataSubscription;

  Future<void> startDataListener() async {
    try {
      BluetoothCharacteristic? notifierCharac = bluetoothService.getNotifier(
        "00000020-1212-efde-1523-785feabcd121",
        "00000021-1212-efde-1523-785feabcd121",
      );
      if (notifierCharac == null) {
        PrimarySnackbar.showInfo(context, "Failed to Ble Listener");
      }
      _dataSubscription = notifierCharac!.onValueReceived.listen((raw) {
        log("Ble Raw: $raw");
        String hex = AppUtils.bytesToHex(raw);
        log("Ble Hex $hex");
        if (hex.contains("4C54")) {
          String command = AppUtils.byteToHex(raw[2]);
          String commandType = AppUtils.byteToHex(raw[3]);
          log("Command $command");
          log("Type $commandType");
          switch (command) {
            case '49':
              if (commandType == '03') {
                String commandStatus = AppUtils.byteToHex(raw[4]);
                setState(() {
                  isLoading = false;
                });
                if (commandStatus == '00') {
                  PrimarySnackbar.showInfo(context, "Successfully Recorded");
                } else {
                  PrimarySnackbar.showInfo(context, "Record Failed!");
                }
              }

            default:
          }
        }
      });

      ref.read(bleDeviceProvider)!.cancelWhenDisconnected(_dataSubscription!);

      await notifierCharac.setNotifyValue(true);
      await Future.delayed(const Duration(seconds: 1));
    } catch (e) {
      PrimarySnackbar.showInfo(context, "Failed to Ble Listener");
    }
  }

  void stopDataListener() {
    _dataSubscription?.cancel();
  }

  @override
  void initState() {
    super.initState();
    WidgetsBinding.instance.addPostFrameCallback((_) async {
      await startDataListener();
      setState(() {
        isTablet = ResponsiveBreakpoints.of(context).isTablet;
      });
    });
  }

  @override
  void dispose() {
    stopDataListener();
    super.dispose();
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      body: SafeArea(
        child: SizedBox(
          width: MediaQuery.of(context).size.width,
          height: MediaQuery.of(context).size.height,
          child: Row(
            children: [
              Flexible(
                flex: 2,
                child: Container(
                  color: AppColor.lightBackground,
                  width: double.maxFinite,
                  height: double.maxFinite,
                  child: SingleChildScrollView(
                    child: Padding(
                      padding: EdgeInsets.only(
                        left: isTablet ? 10 : 20,
                        right: isTablet ? 10 : 20,
                      ),
                      child: Column(
                        spacing: 15,
                        children: [
                          Text('Harvest Egg', style: AppFont.h1PrimaryBold()),
                          GestureDetector(
                            onTap: () {
                              setState(() {
                                currentSelectedType = goodEggType;
                              });
                            },
                            child: Container(
                              padding: EdgeInsets.all(10),
                              decoration: BoxDecoration(
                                color: Colors.brown.shade100,
                                borderRadius: .circular(5),
                                border: currentSelectedType == goodEggType
                                    ? Border.all(
                                        color: AppColor.darkGreen,
                                        width: 5,
                                      )
                                    : null,
                              ),
                              child: Row(
                                mainAxisAlignment: .spaceBetween,
                                children: [
                                  Row(
                                    spacing: 20,
                                    children: [
                                      Image.asset(
                                        AppImage.eggs,
                                        height: isTablet ? 50 : 100,
                                        width: isTablet ? 50 : 100,
                                      ),
                                      Text(
                                        'Good Egg',
                                        style: isTablet
                                            ? AppFont.h2DarkRegular()
                                            : AppFont.h1DarkRegular(),
                                      ),
                                    ],
                                  ),
                                  Text(
                                    goodEgg.display,
                                    style: isTablet
                                        ? AppFont.h2PrimaryBold()
                                        : AppFont.h1PrimaryBold(),
                                  ),
                                ],
                              ),
                            ),
                          ),
                          GestureDetector(
                            onTap: () {
                              setState(() {
                                currentSelectedType = crackEggType;
                              });
                            },
                            child: Container(
                              padding: EdgeInsets.all(10),
                              decoration: BoxDecoration(
                                color: Colors.brown.shade100,
                                borderRadius: .circular(5),
                                border: currentSelectedType == crackEggType
                                    ? Border.all(
                                        color: AppColor.darkGreen,
                                        width: 5,
                                      )
                                    : null,
                              ),
                              child: Row(
                                mainAxisAlignment: .spaceBetween,
                                children: [
                                  Row(
                                    spacing: 20,
                                    children: [
                                      Image.asset(
                                        AppImage.crackEgg,
                                        height: isTablet ? 50 : 100,
                                        width: isTablet ? 50 : 100,
                                      ),
                                      Text(
                                        'Crack Egg',
                                        style: isTablet
                                            ? AppFont.h2DarkRegular()
                                            : AppFont.h1DarkRegular(),
                                      ),
                                    ],
                                  ),
                                  Text(
                                    crackEgg.display,
                                    style: isTablet
                                        ? AppFont.h2PrimaryBold()
                                        : AppFont.h1PrimaryBold(),
                                  ),
                                ],
                              ),
                            ),
                          ),

                          SizedBox(height: isTablet ? 10 : 30),
                          FilledButton(
                            onPressed: () async {
                              if (isLoading == false) {
                                if (goodEgg.quantity == 0) {
                                  PrimarySnackbar.showInfo(
                                    context,
                                    "You have must have atleast 1 Good egg to save.",
                                  );
                                  return;
                                }
                                setState(() {
                                  isLoading = true;
                                });
                                try {
                                  String goodEggHex = AppUtils.intToHex(
                                    goodEgg.quantity,
                                    4,
                                  );
                                  String crackEggHex = AppUtils.intToHex(
                                    crackEgg.quantity,
                                    4,
                                  );
                                  await bluetoothService.sendCommand(
                                    "4C544903$goodEggHex$crackEggHex",
                                    timeout: 60,
                                  );
                                } catch (e) {
                                  log("Error: $e");
                                  PrimarySnackbar.showInfo(
                                    context,
                                    "BLE Error",
                                  );
                                }
                              }
                            },
                            style: FilledButton.styleFrom(
                              backgroundColor: AppColor.primary,
                              shape: RoundedRectangleBorder(
                                borderRadius: .circular(8),
                              ),
                              padding: EdgeInsets.only(
                                top: isTablet ? 10 : 20,
                                bottom: isTablet ? 10 : 20,
                              ),
                            ),
                            child: Builder(
                              builder: (context) {
                                if (isLoading) {
                                  return Center(
                                    child: CircularProgressIndicator(
                                      color: Colors.transparent,
                                      valueColor: AlwaysStoppedAnimation(
                                        Colors.white,
                                      ),
                                    ),
                                  );
                                }
                                return Row(
                                  mainAxisAlignment: .center,
                                  spacing: 20,
                                  children: [
                                    Icon(Icons.save, size: isTablet ? 30 : 50),
                                    Text(
                                      'SAVE',
                                      style: TextStyle(
                                        fontSize: isTablet ? 20 : 30,
                                      ),
                                    ),
                                  ],
                                );
                              },
                            ),
                          ),
                          const SizedBox(height: 20),
                        ],
                      ),
                    ),
                  ),
                ),
              ),
              Flexible(
                child: Container(
                  color: AppColor.secondary,
                  padding: EdgeInsets.all(8),
                  child: Column(
                    children: List.generate(4, (columnIndex) {
                      return SizedBox(
                        height: (MediaQuery.of(context).size.height / 4.5) - 5,
                        child: Row(
                          children: List.generate(3, (rowIndex) {
                            final index = (columnIndex * 3) + rowIndex;

                            return Flexible(
                              child: SizedBox(
                                width: double.maxFinite,
                                height: double.maxFinite,
                                child: Builder(
                                  builder: (context) {
                                    if (keypad[index] == '*') {
                                      return GestureDetector(
                                        onTap: () {
                                          setState(() {
                                            switch (currentSelectedType) {
                                              case 'good':
                                                goodEgg
                                                    .decreaseByBackspaceKeypad();
                                                break;
                                              case 'crack':
                                                crackEgg
                                                    .decreaseByBackspaceKeypad();
                                              default:
                                            }
                                          });
                                        },
                                        child: Card(
                                          elevation: 5,
                                          shape: RoundedRectangleBorder(
                                            borderRadius: .circular(5),
                                          ),
                                          color: Colors.orangeAccent,
                                          child: Padding(
                                            padding: EdgeInsetsGeometry.all(10),
                                            child: Column(
                                              mainAxisSize: .min,
                                              mainAxisAlignment: .center,
                                              children: [
                                                Icon(
                                                  Icons.clear,
                                                  size: isTablet ? 20 : 40,
                                                ),
                                              ],
                                            ),
                                          ),
                                        ),
                                      );
                                    }
                                    if (keypad[index] == '#') {
                                      return GestureDetector(
                                        onTap: () {
                                          AppUtils.backToHome(context);
                                        },
                                        child: Card(
                                          elevation: 5,
                                          shape: RoundedRectangleBorder(
                                            borderRadius: .circular(5),
                                          ),
                                          color: Colors.redAccent,
                                          child: Padding(
                                            padding: EdgeInsetsGeometry.all(10),
                                            child: Column(
                                              mainAxisSize: .min,
                                              mainAxisAlignment: .center,
                                              children: [
                                                Icon(
                                                  Icons.exit_to_app,
                                                  size: isTablet ? 20 : 40,
                                                  color: Colors.white,
                                                ),
                                              ],
                                            ),
                                          ),
                                        ),
                                      );
                                    }
                                    return Consumer(
                                      builder: (context, ref, child) {
                                        int value =
                                            int.tryParse(keypad[index]) ?? 0;
                                        return GestureDetector(
                                          onTap: () {
                                            setState(() {
                                              switch (currentSelectedType) {
                                                case 'good':
                                                  goodEgg.increaseByNumKeypad(
                                                    value,
                                                    maxValue: 999_999_999,
                                                  );
                                                  break;
                                                case 'crack':
                                                  crackEgg.increaseByNumKeypad(
                                                    value,
                                                    maxValue: 999_999_999,
                                                  );
                                                default:
                                              }
                                            });
                                          },
                                          child: Card(
                                            elevation: 5,
                                            shape: RoundedRectangleBorder(
                                              borderRadius: .circular(5),
                                            ),
                                            color: Colors.white,
                                            child: Padding(
                                              padding: EdgeInsetsGeometry.all(
                                                10,
                                              ),
                                              child: Column(
                                                mainAxisSize: .min,
                                                mainAxisAlignment: .center,
                                                children: [
                                                  Text(
                                                    keypad[index],
                                                    style: TextStyle(
                                                      fontWeight:
                                                          FontWeight.bold,
                                                      fontSize: isTablet
                                                          ? 25
                                                          : 50,
                                                    ),
                                                  ),
                                                ],
                                              ),
                                            ),
                                          ),
                                        );
                                      },
                                    );
                                  },
                                ),
                              ),
                            );
                          }),
                        ),
                      );
                    }),
                  ),
                ),
              ),
            ],
          ),
        ),
      ),
    );
  }
}
