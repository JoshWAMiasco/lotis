import 'dart:async';
import 'dart:developer';

import 'package:flutter/material.dart';
import 'package:flutter_blue_plus/flutter_blue_plus.dart';
import 'package:flutter_riverpod/flutter_riverpod.dart';
import 'package:intl/intl.dart';
import 'package:lotis_manager_app/app_instance.dart';
import 'package:lotis_manager_app/components/primary_snackbar.dart';
import 'package:lotis_manager_app/data/egg.dart';
import 'package:lotis_manager_app/constants/app_color.dart';
import 'package:lotis_manager_app/constants/app_font.dart';
import 'package:lotis_manager_app/constants/app_image.dart';
import 'package:lotis_manager_app/constants/app_utils.dart';
import 'package:lotis_manager_app/data/price.dart';
import 'package:lotis_manager_app/providers/ble_device_provider.dart';
import 'package:responsive_framework/responsive_framework.dart';

class SellEggScreen extends ConsumerStatefulWidget {
  const SellEggScreen({super.key});

  @override
  ConsumerState<SellEggScreen> createState() => _SellEggScreenState();
}

class _SellEggScreenState extends ConsumerState<SellEggScreen> {
  final keypad = ['1', '2', '3', '4', '5', '6', '7', '8', '9', '*', '0', '#'];
  bool isTablet = false;
  Egg traycount = Egg(quantity: 0);
  Price price = Price(value: 0.0);
  String trayType = "tray";
  String priceType = "price";
  String? currentSelectedType;
  bool isLoading = false;
  StreamSubscription? _dataSubscription;
  String hasHalf = 'no';
  double totalSales = 0.0;

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
              if (commandType == '05') {
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

  void computeTotalSales() {
    double total = 0;
    total = traycount.quantity * price.value;
    if (hasHalf == 'yes') {
      total += price.value / 2;
    }
    setState(() {
      totalSales = total;
    });
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
                          Text('Sell Egg', style: AppFont.h1PrimaryBold()),
                          GestureDetector(
                            onTap: () {
                              setState(() {
                                currentSelectedType = trayType;
                              });
                            },
                            child: Container(
                              padding: EdgeInsets.all(10),
                              decoration: BoxDecoration(
                                color: Colors.brown.shade100,
                                borderRadius: .circular(5),
                                border: currentSelectedType == trayType
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
                                        AppImage.eggTray,
                                        height: isTablet ? 50 : 100,
                                        width: isTablet ? 50 : 100,
                                      ),
                                      Text(
                                        'Tray',
                                        style: isTablet
                                            ? AppFont.h2DarkRegular()
                                            : AppFont.h1DarkRegular(),
                                      ),
                                    ],
                                  ),
                                  Text(
                                    traycount.display,
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
                                currentSelectedType = priceType;
                              });
                            },
                            child: Container(
                              padding: EdgeInsets.all(10),
                              decoration: BoxDecoration(
                                color: Colors.brown.shade100,
                                borderRadius: .circular(5),
                                border: currentSelectedType == priceType
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
                                        AppImage.money,
                                        height: isTablet ? 50 : 100,
                                        width: isTablet ? 50 : 100,
                                      ),
                                      Text(
                                        'Price',
                                        style: isTablet
                                            ? AppFont.h2DarkRegular()
                                            : AppFont.h1DarkRegular(),
                                      ),
                                    ],
                                  ),
                                  Text(
                                    price.display,
                                    style: isTablet
                                        ? AppFont.h2PrimaryBold()
                                        : AppFont.h1PrimaryBold(),
                                  ),
                                ],
                              ),
                            ),
                          ),
                          Container(
                            padding: EdgeInsets.all(10),
                            decoration: BoxDecoration(
                              color: Colors.brown.shade100,
                              borderRadius: .circular(5),
                            ),
                            child: Row(
                              mainAxisAlignment: .spaceBetween,
                              children: [
                                Row(
                                  spacing: 20,
                                  children: [
                                    Image.asset(
                                      AppImage.halfEgg,
                                      height: isTablet ? 50 : 100,
                                      width: isTablet ? 50 : 100,
                                    ),
                                    Text(
                                      'Half',
                                      style: isTablet
                                          ? AppFont.h2DarkRegular()
                                          : AppFont.h1DarkRegular(),
                                    ),
                                  ],
                                ),
                                Row(
                                  spacing: 30,
                                  children: [
                                    Row(
                                      children: [
                                        RadioGroup(
                                          onChanged: (val) {
                                            setState(() {
                                              hasHalf = 'yes';
                                            });
                                            computeTotalSales();
                                          },
                                          groupValue: hasHalf,
                                          child: Radio(
                                            value: 'yes',
                                            activeColor: AppColor.primary,
                                          ),
                                        ),
                                        InkWell(
                                          onTap: () {
                                            setState(() {
                                              hasHalf = 'yes';
                                            });
                                            computeTotalSales();
                                          },
                                          child: Text(
                                            'Yes',
                                            style: isTablet
                                                ? AppFont.h2PrimaryBold()
                                                : AppFont.h1PrimaryBold(),
                                          ),
                                        ),
                                      ],
                                    ),
                                    Row(
                                      children: [
                                        RadioGroup(
                                          onChanged: (val) {
                                            setState(() {
                                              hasHalf = 'no';
                                            });
                                            computeTotalSales();
                                          },
                                          groupValue: hasHalf,
                                          child: Radio(
                                            value: 'no',
                                            activeColor: AppColor.primary,
                                          ),
                                        ),
                                        InkWell(
                                          onTap: () {
                                            setState(() {
                                              hasHalf = 'no';
                                            });
                                            computeTotalSales();
                                          },
                                          child: Text(
                                            'No',
                                            style: isTablet
                                                ? AppFont.h2PrimaryBold()
                                                : AppFont.h1PrimaryBold(),
                                          ),
                                        ),
                                      ],
                                    ),
                                  ],
                                ),
                              ],
                            ),
                          ),
                          Container(
                            padding: EdgeInsets.all(20),
                            child: Row(
                              mainAxisAlignment: .spaceBetween,
                              children: [
                                Text(
                                  'Total Sales',
                                  style: isTablet
                                      ? AppFont.h2DarkRegular()
                                      : AppFont.h1DarkRegular(),
                                ),
                                Builder(
                                  builder: (context) {
                                    double value = totalSales;
                                    String display = NumberFormat(
                                      "###,###,###.##",
                                    ).format(value);
                                    return Text(
                                      "₱ $display",
                                      style: isTablet
                                          ? AppFont.h2PrimaryBold()
                                          : AppFont.h1PrimaryBold(),
                                    );
                                  },
                                ),
                              ],
                            ),
                          ),
                          SizedBox(height: isTablet ? 10 : 30),
                          FilledButton(
                            onPressed: () async {
                              if (isLoading == false) {
                                if (traycount.quantity == 0) {
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
                                  String trayCountHex = AppUtils.intToHex(
                                    traycount.quantity,
                                    4,
                                  );
                                  String priceHex = AppUtils.intToHex(
                                    price.value.toInt(),
                                    4,
                                  );
                                  String totalSalesHex = AppUtils.intToHex(
                                    totalSales.toInt(),
                                    4,
                                  );
                                  String hasHalfHex = hasHalf == 'yes'
                                      ? '01'
                                      : '00';
                                  await bluetoothService.sendCommand(
                                    "4C544905$trayCountHex$priceHex$hasHalfHex$totalSalesHex",
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
                                              case 'tray':
                                                traycount
                                                    .decreaseByBackspaceKeypad();
                                                break;
                                              case 'price':
                                                price.decreaseByKeypad();
                                              default:
                                            }
                                          });
                                          computeTotalSales();
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
                                                case 'tray':
                                                  traycount.increaseByNumKeypad(
                                                    value,
                                                    maxValue: 999_999_999,
                                                  );
                                                  break;
                                                case 'price':
                                                  price.increaseByKeypad(value);
                                                default:
                                              }
                                            });
                                            computeTotalSales();
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
