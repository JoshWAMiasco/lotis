import 'package:flutter/material.dart';
import 'package:lotis_manager_app/constants/app_color.dart';

class AppFont {
  // Color scheme
  static const Color dark = Color(0xFF1A1A1A);
  static const Color primary = AppColor.primary;
  static const Color danger = AppColor.danger;
  static const Color disabled = AppColor.disabled;
  static const Color white = AppColor.white;
  static const Color darkGreen = AppColor.darkGreen;
  static const Color lightGreen = AppColor.lightGreen;

  // Font sizes
  static const double h1FontSize = 32.0;
  static const double h2FontSize = 24.0;
  static const double h3FontSize = 20.0;
  static const double h4FontSize = 16.0;
  static const double h5FontSize = 14.0;
  static const double paragraphFontSize = 14.0;
  static const double captionFontSize = 12.0;

  // H1 styles
  static TextStyle h1DarkRegular({
    TextDecoration? decoration,
    FontStyle? fontStyle,
  }) {
    return TextStyle(
      fontWeight: FontWeight.w400,
      fontSize: h1FontSize,
      color: dark,
      decoration: decoration,
      fontStyle: fontStyle,
    );
  }

  static TextStyle h1DarkBold({
    TextDecoration? decoration,
    FontStyle? fontStyle,
  }) {
    return TextStyle(
      fontWeight: FontWeight.w700,
      fontSize: h1FontSize,
      color: dark,
      decoration: decoration,
      fontStyle: fontStyle,
    );
  }

  static TextStyle h1PrimaryRegular({
    TextDecoration? decoration,
    FontStyle? fontStyle,
  }) {
    return TextStyle(
      fontWeight: FontWeight.w400,
      fontSize: h1FontSize,
      color: primary,
      decoration: decoration,
      fontStyle: fontStyle,
    );
  }

  static TextStyle h1PrimaryBold({
    TextDecoration? decoration,
    FontStyle? fontStyle,
  }) {
    return TextStyle(
      fontWeight: FontWeight.w700,
      fontSize: h1FontSize,
      color: primary,
      decoration: decoration,
      fontStyle: fontStyle,
    );
  }

  static TextStyle h1WhiteRegular({
    TextDecoration? decoration,
    FontStyle? fontStyle,
  }) {
    return TextStyle(
      fontWeight: FontWeight.w400,
      fontSize: h1FontSize,
      color: white,
      decoration: decoration,
      fontStyle: fontStyle,
    );
  }

  static TextStyle h1WhiteBold({
    TextDecoration? decoration,
    FontStyle? fontStyle,
  }) {
    return TextStyle(
      fontWeight: FontWeight.w700,
      fontSize: h1FontSize,
      color: white,
      decoration: decoration,
      fontStyle: fontStyle,
    );
  }

  static TextStyle h1DangerRegular({
    TextDecoration? decoration,
    FontStyle? fontStyle,
  }) {
    return TextStyle(
      fontWeight: FontWeight.w400,
      fontSize: h1FontSize,
      color: danger,
      decoration: decoration,
      fontStyle: fontStyle,
    );
  }

  static TextStyle h1DangerBold({
    TextDecoration? decoration,
    FontStyle? fontStyle,
  }) {
    return TextStyle(
      fontWeight: FontWeight.w700,
      fontSize: h1FontSize,
      color: danger,
      decoration: decoration,
      fontStyle: fontStyle,
    );
  }

  static TextStyle h1DisabledRegular({
    TextDecoration? decoration,
    FontStyle? fontStyle,
  }) {
    return TextStyle(
      fontWeight: FontWeight.w400,
      fontSize: h1FontSize,
      color: disabled,
      decoration: decoration,
      fontStyle: fontStyle,
    );
  }

  static TextStyle h1DisabledBold({
    TextDecoration? decoration,
    FontStyle? fontStyle,
  }) {
    return TextStyle(
      fontWeight: FontWeight.w700,
      fontSize: h1FontSize,
      color: disabled,
      decoration: decoration,
      fontStyle: fontStyle,
    );
  }

