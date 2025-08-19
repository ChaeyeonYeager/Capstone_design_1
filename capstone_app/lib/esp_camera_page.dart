import 'dart:async';
import 'dart:convert';
import 'dart:html' as html; // 웹
import 'dart:ui_web' as ui;

import 'package:flutter/foundation.dart';
import 'package:flutter/material.dart';
import 'package:webview_flutter/webview_flutter.dart';
import 'package:http/http.dart' as http;

const String kServerBase = 'http://192.168.0.207:8000';          // ★ 수정
const String kStatusUrl  = '$kServerBase/status';
const String kRawUrl     = 'http://192.168.0.203:4747/video'; // DroidCam 원본

class EspCameraPage extends StatefulWidget {
  const EspCameraPage({super.key});

  @override
  State<EspCameraPage> createState() => _EspCameraPageState();
}

class _EspCameraPageState extends State<EspCameraPage> {
  bool _detectOn = true; // 분석 스트림 사용 여부
  WebViewController? _mobileCtrl;
  html.IFrameElement? _iframe;

  Timer? _statusTimer;
  Map<String, dynamic>? _status;

  String get _currentUrl =>
      _detectOn ? '$kServerBase/stream?src=${Uri.encodeComponent(kRawUrl)}'
                : kRawUrl;

  @override
  void initState() {
    super.initState();

    // View init
    if (kIsWeb) {
      _iframe = html.IFrameElement()
        ..src = _currentUrl
        ..style.border = 'none'
        ..style.width = '100%'
        ..style.height = '100%';
      ui.platformViewRegistry.registerViewFactory(
        'esp32cam-html',
        (int viewId) => _iframe!,
      );
    } else {
      _mobileCtrl = WebViewController()
        ..setJavaScriptMode(JavaScriptMode.unrestricted)
        ..loadRequest(Uri.parse(_currentUrl));
    }

    // status polling
    _statusTimer = Timer.periodic(const Duration(seconds: 1), (_) => _fetchStatus());
  }

  @override
  void dispose() {
    _statusTimer?.cancel();
    super.dispose();
  }

  Future<void> _fetchStatus() async {
    if (!_detectOn) {
      if (_status != null) setState(() => _status = null);
      return;
    }
    try {
      final res = await http.get(Uri.parse(kStatusUrl)).timeout(const Duration(seconds: 2));
      if (res.statusCode == 200) {
        final data = json.decode(res.body) as Map<String, dynamic>;
        if (mounted) setState(() => _status = data);
      }
    } catch (_) {}
  }

  void _reload() {
    if (kIsWeb) {
      _iframe?.src = _currentUrl;
      setState(() {}); // 타이틀만 갱신
    } else {
      _mobileCtrl?.loadRequest(Uri.parse(_currentUrl));
    }
  }

  @override
  Widget build(BuildContext context) {
    final title = _detectOn ? '펫캠 (탐지 ON)' : '펫캠 (원본)';

    final viewer = kIsWeb
        ? const HtmlElementView(viewType: 'esp32cam-html')
        : WebViewWidget(controller: _mobileCtrl!);

    Widget namesPill() {
      if (!_detectOn) return const SizedBox.shrink();
      final dets = (_status?['detections'] as List?) ?? const [];
      if (dets.isEmpty) {
        return _pill('탐지 대상 없음', Colors.redAccent);
      }
      // 라벨별 최고 점수만 추려서 표시
      final bestByName = <String, double>{};
      for (final e in dets) {
        final m = e as Map<String, dynamic>;
        final label = (m['label'] ?? '').toString();
        final score = (m['score'] ?? 0.0).toDouble();
        final prev = bestByName[label] ?? 0.0;
        if (score > prev) bestByName[label] = score;
      }
      final items = bestByName.entries.toList()
        ..sort((a, b) => b.value.compareTo(a.value));
      final text = items
          .map((e) => '${e.key}(${(e.value * 100).toStringAsFixed(0)}%)')
          .join(', ');
      return _pill('$text 발견', Colors.green);
    }

    Widget eatingPill() {
      if (!_detectOn) return const SizedBox.shrink();
      final eating = _status?['eating'] == true;
      final nearAny = _status?['near_bowl'] == true;

      if (eating) {
        return _pill('🍽️ 식사 중', Colors.green);
      } else if (nearAny) {
        return _pill('급식기 근처', Colors.orange);
      }
      return const SizedBox.shrink();
    }

    return Scaffold(
      appBar: AppBar(
        title: Text(title),
        actions: [
          Row(children: [
            const Text('탐지'),
            Switch(
              value: _detectOn,
              onChanged: (v) {
                setState(() => _detectOn = v);
                _reload();
              },
            ),
            const SizedBox(width: 8),
          ]),
        ],
      ),
      body: Stack(
        children: [
          Positioned.fill(child: viewer),
          Positioned(
            left: 12,
            top: 12,
            child: Column(
              crossAxisAlignment: CrossAxisAlignment.start,
              children: [
                namesPill(),
                const SizedBox(height: 8),
                eatingPill(),
              ],
            ),
          ),
        ],
      ),
    );
  }

  Widget _pill(String text, Color color) {
    return Container(
      padding: const EdgeInsets.symmetric(horizontal: 12, vertical: 8),
      decoration: BoxDecoration(
        color: color.withOpacity(0.85),
        borderRadius: BorderRadius.circular(999),
      ),
      child: Text(
        text,
        style: const TextStyle(color: Colors.white, fontWeight: FontWeight.bold),
      ),
    );
  }
}
