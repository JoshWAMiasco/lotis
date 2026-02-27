import 'package:lotis_manager_app/services/ble_service.dart';
import 'package:lotis_manager_app/services/storage_service.dart';
import 'package:lotis_manager_app/services/file_service.dart';
import 'package:riverpod/riverpod.dart';

// Export router
export 'package:lotis_manager_app/router/app_router.dart';

final storageService = StorageService();
final bluetoothService = BleService();
final fileService = FileService();
