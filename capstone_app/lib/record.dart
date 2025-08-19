import 'dart:io' show File;
import 'package:flutter/foundation.dart';
import 'package:flutter/material.dart';
import 'package:record/record.dart';
import 'package:firebase_storage/firebase_storage.dart';
import 'package:path_provider/path_provider.dart';
import 'package:cloud_firestore/cloud_firestore.dart';
import 'package:file_picker/file_picker.dart'; // ✅ 추가

class RecordPage extends StatefulWidget {
  final DocumentSnapshot pet;
  const RecordPage({super.key, required this.pet});

  @override
  State<RecordPage> createState() => _RecordPageState();
}

class _RecordPageState extends State<RecordPage> {
  final _recorder = AudioRecorder();

  bool _isRecording = false;
  bool _hasPermission = false;
  String? _filePath;      // 모바일 녹음 파일 경로
  String? _pickedPath;    // 웹/모바일 공통: 파일 선택 경로
  String _status = '대기 중';
  AudioEncoder? _encoder;

  @override
  void initState() {
    super.initState();
    _init();
  }

  Future<void> _init() async {
    final perm = await _recorder.hasPermission();
    final supportsAac = await _recorder.isEncoderSupported(AudioEncoder.aacLc);
    final supportsOpus = await _recorder.isEncoderSupported(AudioEncoder.opus);
    final supportsWav = await _recorder.isEncoderSupported(AudioEncoder.wav);

    _encoder = supportsAac
        ? AudioEncoder.aacLc
        : supportsOpus
            ? AudioEncoder.opus
            : supportsWav
                ? AudioEncoder.wav
                : null;

    setState(() {
      _hasPermission = perm;
      _status = '권한:${perm ? "OK" : "거부"} / 인코더:${_encoder ?? "없음"}';
    });
  }

  @override
  void dispose() {
    _recorder.dispose();
    super.dispose();
  }

  String _ext(AudioEncoder enc) {
    switch (enc) {
      case AudioEncoder.aacLc: return 'm4a';
      case AudioEncoder.opus:  return 'webm';
      case AudioEncoder.wav:   return 'wav';
      default: return 'm4a';
    }
  }

  Future<void> _startRecording() async {
    try {
      if (kIsWeb) {
        // 웹은 녹음 자체는 되더라도 파일 추출이 제한적 → 업로드는 파일 선택으로 처리 권장
        ScaffoldMessenger.of(context).showSnackBar(
          const SnackBar(content: Text('웹은 “파일에서 업로드” 버튼을 사용해주세요.')),
        );
        return;
      }
      if (!await _recorder.hasPermission()) {
        ScaffoldMessenger.of(context).showSnackBar(
          const SnackBar(content: Text('마이크 권한이 필요합니다.')),
        );
        return;
      }
      if (await _recorder.isRecording()) await _recorder.stop();

      final enc = _encoder ?? AudioEncoder.aacLc;
      final dir = await getTemporaryDirectory();
      _filePath = '${dir.path}/voice_${DateTime.now().millisecondsSinceEpoch}.${_ext(enc)}';

      await _recorder.start(
        RecordConfig(encoder: enc, bitRate: enc == AudioEncoder.wav ? 0 : 128000, sampleRate: 44100),
        path: _filePath!,
      );

      await Future.delayed(const Duration(milliseconds: 200));
      _isRecording = await _recorder.isRecording();

      setState(() => _status = _isRecording ? '🎙️ 녹음 중...' : '녹음 시작 실패');
    } catch (e) {
      setState(() => _status = '녹음 시작 오류: $e');
    }
  }

  Future<void> _stopRecording() async {
    try {
      await _recorder.stop();
      setState(() {
        _isRecording = false;
        _status = '녹음 종료';
      });
    } catch (e) {
      setState(() => _status = '녹음 종료 오류: $e');
    }
  }

  // ✅ 파일 선택(웹/모바일 공통)
  Future<void> _pickFile() async {
    final res = await FilePicker.platform.pickFiles(
      type: FileType.custom,
      allowedExtensions: ['m4a', 'mp3', 'wav', 'ogg', 'webm'],
      withData: kIsWeb, // 웹은 bytes로 온다
    );
    if (res == null || res.files.isEmpty) return;

    final f = res.files.single;
    if (kIsWeb) {
      // 웹: 메모리 상의 bytes만 존재
      if (f.bytes == null) {
        setState(() => _status = '파일 선택 실패: 바이트 없음');
        return;
      }
      // 임시 경로 개념 대신 파일명만 기억
      _pickedPath = f.name;
      setState(() => _status = '파일 선택됨(웹): ${f.name}');
    } else {
      // 모바일/데스크탑
      _pickedPath = f.path;
      setState(() => _status = '파일 선택됨: ${f.name}');
    }
  }

