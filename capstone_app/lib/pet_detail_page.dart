import 'dart:typed_data';
import 'dart:async';
import 'dart:convert';

import 'package:flutter/foundation.dart';
import 'package:flutter/material.dart';
import 'package:cloud_firestore/cloud_firestore.dart';
import 'package:firebase_storage/firebase_storage.dart';
import 'package:http/http.dart' as http;
import 'package:audioplayers/audioplayers.dart';

import 'pet_edit_page.dart';
import 'record.dart';

/// ---------- 여러 마리 지원: 개별 탐지 항목 ----------
class DetectionItem {
  final String label;            // 이름(라벨)
  final double score;            // 매칭 점수(0~1)
  final bool nearBowl;           // 급식기 ROI 근처인지
  final List<dynamic>? box;      // [x1,y1,x2,y2] (옵션)

  DetectionItem({
    required this.label,
    required this.score,
    required this.nearBowl,
    this.box,
  });

  factory DetectionItem.fromJson(Map<String, dynamic> json) {
    return DetectionItem(
      label: (json['label'] ?? '').toString(),
      score: (json['score'] ?? 0).toDouble(),
      nearBowl: json['near_bowl'] ?? false,
      box: json['box'] as List<dynamic>?,
    );
  }
}

/// ---------- /status 전체 ----------
class DetectionStatus {
  final bool found;                  // 임계점 이상 탐지 존재?
  final double score;                // 베스트 스코어
  final String? label;               // 베스트 라벨
  final List<DetectionItem> detections; // ★ 여러 마리 결과 리스트
  final bool nearBowlAny;            // 어떤 대상이라도 그릇 근처?
  final bool eating;                 // 식사 중 판단

  DetectionStatus({
    required this.found,
    required this.score,
    this.label,
    required this.detections,
    required this.nearBowlAny,
    required this.eating,
  });

  factory DetectionStatus.fromJson(Map<String, dynamic> json) {
    final list = (json['detections'] as List? ?? [])
        .map((e) => DetectionItem.fromJson(e as Map<String, dynamic>))
        .toList();

    return DetectionStatus(
      found: json['found'] ?? false,
      score: (json['score'] ?? 0).toDouble(),
      label: json['label'],
      detections: list,
      nearBowlAny: json['near_bowl'] ?? false,
      eating: json['eating'] ?? false,
    );
  }
}

class PetDetailPage extends StatefulWidget {
  const PetDetailPage({super.key});

  @override
  State<PetDetailPage> createState() => _PetDetailPageState();
}

class _PetDetailPageState extends State<PetDetailPage> {
  // Firebase Storage 버킷
  static const _bucket = 'gs://capstone-c8066.firebasestorage.app';

  // 웹에서 촬영 직후 미리보기 바이트 캐시
  Uint8List? _webFaceBytes;
  Uint8List? _webBodyBytes;

  // 자동 등록 상태
  bool _registeredTargets = false;

  // 앱 실행 중 최초 한 번만 /targets reset=true 하도록(여러 마리 추가 등록 지원)
  static bool _didResetOnce = false;

  // 🔧 분석 서버 URL (맥북 IP로 바꿔줘!)
  final String _serverBaseUrl = 'http://192.168.0.207:8000';
  final String _statusUrl     = 'http://192.168.0.207:8000/status';

  // 상태 폴링
  DetectionStatus? _detectionStatus;
  Timer? _statusTimer;

  // 오디오 플레이어 (웹/안드/ios 공통)
  late final AudioPlayer _player;

  @override
  void initState() {
    super.initState();
    _player = AudioPlayer();
    _startStatusPolling();
  }

  void _startStatusPolling() {
    _statusTimer = Timer.periodic(const Duration(seconds: 1), (_) => _fetchDetectionStatus());
  }

  Future<void> _fetchDetectionStatus() async {
    try {
      final res = await http.get(Uri.parse(_statusUrl)).timeout(const Duration(seconds: 2));
      if (res.statusCode == 200) {
        final jsonData = jsonDecode(res.body) as Map<String, dynamic>;
        final status = DetectionStatus.fromJson(jsonData);
        if (mounted) {
          setState(() => _detectionStatus = status);
        }
      }
    } catch (_) {
      // 네트워크 에러는 조용히 무시
    }
  }

