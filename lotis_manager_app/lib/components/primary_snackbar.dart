import 'package:flutter/material.dart';
import 'package:lotis_manager_app/constants/app_font.dart';

class PrimarySnackbar {
  static void showInfo(BuildContext context, String message) {
    ScaffoldMessenger.of(context).showSnackBar(
      SnackBar(
        content: Text(message, style: AppFont.h3WhiteRegular()),
        backgroundColor: Colors.black,
        behavior: SnackBarBehavior.floating,
        padding: EdgeInsets.all(20),
        shape: RoundedRectangleBorder(
          borderRadius: BorderRadiusGeometry.circular(10),
        ),
        showCloseIcon: true,
        margin: EdgeInsets.only(
          left: MediaQuery.of(context).size.width * 0.1,
          right: MediaQuery.of(context).size.width * 0.1,
          bottom: MediaQuery.of(context).size.width * 0.02,
        ),
      ),
    );
  }
}
