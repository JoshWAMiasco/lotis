import 'package:flutter/material.dart';
import 'package:lotis_manager_app/constants/app_font.dart';

class EggHarvestConfirmationDialog extends StatefulWidget {
  const EggHarvestConfirmationDialog({super.key});

  @override
  State<EggHarvestConfirmationDialog> createState() =>
      _EggHarvestConfirmationDialogState();
}

class _EggHarvestConfirmationDialogState
    extends State<EggHarvestConfirmationDialog> {
  @override
  Widget build(BuildContext context) {
    return AlertDialog(
      title: Text('Save Egg Harvest', style: AppFont.h3DarkBold()),
    );
  }
}
