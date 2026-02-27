import 'package:flutter/material.dart';
import 'package:flutter_screenutil/flutter_screenutil.dart';
import 'package:lotis_manager_app/constants/app_color.dart';
import 'package:lotis_manager_app/constants/app_font.dart';
import 'package:responsive_framework/responsive_framework.dart';

class SwitchButtonCard extends StatelessWidget {
  const SwitchButtonCard({
    super.key,
    this.isActive = false,
    this.name = '',
    this.editMode = false,
    required this.onTap,
  });
  final bool isActive;
  final String name;
  final bool editMode;
  final VoidCallback onTap;
  @override
  Widget build(BuildContext context) {
    return GestureDetector(
      onTap: onTap,
      child: Builder(
        builder: (context) {
          bool isPhone = ResponsiveBreakpoints.of(context).isTablet;
          return SizedBox(
            height: isPhone ? 120 : 200,
            width: isPhone ? 120 : 200,
            child: Card(
              elevation: isActive ? 0 : 10,
              color: editMode
                  ? AppColor.secondary
                  : isActive
                  ? Colors.white
                  : Colors.grey.shade200,
              shape: RoundedRectangleBorder(
                side: isActive
                    ? BorderSide(color: Colors.black)
                    : BorderSide.none,
                borderRadius: .circular(8),
              ),
              child: Padding(
                padding: EdgeInsets.all(10),
                child: Column(
                  mainAxisAlignment: .spaceBetween,
                  children: [
                    Align(
                      alignment: .topLeft,
                      child: Text(
                        name,
                        style: isPhone
                            ? (editMode
                                  ? AppFont.h5WhiteBold()
                                  : AppFont.h5DarkBold())
                            : (editMode
                                  ? AppFont.h3WhiteBold()
                                  : AppFont.h3DarkBold()),
                      ),
                    ),
                    if (editMode == false)
                      Row(
                        mainAxisAlignment: .end,
                        spacing: 20,
                        children: [
                          Text(
                            isActive ? 'ON' : 'OFF',
                            style: isPhone
                                ? AppFont.h4DarkRegular()
                                : AppFont.h1DarkRegular(),
                          ),
                          Container(
                            height: isPhone ? 10 : 30,
                            width: isPhone ? 10 : 30,
                            decoration: BoxDecoration(
                              color: isActive ? Colors.green : Colors.red,
                              shape: BoxShape.circle,
                            ),
                          ),
                        ],
                      ),
                  ],
                ),
              ),
            ),
          );
        },
      ),
    );
  }
}
