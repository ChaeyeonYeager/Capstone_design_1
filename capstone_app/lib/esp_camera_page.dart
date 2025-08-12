import 'dart:io' show Platform;
import 'dart:html' as html; // 웹
import 'dart:ui_web' as ui;

import 'package:flutter/foundation.dart';
import 'package:flutter/material.dart';
import 'package:webview_flutter/webview_flutter.dart'; // ✅ 모바일 전용

class EspCameraPage extends StatefulWidget {
  const EspCameraPage({super.key});

  @override
  State<EspCameraPage> createState() => _EspCameraPageState();
}

class _EspCameraPageState extends State<EspCameraPage> {
  final String _streamUrl = 'http://172.20.10.9/stream';

  WebViewController? _mobileWebViewController;

  @override
  void initState() {
    super.initState();

    // 웹용 iframe 등록
    if (kIsWeb) {
      ui.platformViewRegistry.registerViewFactory('esp32cam-html', (
        int viewId,
      ) {
        final iframe = html.IFrameElement()
          ..src = _streamUrl
          ..style.border = 'none'
          ..style.width = '100%'
          ..style.height = '100%';
        return iframe;
      });
    } else {
      // 모바일 WebView 초기화
      _mobileWebViewController = WebViewController()
        ..setJavaScriptMode(JavaScriptMode.unrestricted)
        ..loadRequest(Uri.parse(_streamUrl));
    }
  }

  @override
  Widget build(BuildContext context) {
    if (kIsWeb) {
      // ✅ 웹
      return Scaffold(
        appBar: AppBar(title: const Text('펫캠 (웹 iframe)')),
        body: const HtmlElementView(viewType: 'esp32cam-html'),
      );
    } else {
      // ✅ 모바일(Android/iOS)
      return Scaffold(
        appBar: AppBar(title: const Text('펫캠 (모바일 WebView)')),
        body: WebViewWidget(controller: _mobileWebViewController!),
      );
    }
  }
}
