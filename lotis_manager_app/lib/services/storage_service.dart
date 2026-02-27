import 'package:shared_preferences/shared_preferences.dart';

class StorageService {
  SharedPreferences? _prefs;

  Future<void> init() async {
    _prefs = await SharedPreferences.getInstance();
  }

  // String operations
  Future<bool> setString(String key, String value) async {
    if (_prefs == null) await init();
    return await _prefs!.setString(key, value);
  }

  String? getString(String key) {
    return _prefs?.getString(key);
  }

  // Int operations
  Future<bool> setInt(String key, int value) async {
    if (_prefs == null) await init();
    return await _prefs!.setInt(key, value);
  }

  int? getInt(String key) {
    return _prefs?.getInt(key);
  }

  // Bool operations
  Future<bool> setBool(String key, bool value) async {
    if (_prefs == null) await init();
    return await _prefs!.setBool(key, value);
  }

  bool? getBool(String key) {
    return _prefs?.getBool(key);
  }

  // Double operations
  Future<bool> setDouble(String key, double value) async {
    if (_prefs == null) await init();
    return await _prefs!.setDouble(key, value);
  }

  double? getDouble(String key) {
    return _prefs?.getDouble(key);
  }

  // Remove key
  Future<bool> remove(String key) async {
    if (_prefs == null) await init();
    return await _prefs!.remove(key);
  }

  // Clear all
  Future<bool> clear() async {
    if (_prefs == null) await init();
    return await _prefs!.clear();
  }

  // Check if key exists
  bool containsKey(String key) {
    return _prefs?.containsKey(key) ?? false;
  }

  // User session management
  Future<bool> setToken(String token) async {
    return await setString('auth_token', token);
  }

  String? getToken() {
    return getString('auth_token');
  }

  Future<bool> setUserId(String userId) async {
    return await setString('user_id', userId);
  }

  String? getUserId() {
    return getString('user_id');
  }

  Future<bool> isLoggedIn() async {
    final token = getToken();
    return token != null && token.isNotEmpty;
  }

  Future<bool> logout() async {
    await remove('auth_token');
    await remove('user_id');
    return true;
  }
}