  // H2 styles
  static TextStyle h2DarkRegular({
    TextDecoration? decoration,
    FontStyle? fontStyle,
  }) {
    return TextStyle(
      fontWeight: FontWeight.w400,
      fontSize: h2FontSize,
      color: dark,
      decoration: decoration,
      fontStyle: fontStyle,
    );
  }

  static TextStyle h2DarkBold({
    TextDecoration? decoration,
    FontStyle? fontStyle,
  }) {
    return TextStyle(
      fontWeight: FontWeight.w700,
      fontSize: h2FontSize,
      color: dark,
      decoration: decoration,
      fontStyle: fontStyle,
    );
  }

  static TextStyle h2PrimaryRegular({
    TextDecoration? decoration,
    FontStyle? fontStyle,
  }) {
    return TextStyle(
      fontWeight: FontWeight.w400,
      fontSize: h2FontSize,
      color: primary,
      decoration: decoration,
      fontStyle: fontStyle,
    );
  }

  static TextStyle h2PrimaryBold({
    TextDecoration? decoration,
    FontStyle? fontStyle,
  }) {
    return TextStyle(
      fontWeight: FontWeight.w700,
      fontSize: h2FontSize,
      color: primary,
      decoration: decoration,
      fontStyle: fontStyle,
    );
  }

  static TextStyle h2WhiteRegular({
    TextDecoration? decoration,
    FontStyle? fontStyle,
  }) {
    return TextStyle(
      fontWeight: FontWeight.w400,
      fontSize: h2FontSize,
      color: white,
      decoration: decoration,
      fontStyle: fontStyle,
    );
  }

  static TextStyle h2WhiteBold({
    TextDecoration? decoration,
    FontStyle? fontStyle,
  }) {
    return TextStyle(
      fontWeight: FontWeight.w700,
      fontSize: h2FontSize,
      color: white,
      decoration: decoration,
      fontStyle: fontStyle,
    );
  }

  static TextStyle h2DangerRegular({
    TextDecoration? decoration,
    FontStyle? fontStyle,
  }) {
    return TextStyle(
      fontWeight: FontWeight.w400,
      fontSize: h2FontSize,
      color: danger,
      decoration: decoration,
      fontStyle: fontStyle,
    );
  }

  static TextStyle h2DangerBold({
    TextDecoration? decoration,
    FontStyle? fontStyle,
  }) {
    return TextStyle(
      fontWeight: FontWeight.w700,
      fontSize: h2FontSize,
      color: danger,
      decoration: decoration,
      fontStyle: fontStyle,
    );
  }

  static TextStyle h2DisabledRegular({
    TextDecoration? decoration,
    FontStyle? fontStyle,
  }) {
    return TextStyle(
      fontWeight: FontWeight.w400,
      fontSize: h2FontSize,
      color: disabled,
      decoration: decoration,
      fontStyle: fontStyle,
    );
  }

  static TextStyle h2DisabledBold({
    TextDecoration? decoration,
    FontStyle? fontStyle,
  }) {
    return TextStyle(
      fontWeight: FontWeight.w700,
      fontSize: h2FontSize,
      color: disabled,
      decoration: decoration,
      fontStyle: fontStyle,
    );
  }

  // H3 styles
  static TextStyle h3DarkRegular({
    TextDecoration? decoration,
    FontStyle? fontStyle,
  }) {
    return TextStyle(
      fontWeight: FontWeight.w400,
      fontSize: h3FontSize,
      color: dark,
      decoration: decoration,
      fontStyle: fontStyle,
    );
  }

  static TextStyle h3DarkBold({
    TextDecoration? decoration,
    FontStyle? fontStyle,
  }) {
    return TextStyle(
      fontWeight: FontWeight.w700,
      fontSize: h3FontSize,
      color: dark,
      decoration: decoration,
      fontStyle: fontStyle,
    );
  }

  static TextStyle h3PrimaryRegular({
    TextDecoration? decoration,
    FontStyle? fontStyle,
  }) {
    return TextStyle(
      fontWeight: FontWeight.w400,
      fontSize: h3FontSize,
      color: primary,
      decoration: decoration,
      fontStyle: fontStyle,
    );
  }

