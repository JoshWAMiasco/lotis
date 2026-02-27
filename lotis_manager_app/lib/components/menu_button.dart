import 'package:flutter/material.dart';
import 'package:lotis_manager_app/constants/app_color.dart';

class MenuButton extends StatelessWidget {
  const MenuButton({
    super.key,
    required this.title,
    required this.appImage,
    required this.onPressed,
    this.enabled = false,
  });
  final String title;
  final String appImage;
  final VoidCallback onPressed;
  final bool enabled;
  @override
  Widget build(BuildContext context) {
    return TextButton(
      style: TextButton.styleFrom(
        shape: RoundedRectangleBorder(borderRadius: .circular(10)),
        side: BorderSide(color: Colors.grey, width: 2),
        minimumSize: Size(
          (MediaQuery.of(context).size.width / 4) - 30,
          MediaQuery.of(context).size.height * 0.4,
        ),
        backgroundColor: enabled ? Colors.white : Colors.grey,
      ),

      onPressed: enabled ? onPressed : null,
      child: Column(
        spacing: 10,
        children: [
          Image.asset(
            appImage,
            height: MediaQuery.of(context).size.height * 0.18,
            opacity: enabled ? null : AlwaysStoppedAnimation(0.3),
          ),
          Text(
            title,
            style: TextStyle(
              fontSize: MediaQuery.of(context).size.height * 0.05,
              color: enabled ? AppColor.dark : Colors.grey.shade300,
            ),
          ),
        ],
      ),
    );
  }
}