  Future<String?> _uploadToFirebase() async {
    try {
      final petId = widget.pet.id;
      final refBase = FirebaseStorage.instance.ref().child('voices/$petId');

      // 1) 우선순위: 녹음 파일(모바일)
      if (!kIsWeb && _filePath != null) {
        final file = File(_filePath!);
        if (file.existsSync()) {
          final ext = _filePath!.split('.').last;
          final ref = refBase.child('call.$ext');
          await ref.putFile(file);
          return await ref.getDownloadURL();
        }
      }

      // 2) 선택 파일 업로드(웹/모바일 공통)
      final res = await FilePicker.platform.pickFiles(
        type: FileType.custom,
        allowedExtensions: ['m4a', 'mp3', 'wav', 'ogg', 'webm'],
        withData: kIsWeb,
      );
      if (res == null || res.files.isEmpty) {
        setState(() => _status = '업로드 취소됨');
        return null;
      }
      final f = res.files.single;

      if (kIsWeb) {
        if (f.bytes == null) throw '웹 파일 바이트 없음';
        final ext = (f.extension ?? 'webm');
        final ref = refBase.child('call.$ext');
        await ref.putData(f.bytes!, SettableMetadata(contentType: 'audio/$ext'));
        return await ref.getDownloadURL();
      } else {
        if (f.path == null) throw '파일 경로 없음';
        final file = File(f.path!);
        final ext = (f.extension ?? 'm4a');
        final ref = refBase.child('call.$ext');
        await ref.putFile(file);
        return await ref.getDownloadURL();
      }
    } catch (e) {
      setState(() => _status = '업로드 실패: $e');
      return null;
    }
  }

  @override
  Widget build(BuildContext context) {
    final isWeb = kIsWeb;
    return Scaffold(
      appBar: AppBar(title: const Text('음성 녹음')),
      body: Center(
        child: Column(
          mainAxisSize: MainAxisSize.min,
          children: [
            Icon(_isRecording ? Icons.mic : Icons.mic_none,
                color: _isRecording ? Colors.red : Colors.grey, size: 100),
            const SizedBox(height: 8),
            Text('상태: $_status'),
            const SizedBox(height: 16),

            // 모바일 녹음 컨트롤
            if (!isWeb) Row(
              mainAxisSize: MainAxisSize.min,
              children: [
                ElevatedButton(onPressed: _isRecording ? null : _startRecording, child: const Text('녹음 시작')),
                const SizedBox(width: 12),
                ElevatedButton(onPressed: _isRecording ? _stopRecording : null, child: const Text('녹음 종료')),
              ],
            ),

            // 파일 업로드(웹/모바일 공통) — 웹 필수 경로
            const SizedBox(height: 12),
            OutlinedButton.icon(
              icon: const Icon(Icons.upload_file),
              label: Text(isWeb ? '파일에서 업로드(웹 권장)' : '파일에서 업로드'),
              onPressed: _pickFile,
            ),

            const SizedBox(height: 16),
            ElevatedButton.icon(
              icon: const Icon(Icons.cloud_upload),
              label: const Text('저장 & 업로드'),
              onPressed: () async {
                if (_isRecording) await _stopRecording();
                final url = await _uploadToFirebase();
                if (url != null && mounted) {
                  Navigator.pop(context, {'voiceUrl': url});
                }
              },
            ),
            const SizedBox(height: 8),
            const Text('저장 시 이 반려동물의 호출 음성으로 등록됩니다.'),
            if (isWeb)
              const Padding(
                padding: EdgeInsets.only(top: 10),
                child: Text(
                  '웹은 브라우저 정책상 녹음 파일 직업로드가 제한될 수 있어 파일 선택 업로드를 권장합니다.',
                  textAlign: TextAlign.center,
                  style: TextStyle(fontSize: 12, color: Colors.grey),
                ),
              ),
          ],
        ),
      ),
    );
  }
}