  static TextStyle h3PrimaryBold({
    TextDecoration? decoration,
    FontStyle? fontStyle,
  }) {
    return TextStyle(
      fontWeight: FontWeight.w700,
      fontSize: h3FontSize,
      color: primary,
      decoration: decoration,
      fontStyle: fontStyle,
    );
  }

  static TextStyle h3WhiteRegular({
    TextDecoration? decoration,
    FontStyle? fontStyle,
  }) {
    return TextStyle(
      fontWeight: FontWeight.w400,
      fontSize: h3FontSize,
      color: white,
      decoration: decoration,
      fontStyle: fontStyle,
    );
  }

  static TextStyle h3WhiteBold({
    TextDecoration? decoration,
    FontStyle? fontStyle,
  }) {
    return TextStyle(
      fontWeight: FontWeight.w700,
      fontSize: h3FontSize,
      color: white,
      decoration: decoration,
      fontStyle: fontStyle,
    );
  }

  static TextStyle h3DangerRegular({
    TextDecoration? decoration,
    FontStyle? fontStyle,
  }) {
    return TextStyle(
      fontWeight: FontWeight.w400,
      fontSize: h3FontSize,
      color: danger,
      decoration: decoration,
      fontStyle: fontStyle,
    );
  }

  static TextStyle h3DangerBold({
    TextDecoration? decoration,
    FontStyle? fontStyle,
  }) {
    return TextStyle(
      fontWeight: FontWeight.w700,
      fontSize: h3FontSize,
      color: danger,
      decoration: decoration,
      fontStyle: fontStyle,
    );
  }

  static TextStyle h3DisabledRegular({
    TextDecoration? decoration,
    FontStyle? fontStyle,
  }) {
    return TextStyle(
      fontWeight: FontWeight.w400,
      fontSize: h3FontSize,
      color: disabled,
      decoration: decoration,
      fontStyle: fontStyle,
    );
  }

  static TextStyle h3DisabledBold({
    TextDecoration? decoration,
    FontStyle? fontStyle,
  }) {
    return TextStyle(
      fontWeight: FontWeight.w700,
      fontSize: h3FontSize,
      color: disabled,
      decoration: decoration,
      fontStyle: fontStyle,
    );
  }

  // H4 styles
  static TextStyle h4DarkRegular({
    TextDecoration? decoration,
    FontStyle? fontStyle,
  }) {
    return TextStyle(
      fontWeight: FontWeight.w400,
      fontSize: h4FontSize,
      color: dark,
      decoration: decoration,
      fontStyle: fontStyle,
    );
  }

  static TextStyle h4DarkBold({
    TextDecoration? decoration,
    FontStyle? fontStyle,
  }) {
    return TextStyle(
      fontWeight: FontWeight.w700,
      fontSize: h4FontSize,
      color: dark,
      decoration: decoration,
      fontStyle: fontStyle,
    );
  }

  static TextStyle h4PrimaryRegular({
    TextDecoration? decoration,
    FontStyle? fontStyle,
  }) {
    return TextStyle(
      fontWeight: FontWeight.w400,
      fontSize: h4FontSize,
      color: primary,
      decoration: decoration,
      fontStyle: fontStyle,
    );
  }

  static TextStyle h4PrimaryBold({
    TextDecoration? decoration,
    FontStyle? fontStyle,
  }) {
    return TextStyle(
      fontWeight: FontWeight.w700,
      fontSize: h4FontSize,
      color: primary,
      decoration: decoration,
      fontStyle: fontStyle,
    );
  }

  static TextStyle h4WhiteRegular({
    TextDecoration? decoration,
    FontStyle? fontStyle,
  }) {
    return TextStyle(
      fontWeight: FontWeight.w400,
      fontSize: h4FontSize,
      color: white,
      decoration: decoration,
      fontStyle: fontStyle,
    );
  }

  static TextStyle h4WhiteBold({
    TextDecoration? decoration,
    FontStyle? fontStyle,
  }) {
    return TextStyle(
      fontWeight: FontWeight.w700,
      fontSize: h4FontSize,
      color: white,
      decoration: decoration,
      fontStyle: fontStyle,
    );
  }

