import 'package:flutter/material.dart';
import 'package:firebase_core/firebase_core.dart';
import 'package:firebase_auth/firebase_auth.dart';

import 'firebase_options.dart';
import 'login.dart';
import 'signup.dart';
import 'home.dart'; // 여기에 HomePage 정의돼 있음
import 'input_form_page.dart';
import 'pet_detail_page.dart';
import 'camera.dart';
import 'esp_camera_page.dart';


import 'route_bus.dart';    // ✅ routeBus 싱글턴
import 'mic_overlay.dart'; // ✅ 오버레이

void main() async {
  WidgetsFlutterBinding.ensureInitialized();
  await Firebase.initializeApp(options: DefaultFirebaseOptions.currentPlatform);
  runApp(const MyApp());
}

class MyApp extends StatelessWidget {
  const MyApp({super.key});
  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      title: '반려동물 급식기 app',
      theme: ThemeData(primarySwatch: Colors.blue),
      initialRoute: FirebaseAuth.instance.currentUser == null ? '/' : '/home',
      routes: {
        '/': (context) => const LoginPage(),
        '/signup': (context) => const SignUpPage(),
        '/home': (context) => const HomePage(),
        '/camera': (context) => const CameraCapturePage(),
        '/add': (context) => const InputFormPage(),
        '/detail': (context) => const PetDetailPage(),
        '/esp_camera': (context) => const EspCameraPage(),
      },
      // ✅ 여기!
      navigatorObservers: [routeBus],
      // ✅ 전역 오버레이
      builder: (context, child) {
        return Stack(
          children: [
            child ?? const SizedBox.shrink(),
            const MicOverlay(),
          ],
        );
      },
    );
  }
}
