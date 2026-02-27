import 'dart:io';
import 'package:path_provider/path_provider.dart';

class FileService {
  Future<void> init() async {
    // Initialize file service
  }

  Future<String> getAppDocumentsDirectory() async {
    final directory = await getApplicationDocumentsDirectory();
    return directory.path;
  }

  Future<String> getAppCacheDirectory() async {
    final directory = await getTemporaryDirectory();
    return directory.path;
  }

  Future<File> writeFile(String fileName, String content) async {
    final directory = await getApplicationDocumentsDirectory();
    final file = File('${directory.path}/$fileName');
    return await file.writeAsString(content);
  }

  Future<String> readFile(String fileName) async {
    final directory = await getApplicationDocumentsDirectory();
    final file = File('${directory.path}/$fileName');

    if (await file.exists()) {
      return await file.readAsString();
    } else {
      throw Exception('File not found: $fileName');
    }
  }

  Future<bool> fileExists(String fileName) async {
    final directory = await getApplicationDocumentsDirectory();
    final file = File('${directory.path}/$fileName');
    return await file.exists();
  }

  Future<void> deleteFile(String fileName) async {
    final directory = await getApplicationDocumentsDirectory();
    final file = File('${directory.path}/$fileName');

    if (await file.exists()) {
      await file.delete();
    }
  }

  Future<List<FileSystemEntity>> listFiles() async {
    final directory = await getApplicationDocumentsDirectory();
    return directory.listSync();
  }
}
