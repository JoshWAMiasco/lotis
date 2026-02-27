import 'dart:async';
import 'dart:developer';

import 'package:flutter/material.dart';
import 'package:flutter_blue_plus/flutter_blue_plus.dart';
import 'package:flutter_riverpod/flutter_riverpod.dart';
import 'package:go_router/go_router.dart';
import 'package:lotis_manager_app/app_instance.dart';
import 'package:lotis_manager_app/components/primary_snackbar.dart';
import 'package:lotis_manager_app/components/switch_button_card.dart';
import 'package:lotis_manager_app/constants/app_color.dart';
import 'package:lotis_manager_app/constants/app_font.dart';
import 'package:lotis_manager_app/constants/app_utils.dart';
import 'package:lotis_manager_app/providers/ble_device_provider.dart';
import 'package:responsive_framework/responsive_framework.dart';
import 'package:shared_preferences/shared_preferences.dart';

class SwitchScreens extends ConsumerStatefulWidget {
  const SwitchScreens({super.key});

  @override
  ConsumerState<SwitchScreens> createState() => _SwitchScreensState();
}

class _SwitchScreensState extends ConsumerState<SwitchScreens> {
  bool editMode = false;
  bool isLoading = true;

  Map<int, String> switchNames = {
    0: "Switch 0",
    1: "Switch 1",
    2: "Switch 2",
    3: "Switch 3",
    4: "Switch 4",
    5: "Switch 5",
    6: "Switch 6",
    7: "Switch 7",
  };

  Map<int, bool> switchStates = {
    0: false,
    1: false,
    2: false,
    3: false,
    4: false,
    5: false,
    6: false,
    7: false,
  };

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
            case '48':
              if (commandType == '02') {
                setState(() {
                  switchStates = {
                    0: raw[4] == 1,
                    1: raw[5] == 1,
                    2: raw[6] == 1,
                    3: raw[7] == 1,
                    4: raw[8] == 1,
                    5: raw[9] == 1,
                    6: raw[10] == 1,
                    7: raw[11] == 1,
                  };
                  isLoading = false;
                });
              } else if (commandType == '03' && raw.last == 0) {
                int index = raw[4] - 1;
                setState(() {
                  switchStates[index] = !(switchStates[index] ?? false);
                });
              }
              break;
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

  Future<void> editName(int count) async {
    final TextEditingController _textEditingController = TextEditingController(
      text: switchNames[count],
    );
    final _fromkey = GlobalKey<FormState>();
    await showDialog<String?>(
      context: context,
      builder: (context) {
        return AlertDialog(
          shape: RoundedRectangleBorder(borderRadius: .circular(10)),
          title: Text('Name', style: AppFont.h5DarkBold()),
          content: Form(
            key: _fromkey,
            child: TextFormField(
              controller: _textEditingController,
              decoration: InputDecoration(border: OutlineInputBorder()),
              validator: (value) {
                if (value == null || value == '') {
                  return 'This is a required field';
                } else {
                  return null;
                }
              },
            ),
          ),
          actions: [
            FilledButton(
              onPressed: () {
                Navigator.of(context).pop();
              },
              style: FilledButton.styleFrom(backgroundColor: AppColor.danger),
              child: Text('Cancel'),
            ),
            FilledButton(
              onPressed: () {
                if (_fromkey.currentState!.validate()) {
                  Navigator.of(context).pop(_textEditingController.text);
                }
              },
              style: FilledButton.styleFrom(
                backgroundColor: AppColor.lightGreen,
              ),
              child: Text('Save'),
            ),
          ],
        );
      },
    ).then((name) async {
      if (name != null) {
        final sharedPref = await SharedPreferences.getInstance();
        await sharedPref.setString('switch_$count', name);
        setState(() {
          switchNames[count] = name;
        });
      }
    });
  }

  Future<void> getNames() async {
    final sharedRef = await SharedPreferences.getInstance();
    for (var i = 0; i < 8; i++) {
      String? name = sharedRef.getString('switch_$i');
      if (name != null) {
        setState(() {
          switchNames[i] = name;
        });
      }
    }
  }

