// route_bus.dart
import 'package:flutter/material.dart';

class RouteBus extends NavigatorObserver {
  // 현재 라우트 이름을 내보내는 Notifier
  final ValueNotifier<String?> routeName = ValueNotifier<String?>(null);

  @override
  void didPush(Route route, Route<dynamic>? previousRoute) {
    routeName.value = route.settings.name;
    super.didPush(route, previousRoute);
  }

  @override
  void didPop(Route route, Route<dynamic>? previousRoute) {
    routeName.value = previousRoute?.settings.name;
    super.didPop(route, previousRoute);
  }

  @override
  void didReplace({Route<dynamic>? newRoute, Route<dynamic>? oldRoute}) {
    routeName.value = newRoute?.settings.name;
    super.didReplace(newRoute: newRoute, oldRoute: oldRoute);
  }
}

// 어디서나 쓰는 전역 싱글턴
final RouteBus routeBus = RouteBus();