  @override
  void dispose() {
    _statusTimer?.cancel();
    _player.dispose();
    super.dispose();
  }

  @override
  Widget build(BuildContext context) {
    final args = ModalRoute.of(context)?.settings.arguments;
    if (args == null || args is! DocumentSnapshot) {
      return const Scaffold(body: Center(child: Text('잘못된 접근입니다')));
    }
    final DocumentSnapshot pet = args;
    final petDocRef = pet.reference;

    return StreamBuilder<DocumentSnapshot>(
      stream: petDocRef.snapshots(),
      builder: (context, snap) {
        if (!snap.hasData) {
          return const Scaffold(
            body: Center(child: CircularProgressIndicator()),
          );
        }

        final petData = snap.data!;
        final data = petData.data() as Map<String, dynamic>? ?? {};

        final faceUrl = data['faceImageUrl'] as String?;
        final bodyUrl = data['bodyImageUrl'] as String?;
        final facePath = data['faceImagePath'] as String?;
        final bodyPath = data['bodyImagePath'] as String?;
        final voiceUrl = data['voiceUrl'] as String?; // 🔊 저장된 호출 음성

        // ✅ 자동 등록 (한 번만): 첫 등록만 reset=true, 이후는 append
        if (!_registeredTargets && (faceUrl?.isNotEmpty == true || bodyUrl?.isNotEmpty == true)) {
          _registeredTargets = true;

          final urls = <String>[];
          if (faceUrl != null && faceUrl.isNotEmpty) urls.add(faceUrl);
          if (bodyUrl != null && bodyUrl.isNotEmpty) urls.add(bodyUrl);

          final label = (data['name'] ?? 'pet').toString();

          // 첫 호출 때만 리셋, 이후 실행들(다른 동물들)은 append
          final resetThisTime = !_didResetOnce;
          _registerTargetsToServer(
            serverBaseUrl: _serverBaseUrl,
            imageUrls: urls,
            label: label,
            reset: resetThisTime,
          ).then((ok) {
            if (ok && resetThisTime) _didResetOnce = true;
          });
        }

        return Scaffold(
          appBar: AppBar(
            title: Text('${data['name'] ?? '반려동물'}의 정보'),
            actions: [
              IconButton(
                icon: const Icon(Icons.edit),
                onPressed: () {
                  Navigator.push(
                    context,
                    MaterialPageRoute(builder: (_) => PetEditPage(pet: petData)),
                  );
                },
              ),
            ],
          ),
          body: Padding(
            padding: const EdgeInsets.all(16),
            child: ListView(
              children: [
                _buildDetectionStatusWidget(),
                const SizedBox(height: 16),

                // 얼굴/몸통 이미지
                Text('얼굴 사진', style: Theme.of(context).textTheme.titleMedium),
                const SizedBox(height: 8),
                _storageImage(url: faceUrl, path: facePath, webBytes: _webFaceBytes, height: 180),
                const SizedBox(height: 16),

                Text('몸통 사진', style: Theme.of(context).textTheme.titleMedium),
                const SizedBox(height: 8),
                _storageImage(url: bodyUrl, path: bodyPath, webBytes: _webBodyBytes, height: 180),
                const SizedBox(height: 24),

                // 반려동물 정보
                Text('이름: ${data['name'] ?? '정보 없음'}'),
                Text('몸무게: ${data['weight'] ?? '-'} kg'),
                Text('나이: ${data['age'] ?? '-'} 세'),
                Text('활동 수준: ${data['activityLevel'] ?? '-'}'),
                Text('급식 횟수: ${data['feedCount'] ?? '-'} 회'),
                Text('급식 시간: ${data['feedTimes'] is List ? (data['feedTimes'] as List).join(", ") : "등록되지 않음"}'),
                Text('100g당 칼로리: ${data['kcalPer100g'] ?? '-'} kcal'),
                Text('유동성 단계: ${data['viscosityLevel'] ?? '-'}'),

                const SizedBox(height: 24),

                // 🔊 녹음된 음성 재생 UI
                if (voiceUrl != null && voiceUrl.isNotEmpty) ...[
                  Text('📢 등록된 호출 음성', style: Theme.of(context).textTheme.titleMedium),
                  const SizedBox(height: 8),
                  Row(
                    children: [
                      ElevatedButton.icon(
                        icon: const Icon(Icons.play_arrow),
                        label: const Text('재생'),
                        onPressed: () async {
                          await _player.stop();
                          await _player.play(UrlSource(voiceUrl));
                        },
                      ),
                      const SizedBox(width: 12),
                      OutlinedButton.icon(
                        icon: const Icon(Icons.stop),
                        label: const Text('정지'),
                        onPressed: () async {
                          await _player.stop();
                        },
                      ),
                    ],
                  ),
                ],
              ],
            ),
          ),

          // 🔹 녹음 버튼 + 카메라 버튼 2개로
          floatingActionButton: Column(
            mainAxisSize: MainAxisSize.min,
            children: [
              FloatingActionButton(
                heroTag: 'recordBtn',
                tooltip: '음성 녹음',
                child: const Icon(Icons.mic),
                onPressed: () async {
                  final result = await Navigator.push(
                    context,
                    MaterialPageRoute(builder: (_) => RecordPage(pet: petData)),
                  );

                  if (result is Map<String, dynamic>) {
                    final newVoiceUrl = result['voiceUrl'] as String?;
                    if (newVoiceUrl != null && newVoiceUrl.isNotEmpty) {
                      await pet.reference.update({'voiceUrl': newVoiceUrl});
                      if (!mounted) return;
                      ScaffoldMessenger.of(context).showSnackBar(
                        const SnackBar(content: Text('🎤 음성 업로드 완료!')),
                      );
                    }
                  }
                },
              ),
              const SizedBox(height: 12),
              FloatingActionButton(
                heroTag: 'cameraBtn',
                tooltip: '사진 촬영',
                child: const Icon(Icons.camera_alt),
                onPressed: () async {
                  final result = await Navigator.pushNamed(context, '/camera');
                  if (!mounted) return;

                  if (result is Map<String, dynamic>) {
                    final url = result['url'] as String?;
                    final bytes = result['bytes'] as Uint8List?;
                    final part = (result['part'] as String?) ?? 'face'; // 'face' | 'body'
                    final path = result['path'] as String?;

                    final urlField = part == 'body' ? 'bodyImageUrl' : 'faceImageUrl';
                    final pathField = part == 'body' ? 'bodyImagePath' : 'faceImagePath';

                    if (url != null) {
                      await pet.reference.update({
                        urlField: url,
                        if (path != null) pathField: path,
                      });

                      // 사진 바뀌면 다시 등록되도록 초기화
                      _registeredTargets = false;
                    }

                    setState(() {
                      if (kIsWeb && bytes != null) {
                        if (part == 'body') {
                          _webBodyBytes = bytes;
                        } else {
                          _webFaceBytes = bytes;
                        }
                      }
                    });

                    ScaffoldMessenger.of(context).showSnackBar(
                      SnackBar(content: Text('📸 ${part == "body" ? "몸통" : "얼굴"} 사진 업로드 완료!')),
                    );
                  }
                },
              ),
            ],
          ),
        );
      },
    );
  }

