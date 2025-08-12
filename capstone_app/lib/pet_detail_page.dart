import 'dart:io';
import 'dart:typed_data';
import 'package:flutter/foundation.dart';
import 'package:flutter/material.dart';
import 'package:cloud_firestore/cloud_firestore.dart';

import 'pet_edit_page.dart';

class PetDetailPage extends StatefulWidget {
  const PetDetailPage({super.key});

  @override
  State<PetDetailPage> createState() => _PetDetailPageState();
}

class _PetDetailPageState extends State<PetDetailPage> {
  File? _imageFile; // 모바일용 로컬 파일
  Uint8List? _webImageBytes; // 웹용 이미지 바이트

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
      builder: (context, snapshot) {
        if (!snapshot.hasData) {
          return const Scaffold(
            body: Center(child: CircularProgressIndicator()),
          );
        }

        final petData = snapshot.data!;
        final data = petData.data() as Map<String, dynamic>;
        final feedTimes = data['feedTimes'];
        final imageUrl = data['imageUrl'] as String?;

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
            padding: const EdgeInsets.all(16.0),
            child: ListView(
              children: [
                if (imageUrl?.isNotEmpty == true) ...[
                  kIsWeb
                      ? (_webImageBytes != null
                            ? Image.memory(_webImageBytes!)
                            : Image.network(imageUrl!))
                      : Image.network(imageUrl!),
                  const SizedBox(height: 16),
                ] else if (_imageFile != null) ...[
                  Image.file(_imageFile!),
                  const SizedBox(height: 16),
                ],
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
              ],
            ),
          ),
          floatingActionButton: FloatingActionButton(
            tooltip: '사진 촬영',
            child: const Icon(Icons.camera_alt),
            onPressed: () async {
              final result = await Navigator.pushNamed(context, '/camera');

              if (result is Map<String, dynamic>) {
                final String url = result['url'];
                final Uint8List? bytes = result['bytes'];

                // Firestore에 이미지 URL 저장
                await pet.reference.update({'imageUrl': url});

                setState(() {
                  _imageFile = null;
                  _webImageBytes = bytes;
                });

                if (context.mounted) {
                  ScaffoldMessenger.of(context).showSnackBar(
                    const SnackBar(content: Text('📸 사진 업로드 완료!')),
                  );
                }
              }
            },
          ),
        );
      },
    );
  }
}
