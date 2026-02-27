import 'package:flutter/material.dart';
import 'package:go_router/go_router.dart';
import 'package:intl/intl.dart';

class AppUtils {
  // Date formatters
  static String formatDate(DateTime date) {
    return DateFormat('yyyy-MM-dd').format(date);
  }

  static String formatDateCompleteMonth(DateTime date) {
    return DateFormat('MMMM dd, yyyy').format(date);
  }

  static String formatDateTime(DateTime dateTime) {
    return DateFormat('yyyy-MM-dd HH:mm:ss').format(dateTime);
  }

  static String formatTime(DateTime time) {
    return DateFormat('hh:mm:ss a').format(time);
  }

  // Number formatters
  static String formatCurrency(double amount) {
    return NumberFormat.currency(symbol: '\$').format(amount);
  }

  static String formatNumber(num number) {
    return NumberFormat('#,###').format(number);
  }

  // String utilities
  static String capitalize(String text) {
    if (text.isEmpty) return text;
    return text[0].toUpperCase() + text.substring(1);
  }

  static String truncate(String text, int maxLength) {
    if (text.length <= maxLength) return text;
    return '${text.substring(0, maxLength)}...';
  }

  // Hex encoding/decoding utilities
  static String bytesToHex(List<int> bytes) {
    final StringBuffer buffer = StringBuffer();

    for (final byte in bytes) {
      buffer.write(byte.toRadixString(16).padLeft(2, '0'));
    }

    return buffer.toString().toUpperCase(); // optional uppercase
  }

  static String byteToHex(int value) =>
      value.toRadixString(16).padLeft(2, '0').toUpperCase();

  static List<int> hexToBytes(String hex) {
    hex = hex.replaceAll(' ', '').toLowerCase();

    final List<int> result = [];

    for (int i = 0; i < hex.length; i += 2) {
      final byte = hex.substring(i, i + 2);
      result.add(int.parse(byte, radix: 16));
    }

    return result;
  }

  static void backToHome(BuildContext context) {
    final router = GoRouter.of(context);
    while (router.state.path != '/home' && router.canPop()) {
      router.pop();
    }
  }

  static String intToHex(int value, int padding) {
    return value.toRadixString(16).toUpperCase().padLeft(padding, '0');
  }
}