  static TextStyle h4DangerRegular({
    TextDecoration? decoration,
    FontStyle? fontStyle,
  }) {
    return TextStyle(
      fontWeight: FontWeight.w400,
      fontSize: h4FontSize,
      color: danger,
      decoration: decoration,
      fontStyle: fontStyle,
    );
  }

  static TextStyle h4DangerBold({
    TextDecoration? decoration,
    FontStyle? fontStyle,
  }) {
    return TextStyle(
      fontWeight: FontWeight.w700,
      fontSize: h4FontSize,
      color: danger,
      decoration: decoration,
      fontStyle: fontStyle,
    );
  }

  static TextStyle h4DisabledRegular({
    TextDecoration? decoration,
    FontStyle? fontStyle,
  }) {
    return TextStyle(
      fontWeight: FontWeight.w400,
      fontSize: h4FontSize,
      color: disabled,
      decoration: decoration,
      fontStyle: fontStyle,
    );
  }

  static TextStyle h4DisabledBold({
    TextDecoration? decoration,
    FontStyle? fontStyle,
  }) {
    return TextStyle(
      fontWeight: FontWeight.w700,
      fontSize: h4FontSize,
      color: disabled,
      decoration: decoration,
      fontStyle: fontStyle,
    );
  }

  // H5 styles
  static TextStyle h5DarkRegular({
    TextDecoration? decoration,
    FontStyle? fontStyle,
  }) {
    return TextStyle(
      fontWeight: FontWeight.w400,
      fontSize: h5FontSize,
      color: dark,
      decoration: decoration,
      fontStyle: fontStyle,
    );
  }

  static TextStyle h5DarkBold({
    TextDecoration? decoration,
    FontStyle? fontStyle,
  }) {
    return TextStyle(
      fontWeight: FontWeight.w700,
      fontSize: h5FontSize,
      color: dark,
      decoration: decoration,
      fontStyle: fontStyle,
    );
  }

  static TextStyle h5PrimaryRegular({
    TextDecoration? decoration,
    FontStyle? fontStyle,
  }) {
    return TextStyle(
      fontWeight: FontWeight.w400,
      fontSize: h5FontSize,
      color: primary,
      decoration: decoration,
      fontStyle: fontStyle,
    );
  }

  static TextStyle h5PrimaryBold({
    TextDecoration? decoration,
    FontStyle? fontStyle,
  }) {
    return TextStyle(
      fontWeight: FontWeight.w700,
      fontSize: h5FontSize,
      color: primary,
      decoration: decoration,
      fontStyle: fontStyle,
    );
  }

  static TextStyle h5WhiteRegular({
    TextDecoration? decoration,
    FontStyle? fontStyle,
  }) {
    return TextStyle(
      fontWeight: FontWeight.w400,
      fontSize: h5FontSize,
      color: white,
      decoration: decoration,
      fontStyle: fontStyle,
    );
  }

  static TextStyle h5WhiteBold({
    TextDecoration? decoration,
    FontStyle? fontStyle,
  }) {
    return TextStyle(
      fontWeight: FontWeight.w700,
      fontSize: h5FontSize,
      color: white,
      decoration: decoration,
      fontStyle: fontStyle,
    );
  }

  static TextStyle h5DangerRegular({
    TextDecoration? decoration,
    FontStyle? fontStyle,
  }) {
    return TextStyle(
      fontWeight: FontWeight.w400,
      fontSize: h5FontSize,
      color: danger,
      decoration: decoration,
      fontStyle: fontStyle,
    );
  }

  static TextStyle h5DangerBold({
    TextDecoration? decoration,
    FontStyle? fontStyle,
  }) {
    return TextStyle(
      fontWeight: FontWeight.w700,
      fontSize: h5FontSize,
      color: danger,
      decoration: decoration,
      fontStyle: fontStyle,
    );
  }

  static TextStyle h5DisabledRegular({
    TextDecoration? decoration,
    FontStyle? fontStyle,
  }) {
    return TextStyle(
      fontWeight: FontWeight.w400,
      fontSize: h5FontSize,
      color: disabled,
      decoration: decoration,
      fontStyle: fontStyle,
    );
  }

