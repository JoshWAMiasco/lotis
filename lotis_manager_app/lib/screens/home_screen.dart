import 'dart:async';
import 'dart:developer';

import 'package:flutter/material.dart';
import 'package:flutter_blue_plus/flutter_blue_plus.dart';
import 'package:flutter_riverpod/flutter_riverpod.dart';
import 'package:flutter_svg/flutter_svg.dart';
import 'package:go_router/go_router.dart';
import 'package:lotis_manager_app/app_instance.dart';
import 'package:lotis_manager_app/components/ble_connection_dialog.dart';
import 'package:lotis_manager_app/components/menu_button.dart';
import 'package:lotis_manager_app/components/primary_snackbar.dart';
import 'package:lotis_manager_app/constants/app_color.dart';
import 'package:lotis_manager_app/constants/app_font.dart';
import 'package:lotis_manager_app/constants/app_image.dart';
import 'package:lotis_manager_app/constants/app_svg.dart';
import 'package:lotis_manager_app/constants/app_utils.dart';
import 'package:lotis_manager_app/providers/ble_device_provider.dart';
import 'package:lotis_manager_app/providers/ble_services_provider.dart';
import 'package:lotis_manager_app/providers/switch_state_provider.dart';
import 'package:lotis_manager_app/services/ble_service.dart';

class HomeScreen extends StatefulWidget {
  const HomeScreen({super.key});

  @override
  State<HomeScreen> createState() => _HomeScreenState();
}

class _HomeScreenState extends State<HomeScreen> {
  bool connecting = false;
  bool enableFeatures = false;

  @override
  void dispose() {
    super.dispose();
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      backgroundColor: AppColor.lightBackground,
      appBar: null,
      body: SafeArea(
        child: SingleChildScrollView(
          child: Padding(
            padding: const EdgeInsets.all(10),
            child: Wrap(
              spacing: 20,
              runSpacing: 20,
              children: [
                MenuButton(
                  title: 'Harvest',
                  enabled: enableFeatures,
                  appImage: AppImage.harvest,
                  onPressed: () {
                    GoRouter.of(context).pushNamed('harvest');
                  },
                ),
                MenuButton(
                  title: 'Sell',
                  enabled: enableFeatures,
                  appImage: AppImage.sellEgg,
                  onPressed: () {
                    GoRouter.of(context).pushNamed('sell');
                  },
                ),
                MenuButton(
                  title: 'Settings',
                  enabled: enableFeatures,
                  appImage: AppImage.settings,
                  onPressed: () {
                    GoRouter.of(context).pushNamed('settings');
                  },
                ),
                MenuButton(
                  title: 'Switch',
                  enabled: enableFeatures,
                  appImage: AppImage.switches,
                  onPressed: () {
                    GoRouter.of(context).pushNamed('switch');
                  },
                ),
              ],
            ),
          ),
        ),
      ),
      floatingActionButton: Consumer(
        builder: (context, ref, child) {
          if (connecting) {
            return FloatingActionButton.large(
              backgroundColor: AppColor.primary,
              onPressed: () {},
              child: CircularProgressIndicator(
                color: Colors.transparent,
                valueColor: AlwaysStoppedAnimation(Colors.white),
              ),
            );
          }
          return FloatingActionButton.extended(
            backgroundColor: ref.watch(bleDeviceProvider) == null
                ? AppColor.primary
                : AppColor.darkGreen,
            onPressed: () async {
              if (ref.read(bleDeviceProvider) == null) {
                await showDialog<BluetoothDevice?>(
                  context: context,
                  builder: (context) {
                    return BleConnectionDialog();
                  },
                ).then((device) async {
                  if (device == null) {
                    return;
                  }
                  setState(() {
                    connecting = true;
                  });
                  try {
                    await device
                        .connect(timeout: const Duration(seconds: 10))
                        .then((_) async {
                          await device.discoverServices().then((
                            listOfServices,
                          ) async {
                            if (listOfServices.isNotEmpty) {
                              for (var service in listOfServices) {
                                log("🌀 ${service.serviceUuid.str}");
                                for (var charac in service.characteristics) {
                                  log("   ⚪️ ${charac.characteristicUuid.str}");
                                }
                              }
                            }
                            if (!mounted) return;
                            bluetoothService.setup(device, listOfServices);
                            ref.read(bleDeviceProvider.notifier).set(device);

                            setState(() {
                              connecting = false;
                              enableFeatures = true;
                            });

                            ref
                                .read(bleServicesProvider.notifier)
                                .set(listOfServices);
                            await bluetoothService.sendCommand("4C544301");
                          });
                        });
                    PrimarySnackbar.showInfo(context, "LoTIS connected");
                  } catch (e) {
                    PrimarySnackbar.showInfo(context, "Failed to connect");
                    setState(() {
                      connecting = false;
                    });
                  }
                });
              } else {
                await showDialog<bool?>(
                  context: context,
                  builder: (_) {
                    return AlertDialog(
                      title: Text('Disconnect', style: AppFont.h4DarkBold()),
                      content: Text(
                        'Are you sure you want to disconnect to LoTIS Device',
                        style: AppFont.h5DarkRegular(),
                      ),
                      actions: [
                        FilledButton(
                          style: FilledButton.styleFrom(
                            backgroundColor: Colors.red,
                          ),
                          onPressed: () {
                            Navigator.of(context).pop(true);
                          },
                          child: Text(
                            'Disconnect',
                            style: AppFont.h5WhiteBold(),
                          ),
                        ),
                      ],
                    );
                  },
                ).then((isYes) async {
                  if (isYes ?? false) {
                    try {
                      await ref.read(bleDeviceProvider)?.disconnect();
                      PrimarySnackbar.showInfo(context, "LoTIS Disconnected");
                      ref.read(bleDeviceProvider.notifier).set(null);
                      ref.read(bleServicesProvider.notifier).set([]);
                      bluetoothService.clear();
                      setState(() {
                        enableFeatures = false;
                      });
                    } catch (e) {
                      PrimarySnackbar.showInfo(context, "Failed to Disconnec");
                    }
                  }
                });
              }
            },
            label: Builder(
              builder: (_) {
                if (ref.watch(bleDeviceProvider) != null) {
                  return Row(
                    spacing: 20,
                    children: [
                      Row(
                        spacing: 25,
                        children: [
                          SvgPicture.asset(
                            AppSvg.scanDeviceIcon,
                            height: 25,
                            width: 25,
                            colorFilter: ColorFilter.mode(
                              Colors.white,
                              BlendMode.srcIn,
                            ),
                          ),
                          Text('LoTIS', style: AppFont.h5WhiteRegular()),
                        ],
                      ),
                      Icon(Icons.close, color: Colors.red),
                    ],
                  );
                }
                return Row(
                  spacing: 25,
                  children: [
                    SvgPicture.asset(
                      AppSvg.scanDeviceIcon,
                      height: 25,
                      width: 25,
                      colorFilter: ColorFilter.mode(
                        Colors.white,
                        BlendMode.srcIn,
                      ),
                    ),
                    Text('Connect', style: AppFont.h5WhiteRegular()),
                  ],
                );
              },
            ),
          );
        },
      ),
    );
  }
}
