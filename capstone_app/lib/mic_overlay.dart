// mic_overlay.dart
import 'package:flutter/material.dart';
import 'route_bus.dart';
import 'audio_uploader.dart'; // 녹음/업로드 로직
// mic_overlay.dart (버튼 핸들러 일부)
import 'package:flutter/foundation.dart' show kIsWeb;

// 오버레이가 보일 라우트들 (원하는 화면만 추가하면 됨)
const _visibleRoutes = <String>{
  '/camera', // 카메라 촬영 화면
  '/detail', // 상세 페이지
  '/add', // 입력 폼 등
};

class MicOverlay extends StatefulWidget {
  const MicOverlay({super.key});

  @override
  State<MicOverlay> createState() => _MicOverlayState();
}

class _MicOverlayState extends State<MicOverlay> {
  bool _recording = false;
  Duration _dur = Duration.zero;

  @override
  Widget build(BuildContext context) {
    return ValueListenableBuilder<String?>(
      valueListenable: routeBus.routeName,
      builder: (_, routeName, __) {
        final show = routeName != null && _visibleRoutes.contains(routeName);
        if (!show) return const SizedBox.shrink();

        return Positioned(
          right: 16,
          bottom: 80,
          child: FloatingActionButton.extended(
            heroTag: 'mic-overlay',
            onPressed: _onPressed,
            icon: Icon(_recording ? Icons.stop : Icons.mic),
            label: Text(_recording ? '녹음 중지' : '녹음 시작'),
          ),
        );
      },
    );
  }

  Future<void> _onPressed() async {
    try {
      if (!_recording) {
        await AudioUploader.I.startRecording();
        setState(() => _recording = true);
      } else {
        final res = await AudioUploader.I.stopAndMaybeUpload(upload: !kIsWeb);
        setState(() {
          _recording = false;
          _dur = res.duration;
        });
        if (!mounted) return;
        ScaffoldMessenger.of(
          context,
        ).showSnackBar(SnackBar(content: Text('녹음 완료: ${_dur.inSeconds}s')));
      }
    } catch (e) {
      if (!mounted) return;
      ScaffoldMessenger.of(
        context,
      ).showSnackBar(SnackBar(content: Text('녹음 오류: $e')));
    }
  }
}