  static TextStyle h5DisabledBold({
    TextDecoration? decoration,
    FontStyle? fontStyle,
  }) {
    return TextStyle(
      fontWeight: FontWeight.w700,
      fontSize: h5FontSize,
      color: disabled,
      decoration: decoration,
      fontStyle: fontStyle,
    );
  }

  // Paragraph styles
  static TextStyle paragraphDarkRegular({
    TextDecoration? decoration,
    FontStyle? fontStyle,
  }) {
    return TextStyle(
      fontWeight: FontWeight.w400,
      fontSize: paragraphFontSize,
      color: dark,
      decoration: decoration,
      fontStyle: fontStyle,
    );
  }

  static TextStyle paragraphDarkBold({
    TextDecoration? decoration,
    FontStyle? fontStyle,
  }) {
    return TextStyle(
      fontWeight: FontWeight.w700,
      fontSize: paragraphFontSize,
      color: dark,
      decoration: decoration,
      fontStyle: fontStyle,
    );
  }

  static TextStyle paragraphPrimaryRegular({
    TextDecoration? decoration,
    FontStyle? fontStyle,
  }) {
    return TextStyle(
      fontWeight: FontWeight.w400,
      fontSize: paragraphFontSize,
      color: primary,
      decoration: decoration,
      fontStyle: fontStyle,
    );
  }

  static TextStyle paragraphPrimaryBold({
    TextDecoration? decoration,
    FontStyle? fontStyle,
  }) {
    return TextStyle(
      fontWeight: FontWeight.w700,
      fontSize: paragraphFontSize,
      color: primary,
      decoration: decoration,
      fontStyle: fontStyle,
    );
  }

  static TextStyle paragraphWhiteRegular({
    TextDecoration? decoration,
    FontStyle? fontStyle,
  }) {
    return TextStyle(
      fontWeight: FontWeight.w400,
      fontSize: paragraphFontSize,
      color: white,
      decoration: decoration,
      fontStyle: fontStyle,
    );
  }

  static TextStyle paragraphWhiteBold({
    TextDecoration? decoration,
    FontStyle? fontStyle,
  }) {
    return TextStyle(
      fontWeight: FontWeight.w700,
      fontSize: paragraphFontSize,
      color: white,
      decoration: decoration,
      fontStyle: fontStyle,
    );
  }

  static TextStyle paragraphDangerRegular({
    TextDecoration? decoration,
    FontStyle? fontStyle,
  }) {
    return TextStyle(
      fontWeight: FontWeight.w400,
      fontSize: paragraphFontSize,
      color: danger,
      decoration: decoration,
      fontStyle: fontStyle,
    );
  }

  static TextStyle paragraphDangerBold({
    TextDecoration? decoration,
    FontStyle? fontStyle,
  }) {
    return TextStyle(
      fontWeight: FontWeight.w700,
      fontSize: paragraphFontSize,
      color: danger,
      decoration: decoration,
      fontStyle: fontStyle,
    );
  }

  static TextStyle paragraphDisabledRegular({
    TextDecoration? decoration,
    FontStyle? fontStyle,
  }) {
    return TextStyle(
      fontWeight: FontWeight.w400,
      fontSize: paragraphFontSize,
      color: disabled,
      decoration: decoration,
      fontStyle: fontStyle,
    );
  }

  static TextStyle paragraphDisabledBold({
    TextDecoration? decoration,
    FontStyle? fontStyle,
  }) {
    return TextStyle(
      fontWeight: FontWeight.w700,
      fontSize: paragraphFontSize,
      color: disabled,
      decoration: decoration,
      fontStyle: fontStyle,
    );
  }

  static TextStyle paragraphDarkGreenRegular({
    TextDecoration? decoration,
    FontStyle? fontStyle,
  }) {
    return TextStyle(
      fontWeight: FontWeight.w400,
      fontSize: paragraphFontSize,
      color: darkGreen,
      decoration: decoration,
      fontStyle: fontStyle,
    );
  }

