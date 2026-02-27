import 'dart:developer';

import 'package:flutter/material.dart';
import 'package:go_router/go_router.dart';
import 'package:lotis_manager_app/app_instance.dart';
import 'package:lotis_manager_app/components/primary_snackbar.dart';
import 'package:lotis_manager_app/constants/app_color.dart';
import 'package:lotis_manager_app/constants/app_font.dart';
import 'package:lotis_manager_app/constants/app_utils.dart';
import 'package:responsive_framework/responsive_framework.dart';

class SetDatetimeScreen extends StatefulWidget {
  const SetDatetimeScreen({super.key});

  @override
  State<SetDatetimeScreen> createState() => _SetDatetimeScreenState();
}

class _SetDatetimeScreenState extends State<SetDatetimeScreen> {
  final TextEditingController _hourController = TextEditingController();
  final TextEditingController _minuteController = TextEditingController();
  bool isPm = false;
  DateTime dateSet = DateTime.now();
  TimeOfDay timeSet = TimeOfDay.now();
  bool isTablet = false;
  @override
  void initState() {
    super.initState();
    WidgetsBinding.instance.addPostFrameCallback((_) {
      _hourController.text = timeSet.hour > 12
          ? (timeSet.hour - 12).toString().padLeft(2, "0")
          : timeSet.hour.toString().padLeft(2, "0");
      _minuteController.text = timeSet.minute.toString().padLeft(2, "0");
      isPm = timeSet.period == DayPeriod.pm;
      setState(() {
        isTablet = ResponsiveBreakpoints.of(context).isTablet;
      });
    });
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      backgroundColor: AppColor.background,
      appBar: AppBar(
        title: Text('Set Date & Time', style: AppFont.h1DarkRegular()),
        backgroundColor: AppColor.lightBackground,
        centerTitle: true,
        automaticallyImplyLeading: false,
      ),
      body: SafeArea(
        child: Padding(
          padding: const EdgeInsets.all(15),
          child: Column(
            spacing: 20,
            mainAxisSize: .min,
            children: [
              SizedBox(
                width: MediaQuery.of(context).size.width,
                child: Row(
                  spacing: 20,
                  mainAxisSize: .min,
                  children: [
                    Container(
                      decoration: BoxDecoration(
                        color: Colors.white,
                        border: Border.all(color: Colors.grey),
                      ),
                      padding: EdgeInsets.all(8),
                      height: MediaQuery.of(context).size.height * 0.5,
                      width: MediaQuery.of(context).size.width * 0.3,
                      child: CalendarDatePicker(
                        initialDate: dateSet,
                        firstDate: DateTime.now().subtract(
                          const Duration(days: 10),
                        ),
                        lastDate: DateTime.now().add(Duration(days: 30)),
                        onDateChanged: (date) {
                          setState(() {
                            dateSet = date;
                          });
                        },
                      ),
                    ),
                    SizedBox(
                      height: MediaQuery.of(context).size.height * 0.5,
                      width: MediaQuery.of(context).size.width * 0.5,
                      child: Container(
                        decoration: BoxDecoration(color: Colors.white),
                        padding: EdgeInsets.all(20),
                        child: Column(
                          mainAxisAlignment: .center,
                          spacing: 20,
                          children: [
                            Text(
                              'Time',
                              style: TextStyle(
                                fontWeight: FontWeight.bold,
                                color: AppColor.primary,
                                fontSize: 20,
                              ),
                            ),
                            Row(
                              mainAxisAlignment: .center,
                              spacing: 15,
                              children: [
                                Container(
                                  decoration: BoxDecoration(
                                    border: Border.all(color: Colors.grey),
                                  ),
                                  padding: EdgeInsets.all(10),
                                  height: 95,
                                  width: 85,
                                  child: TextField(
                                    controller: _hourController,
                                    decoration: InputDecoration(
                                      border: InputBorder.none,
                                      labelText: 'Hour',
                                      floatingLabelBehavior:
                                          FloatingLabelBehavior.always,
                                    ),
                                    keyboardType: TextInputType.number,
                                    style: TextStyle(
                                      fontWeight: FontWeight.bold,
                                      color: AppColor.primary,
                                      fontSize: 50,
                                    ),
                                    cursorWidth: 0,
                                  ),
                                ),
                                Text(
                                  ":",
                                  style: TextStyle(
                                    fontWeight: FontWeight.bold,
                                    color: AppColor.primary,
                                    fontSize: 50,
                                  ),
                                ),
                                Container(
                                  decoration: BoxDecoration(
                                    border: Border.all(color: Colors.grey),
                                  ),
                                  padding: EdgeInsets.all(10),
                                  height: 95,
                                  width: 85,
                                  child: TextField(
                                    controller: _minuteController,
                                    decoration: InputDecoration(
                                      border: InputBorder.none,
                                      labelText: 'Minute',
                                      floatingLabelBehavior:
                                          FloatingLabelBehavior.always,
                                    ),
                                    keyboardType: TextInputType.number,
                                    style: TextStyle(
                                      fontWeight: FontWeight.bold,
                                      color: AppColor.primary,
                                      fontSize: 50,
                                    ),
                                    cursorWidth: 0,
                                  ),
                                ),
                              ],
                            ),
                            Row(
                              mainAxisAlignment: .center,
                              spacing: 10,
                              children: [
                                TextButton(
                                  style: TextButton.styleFrom(
                                    shape: RoundedRectangleBorder(
                                      borderRadius: .circular(5),
                                    ),
                                    backgroundColor: isPm
                                        ? Colors.white
                                        : AppColor.darkGreen,
                                    side: BorderSide(color: Colors.grey),
                                  ),
                                  onPressed: () {
                                    setState(() {
                                      isPm = false;
                                    });
                                  },
                                  child: Text(
                                    "AM",
                                    style: TextStyle(
                                      fontWeight: FontWeight.bold,
                                      color: isPm
                                          ? AppColor.primary
                                          : Colors.white,
                                      fontSize: 40,
                                    ),
                                  ),
                                ),
                                TextButton(
                                  style: TextButton.styleFrom(
                                    shape: RoundedRectangleBorder(
                                      borderRadius: .circular(5),
                                    ),
                                    backgroundColor: isPm
                                        ? AppColor.darkGreen
                                        : Colors.white,
                                    side: BorderSide(color: Colors.grey),
                                  ),
                                  onPressed: () {
                                    setState(() {
                                      isPm = true;
                                    });
                                  },
                                  child: Text(
                                    "PM",
                                    style: TextStyle(
                                      fontWeight: FontWeight.bold,
                                      color: isPm
                                          ? Colors.white
                                          : AppColor.primary,
                                      fontSize: 40,
                                    ),
                                  ),
                                ),
                              ],
                            ),
                          ],
                        ),
                      ),
                    ),
                  ],
                ),
              ),
              FilledButton(
                onPressed: () async {
                  try {
                    String hexCommand = "4C54";
                    hexCommand += "4801"; // command and command type

                    hexCommand += AppUtils.intToHex(dateSet.year, 4);
                    hexCommand += AppUtils.intToHex(dateSet.month, 2);
                    hexCommand += AppUtils.intToHex(dateSet.day, 2);
                    int hour = int.tryParse(_hourController.text) ?? 0;
                    int minute = int.tryParse(_minuteController.text) ?? 0;
                    hexCommand += AppUtils.intToHex(hour, 2);
                    hexCommand += AppUtils.intToHex(minute, 2);
                    hexCommand += isPm ? "01" : "00";
                    await bluetoothService.sendCommand(hexCommand);
                  } catch (e) {
                    PrimarySnackbar.showInfo(context, "BLE error");
                  }
                },
                style: FilledButton.styleFrom(
                  backgroundColor: AppColor.primary,
                  shape: RoundedRectangleBorder(borderRadius: .circular(8)),
                  padding: EdgeInsets.only(
                    top: isTablet ? 10 : 20,
                    bottom: isTablet ? 10 : 20,
                  ),
                ),
                child: Builder(
                  builder: (context) {
                    return Row(
                      mainAxisAlignment: .center,
                      spacing: 20,
                      children: [
                        Icon(Icons.save, size: isTablet ? 30 : 50),
                        Text(
                          'SAVE',
                          style: TextStyle(fontSize: isTablet ? 20 : 30),
                        ),
                      ],
                    );
                  },
                ),
              ),
            ],
          ),
        ),
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
