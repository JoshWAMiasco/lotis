import 'package:flutter/widgets.dart';
import 'package:go_router/go_router.dart';
import 'package:lotis_manager_app/screens/egg_harvest_screen.dart';
import 'package:lotis_manager_app/screens/harvest_screen.dart';
import 'package:lotis_manager_app/screens/home_screen.dart';
import 'package:lotis_manager_app/screens/root_screen.dart';
import 'package:lotis_manager_app/screens/sell_egg_screen.dart';
import 'package:lotis_manager_app/screens/sell_screen.dart';
import 'package:lotis_manager_app/screens/set_datetime_screen.dart';
import 'package:lotis_manager_app/screens/settings_screen.dart';
import 'package:lotis_manager_app/screens/switch_screens.dart';

final appRouter = GoRouter(
  initialLocation: '/',
  routes: [
    GoRoute(path: '/', builder: (context, state) => const RootScreen()),
    GoRoute(
      path: '/home',
      name: 'home',
      builder: (context, state) {
        return HomeScreen();
      },
      routes: [
        GoRoute(
          path: 'harvest',
          name: 'harvest',
          builder: (context, state) {
            return HarvestScreen();
          },
          routes: [
            GoRoute(
              path: 'egg_harvest',
              name: 'egg_harvest',
              builder: (context, state) {
                return EggHarvestScreen();
              },
            ),
          ],
        ),
        GoRoute(
          path: 'sell',
          name: 'sell',
          builder: (context, state) {
            return SellScreen();
          },
          routes: [
            GoRoute(
              path: 'sell_egg',
              name: 'sell_egg',
              builder: (context, state) {
                return SellEggScreen();
              },
            ),
          ],
        ),
        GoRoute(
          path: 'settings',
          name: 'settings',
          builder: (context, state) {
            return SettingsScreen();
          },
          routes: [
            GoRoute(
              path: 'set-date-time',
              name: 'set-date-time',
              builder: (context, state) {
                return SetDatetimeScreen();
              },
            ),
          ],
        ),
        GoRoute(
          path: 'switch',
          name: 'switch',
          builder: (context, state) {
            return SwitchScreens();
          },
        ),
      ],
    ),
  ],
);