  static TextStyle paragraphDarkGreenBold({
    TextDecoration? decoration,
    FontStyle? fontStyle,
  }) {
    return TextStyle(
      fontWeight: FontWeight.w700,
      fontSize: paragraphFontSize,
      color: darkGreen,
      decoration: decoration,
      fontStyle: fontStyle,
    );
  }

  static TextStyle paragraphLightGreenRegular({
    TextDecoration? decoration,
    FontStyle? fontStyle,
  }) {
    return TextStyle(
      fontWeight: FontWeight.w400,
      fontSize: paragraphFontSize,
      color: lightGreen,
      decoration: decoration,
      fontStyle: fontStyle,
    );
  }

  static TextStyle paragraphLightGreenBold({
    TextDecoration? decoration,
    FontStyle? fontStyle,
  }) {
    return TextStyle(
      fontWeight: FontWeight.w700,
      fontSize: paragraphFontSize,
      color: lightGreen,
      decoration: decoration,
      fontStyle: fontStyle,
    );
  }

  // Caption styles
  static TextStyle captionDarkRegular({
    TextDecoration? decoration,
    FontStyle? fontStyle,
  }) {
    return TextStyle(
      fontWeight: FontWeight.w400,
      fontSize: captionFontSize,
      color: dark,
      decoration: decoration,
      fontStyle: fontStyle,
    );
  }

  static TextStyle captionDarkBold({
    TextDecoration? decoration,
    FontStyle? fontStyle,
  }) {
    return TextStyle(
      fontWeight: FontWeight.w700,
      fontSize: captionFontSize,
      color: dark,
      decoration: decoration,
      fontStyle: fontStyle,
    );
  }

  static TextStyle captionPrimaryRegular({
    TextDecoration? decoration,
    FontStyle? fontStyle,
  }) {
    return TextStyle(
      fontWeight: FontWeight.w400,
      fontSize: captionFontSize,
      color: primary,
      decoration: decoration,
      fontStyle: fontStyle,
    );
  }

  static TextStyle captionPrimaryBold({
    TextDecoration? decoration,
    FontStyle? fontStyle,
  }) {
    return TextStyle(
      fontWeight: FontWeight.w700,
      fontSize: captionFontSize,
      color: primary,
      decoration: decoration,
      fontStyle: fontStyle,
    );
  }

  static TextStyle captionWhiteRegular({
    TextDecoration? decoration,
    FontStyle? fontStyle,
  }) {
    return TextStyle(
      fontWeight: FontWeight.w400,
      fontSize: captionFontSize,
      color: white,
      decoration: decoration,
      fontStyle: fontStyle,
    );
  }

  static TextStyle captionWhiteBold({
    TextDecoration? decoration,
    FontStyle? fontStyle,
  }) {
    return TextStyle(
      fontWeight: FontWeight.w700,
      fontSize: captionFontSize,
      color: white,
      decoration: decoration,
      fontStyle: fontStyle,
    );
  }

  static TextStyle captionDangerRegular({
    TextDecoration? decoration,
    FontStyle? fontStyle,
  }) {
    return TextStyle(
      fontWeight: FontWeight.w400,
      fontSize: captionFontSize,
      color: danger,
      decoration: decoration,
      fontStyle: fontStyle,
    );
  }

  static TextStyle captionDangerBold({
    TextDecoration? decoration,
    FontStyle? fontStyle,
  }) {
    return TextStyle(
      fontWeight: FontWeight.w700,
      fontSize: captionFontSize,
      color: danger,
      decoration: decoration,
      fontStyle: fontStyle,
    );
  }

  static TextStyle captionDisabledRegular({
    TextDecoration? decoration,
    FontStyle? fontStyle,
  }) {
    return TextStyle(
      fontWeight: FontWeight.w400,
      fontSize: captionFontSize,
      color: disabled,
      decoration: decoration,
      fontStyle: fontStyle,
    );
  }

  static TextStyle captionDisabledBold({
    TextDecoration? decoration,
    FontStyle? fontStyle,
  }) {
    return TextStyle(
      fontWeight: FontWeight.w700,
      fontSize: captionFontSize,
      color: disabled,
      decoration: decoration,
      fontStyle: fontStyle,
    );
  }
}
