// audio_uploader.dart (WEB-SAFE, null-safe)
import 'package:flutter/foundation.dart' show kIsWeb;
import 'package:record/record.dart';
// import 'package:path_provider/path_provider.dart'; // 모바일에서만 사용
// import 'dart:io'; // 모바일에서만 사용
import 'package:firebase_storage/firebase_storage.dart';

class AudioResult {
  final String pathOrUrl;
  final Duration duration;
  final String? downloadUrl;
  AudioResult({
    required this.pathOrUrl,
    required this.duration,
    this.downloadUrl,
  });
}

class AudioUploader {
  AudioUploader._();
  static final AudioUploader I = AudioUploader._();

  final _rec = AudioRecorder();
  String? _path; // 모바일에서만 사용 예정
  DateTime? _startedAt;

  Future<void> startRecording() async {
    if (!await _rec.hasPermission()) {
      throw Exception('마이크 권한이 없습니다.');
    }
    _startedAt = DateTime.now();

    final dummyWebName = 'web_record_${DateTime.now().millisecondsSinceEpoch}.m4a';

    await _rec.start(
      const RecordConfig(
        encoder: AudioEncoder.aacLc,
        bitRate: 128000,
        sampleRate: 44100,
      ),
      path: kIsWeb ? dummyWebName : (_path ?? ''), 
      // 모바일에서는 추후 IO 버전에서 _path 실제 경로 세팅
    );
  }

  Future<AudioResult> stopAndMaybeUpload({bool upload = false}) async {
    final stopped = await _rec.stop();
    final dur = DateTime.now().difference(_startedAt ?? DateTime.now());
    _startedAt = null;

    if (upload && kIsWeb) {
      throw Exception('웹 업로드는 별도 구현 필요 (bytes 추출 → putData)');
    }

    return AudioResult(
      pathOrUrl: stopped ?? '',
      duration: dur,
      downloadUrl: null,
    );
  }
}