  /// ---------- 여러 마리 등록 지원: reset 파라미터 추가 ----------
  Future<bool> _registerTargetsToServer({
    required String serverBaseUrl,
    required List<String> imageUrls,
    required String label,
    required bool reset,
  }) async {
    try {
      final uri = Uri.parse('$serverBaseUrl/targets');
      final request = http.MultipartRequest('POST', uri)
        ..fields['urls'] = imageUrls.join(',')
        ..fields['label'] = label
        ..fields['reset'] = reset.toString(); // "true"/"false"

      final response = await request.send();
      if (response.statusCode == 200) {
        debugPrint('✅ 타깃 등록 성공: label=$label, reset=$reset');
        return true;
      } else {
        final body = await response.stream.bytesToString();
        debugPrint('❌ 타깃 등록 실패(${response.statusCode}): $body');
        return false;
      }
    } catch (e) {
      debugPrint('❌ 타깃 등록 중 오류: $e');
      return false;
    }
  }

  /// ---------- 상태 표시 위젯(여러 마리) ----------
  Widget _buildDetectionStatusWidget() {
    if (_detectionStatus == null) {
      return const Text('상태 불러오는 중...');
    }

    final ds = _detectionStatus!;

    if (!ds.found || ds.detections.isEmpty) {
      return const Text(
        '탐지 대상 없음',
        style: TextStyle(color: Colors.redAccent, fontWeight: FontWeight.bold, fontSize: 16),
      );
    }

    // 같은 이름이 여러 박스로 잡히면 최고 점수만 반영
    final bestByName = <String, double>{};
    final nearNames = <String>{};

    for (final d in ds.detections) {
      final prev = bestByName[d.label] ?? 0.0;
      if (d.score > prev) bestByName[d.label] = d.score;
      if (d.nearBowl) nearNames.add(d.label);
    }

    final items = bestByName.entries.toList()
      ..sort((a, b) => b.value.compareTo(a.value));

    final foundLine = items
        .map((e) => '${e.key}(${(e.value * 100).toStringAsFixed(0)}%)')
        .join(', ');

    final eatingLine = ds.eating
        ? '🍽️ 식사 중'
        : (nearNames.isNotEmpty ? '급식기 근처: ${nearNames.join(", ")}' : null);

    return Column(
      crossAxisAlignment: CrossAxisAlignment.start,
      children: [
        Text(
          '$foundLine 발견',
          style: const TextStyle(color: Colors.green, fontWeight: FontWeight.bold, fontSize: 16),
        ),
        if (eatingLine != null)
          Padding(
            padding: const EdgeInsets.only(top: 4),
            child: Text(
              eatingLine,
              style: const TextStyle(color: Colors.orange, fontWeight: FontWeight.w600),
            ),
          ),
      ],
    );
  }

