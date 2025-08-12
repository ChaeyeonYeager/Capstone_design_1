import 'dart:typed_data';
import 'package:flutter/foundation.dart';
import 'package:flutter/material.dart';
import 'package:cloud_firestore/cloud_firestore.dart';
import 'package:firebase_storage/firebase_storage.dart';

import 'pet_edit_page.dart';

class PetDetailPage extends StatefulWidget {
  const PetDetailPage({super.key});

  @override
  State<PetDetailPage> createState() => _PetDetailPageState();
}

class _PetDetailPageState extends State<PetDetailPage> {
  // 버킷 상수
  static const _bucket = 'gs://capstone-c8066.firebasestorage.app';

  // 웹에서 촬영 직후 미리보기용 바이트(새로고침 전까지 캐시)
  Uint8List? _webFaceBytes;
  Uint8List? _webBodyBytes;

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

        final feedTimes = data['feedTimes'];
        final faceUrl = data['faceImageUrl'] as String?;
        final bodyUrl = data['bodyImageUrl'] as String?;
        final facePath = data['faceImagePath'] as String?;
        final bodyPath = data['bodyImagePath'] as String?;

        return Scaffold(
          appBar: AppBar(
            title: Text('${data['name'] ?? '반려동물'}의 정보'),
            actions: [
              IconButton(
                icon: const Icon(Icons.edit),
                onPressed: () {
                  Navigator.push(
                    context,
                    MaterialPageRoute(
                      builder: (_) => PetEditPage(pet: petData),
                    ),
                  );
                },
              ),
            ],
          ),
          body: Padding(
            padding: const EdgeInsets.all(16),
            child: ListView(
              children: [
                Text('얼굴 사진', style: Theme.of(context).textTheme.titleMedium),
                const SizedBox(height: 8),
                _storageImage(
                  url: faceUrl,
                  path: facePath,
                  webBytes: _webFaceBytes,
                  height: 180,
                ),
                const SizedBox(height: 16),

                Text('몸통 사진', style: Theme.of(context).textTheme.titleMedium),
                const SizedBox(height: 8),
                _storageImage(
                  url: bodyUrl,
                  path: bodyPath,
                  webBytes: _webBodyBytes,
                  height: 180,
                ),
                const SizedBox(height: 24),

                Text('이름: ${data['name'] ?? '정보 없음'}'),
                Text('몸무게: ${data['weight'] ?? '-'} kg'),
                Text('나이: ${data['age'] ?? '-'} 세'),
                Text('활동 수준: ${data['activityLevel'] ?? '-'}'),
                Text('급식 횟수: ${data['feedCount'] ?? '-'} 회'),
                Text(
                  '급식 시간: ${feedTimes is List ? feedTimes.join(', ') : '등록되지 않음'}',
                ),
                Text('100g당 칼로리: ${data['kcalPer100g'] ?? '-'} kcal'),
                Text('유동성 단계: ${data['viscosityLevel'] ?? '-'}'),
                const SizedBox(height: 20),
                // 삭제 버튼
                ElevatedButton(
                  onPressed: () async {
                    // Firebase에서 데이터 삭제
                    await pet.reference.delete();
                    ScaffoldMessenger.of(context).showSnackBar(
                      const SnackBar(content: Text('반려동물 정보가 삭제되었습니다')),
                    );
                    Navigator.pop(context); // 이전 페이지로 돌아가기
                  },
                  style: ElevatedButton.styleFrom(
                    backgroundColor: Colors.red, // 버튼 배경 색상 설정
                  ),
                  child: const Text('삭제'),
                ),
              ],
            ),
          ),
          floatingActionButton: FloatingActionButton(
            tooltip: '사진 촬영',
            child: const Icon(Icons.camera_alt),
            onPressed: () async {
              final result = await Navigator.pushNamed(context, '/camera');
              if (!mounted) return;

              if (result is Map<String, dynamic>) {
                final url = result['url'] as String?;
                final bytes = result['bytes'] as Uint8List?;
                final part =
                    (result['part'] as String?) ?? 'face'; // 'face' | 'body'
                final path = result['path'] as String?;

                final urlField = part == 'body'
                    ? 'bodyImageUrl'
                    : 'faceImageUrl';
                final pathField = part == 'body'
                    ? 'bodyImagePath'
                    : 'faceImagePath';

                if (url != null) {
                  await pet.reference.update({
                    urlField: url,
                    if (path != null) pathField: path,
                  });
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
                  SnackBar(
                    content: Text(
                      '📸 ${part == "body" ? "몸통" : "얼굴"} 사진 업로드 완료!',
                    ),
                  ),
                );
              }
            },
          ),
        );
      },
    );
  }

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
    print('[IMG] getData start: bucket=$_bucket, path=$fullPath');

    final future = storage
        .ref(fullPath)
        .getData(10 * 1024 * 1024)
        .timeout(const Duration(seconds: 15));

    return FutureBuilder<Uint8List?>(
      // Firebase Storage에서 이미지 가져오기
      future: future,
      builder: (context, snap) {
        if (snap.connectionState != ConnectionState.done) {
          return SizedBox(
            height: height,
            child: const Center(child: CircularProgressIndicator()),
          );
        }

        if (snap.hasError) {
          print('[IMG] getData error for $fullPath => ${snap.error}');
          if ((fallbackUrl ?? '').isNotEmpty) {
            return Image.network(
              fallbackUrl!,
              height: height,
              fit: BoxFit.cover,
              errorBuilder: (_, __, ___) =>
                  _placeholderBox('이미지 로드 실패(SDK+URL)'),
            );
          }
          return _placeholderBox('이미지 로드 실패(SDK)');
        }

        final bytes = snap.data;
        if (bytes == null || bytes.isEmpty) {
          print('[IMG] getData returned null/empty for $fullPath');
          return _placeholderBox('이미지 로드 실패(빈 데이터)');
        }

        print('[IMG] getData success: ${bytes.length} bytes for $fullPath');
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