  @override
  void initState() {
    super.initState();
    WidgetsBinding.instance.addPostFrameCallback((_) async {
      await getNames();
      await startDataListener();
      setState(() {
        isLoading = true;
      });
      await bluetoothService.sendCommand("4C544901");
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
      backgroundColor: AppColor.lightBackground,
      appBar: AppBar(
        automaticallyImplyLeading: false,
        title: Text('Switch', style: AppFont.h1PrimaryRegular()),
        centerTitle: true,
        backgroundColor: AppColor.lightBackground,
        elevation: 0,
      ),
      body: Builder(
        builder: (context) {
          if (isLoading) {
            return Center(
              child: CircularProgressIndicator(
                color: Colors.transparent,
                valueColor: AlwaysStoppedAnimation(AppColor.primary),
              ),
            );
          }
          return SafeArea(
            child: Padding(
              padding: const EdgeInsets.all(10),
              child: SingleChildScrollView(
                child: Consumer(
                  builder: (context, ref, _) {
                    return Wrap(
                      spacing: 20,
                      runSpacing: 20,
                      children: [
                        Wrap(
                          spacing: 20,
                          runSpacing: 20,
                          children: List.generate(switchStates.entries.length, (
                            index,
                          ) {
                            return SwitchButtonCard(
                              onTap: () async {
                                if (editMode) {
                                  editName(index);
                                } else {
                                  String switchPin = AppUtils.byteToHex(
                                    index + 1,
                                  );
                                  try {
                                    await bluetoothService.sendCommand(
                                      "4C544803$switchPin",
                                    );
                                  } catch (e) {
                                    PrimarySnackbar.showInfo(
                                      context,
                                      "Error Ble Commmand",
                                    );
                                  }
                                }
                              },
                              editMode: editMode,
                              isActive: switchStates[index] ?? false,
                              name: switchNames[index] ?? '',
                            );
                          }),
                        ),
                        Builder(
                          builder: (context) {
                            bool isPhone = ResponsiveBreakpoints.of(
                              context,
                            ).isTablet;
                            return SizedBox(
                              height: isPhone ? 120 : 200,
                              width: isPhone ? 120 : 200,
                              child: GestureDetector(
                                onTap: () {
                                  setState(() {
                                    editMode = !editMode;
                                  });
                                },
                                child: Card(
                                  elevation: 0,
                                  color: AppColor.lightBackground,
                                  shape: RoundedRectangleBorder(
                                    side: BorderSide(color: Colors.black),
                                    borderRadius: .circular(8),
                                  ),
                                  child: Padding(
                                    padding: EdgeInsets.all(10),
                                    child: Column(
                                      mainAxisAlignment: .center,
                                      spacing: 20,
                                      children: [
                                        Icon(
                                          editMode ? Icons.close : Icons.edit,
                                          size: isPhone ? 20 : 50,
                                        ),
                                        Text(
                                          'Edit',
                                          style: isPhone
                                              ? AppFont.h5DarkRegular()
                                              : AppFont.h2DarkRegular(),
                                        ),
                                      ],
                                    ),
                                  ),
                                ),
                              ),
                            );
                          },
                        ),
                      ],
                    );
                  },
                ),
              ),
            ),
          );
        },
      ),
      floatingActionButtonLocation: .endFloat,
      floatingActionButton: FloatingActionButton.extended(
        onPressed: () {},

        backgroundColor: AppColor.darkGreen,
        label: Row(
          mainAxisSize: .min,
          spacing: 20,
          children: [
            IconButton(
              onPressed: () {
                AppUtils.backToHome(context);
              },
              icon: Icon(Icons.home, color: Colors.white),
            ),
            IconButton(
              onPressed: () {
                GoRouter.of(context).pop();
              },
              icon: Row(
                children: [
                  Icon(Icons.arrow_back_ios, color: Colors.white),
                  Text('Back', style: AppFont.h4WhiteBold()),
                ],
              ),
            ),
          ],
        ),
      ),
    );
  }
}