  /// ---------- 스토리지 이미지 로더 ----------
  Widget _storageImage({
    required String? url,
    required String? path,
    Uint8List? webBytes,
    double height = 180,
  }) {
    if (kIsWeb && webBytes != null) {
      return Image.memory(webBytes, height: height, fit: BoxFit.cover);
    }

    if ((path ?? '').isNotEmpty) {
      return _sdkImage(path!, height, fallbackUrl: url);
    }

    if ((url ?? '').isNotEmpty) {
      return Image.network(
        url!,
        height: height,
        fit: BoxFit.cover,
        errorBuilder: (_, __, ___) => _placeholderBox('이미지 로드 실패(URL)'),
      );
    }

    return _placeholderBox('이미지가 없습니다.');
  }

  Widget _sdkImage(String fullPath, double height, {String? fallbackUrl}) {
    final storage = FirebaseStorage.instanceFor(bucket: _bucket);
    debugPrint('[IMG] getData start: bucket=$_bucket, path=$fullPath');

    final future = storage
        .ref(fullPath)
        .getData(10 * 1024 * 1024)
        .timeout(const Duration(seconds: 15));

    return FutureBuilder<Uint8List?>(
      future: future,
      builder: (context, snap) {
        if (snap.connectionState != ConnectionState.done) {
          return SizedBox(
            height: height,
            child: const Center(child: CircularProgressIndicator()),
          );
        }

        if (snap.hasError) {
          debugPrint('[IMG] getData error for $fullPath => ${snap.error}');
          if ((fallbackUrl ?? '').isNotEmpty) {
            return Image.network(
              fallbackUrl!,
              height: height,
              fit: BoxFit.cover,
              errorBuilder: (_, __, ___) => _placeholderBox('이미지 로드 실패(SDK+URL)'),
            );
          }
          return _placeholderBox('이미지 로드 실패(SDK)');
        }

        final bytes = snap.data;
        if (bytes == null || bytes.isEmpty) {
          debugPrint('[IMG] getData returned null/empty for $fullPath');
          return _placeholderBox('이미지 로드 실패(빈 데이터)');
        }

        debugPrint('[IMG] getData success: ${bytes.length} bytes for $fullPath');
        return Image.memory(bytes, height: height, fit: BoxFit.cover);
      },
    );
  }

  Widget _placeholderBox(String text) {
    return Container(
      height: 180,
      alignment: Alignment.center,
      decoration: BoxDecoration(
        color: Colors.grey.shade200,
        borderRadius: BorderRadius.circular(8),
        border: Border.all(color: Colors.grey.shade300),
      ),
      child: Text(text, style: const TextStyle(color: Colors.grey)),
    );
  }
}
