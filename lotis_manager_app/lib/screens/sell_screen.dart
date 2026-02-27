import 'package:flutter/material.dart';
import 'package:go_router/go_router.dart';
import 'package:lotis_manager_app/app_instance.dart';
import 'package:lotis_manager_app/components/menu_button.dart';
import 'package:lotis_manager_app/constants/app_color.dart';
import 'package:lotis_manager_app/constants/app_font.dart';
import 'package:lotis_manager_app/constants/app_image.dart';
import 'package:lotis_manager_app/constants/app_utils.dart';

class SellScreen extends StatelessWidget {
  const SellScreen({super.key});

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      backgroundColor: AppColor.lightBackground,
      appBar: AppBar(
        title: Text('Sell', style: AppFont.h1DarkRegular()),
        backgroundColor: AppColor.lightBackground,
        centerTitle: true,
        automaticallyImplyLeading: false,
      ),
      body: Padding(
        padding: const EdgeInsets.all(10),
        child: Wrap(
          children: [
            MenuButton(
              enabled: true,
              title: 'Egg',
              appImage: AppImage.eggs,
              onPressed: () {
                GoRouter.of(context).goNamed('sell_egg');
              },
            ),
          ],
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
