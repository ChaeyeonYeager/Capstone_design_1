import 'package:flutter/material.dart';
import 'package:firebase_auth/firebase_auth.dart';
import 'package:cloud_firestore/cloud_firestore.dart';

import 'target_sync.dart'; // ★ 저장 직후 타깃 자동 등록/재구성

class PetEditPage extends StatefulWidget {
  final DocumentSnapshot pet;

  const PetEditPage({super.key, required this.pet});

  @override
  State<PetEditPage> createState() => _PetEditPageState();
}

class _PetEditPageState extends State<PetEditPage> {
  late TextEditingController nameController;
  late TextEditingController weightController;
  late TextEditingController ageController;
  late TextEditingController activityController;
  late TextEditingController feedCountController;
  late TextEditingController kcalController;
  late TextEditingController viscosityController;

  List<TimeOfDay> feedTimes = [];

  @override
  void initState() {
    super.initState();
    final data = widget.pet.data() as Map<String, dynamic>? ?? {};

    nameController = TextEditingController(
      text: (data['name'] ?? '').toString(),
    );
    weightController = TextEditingController(
      text: (data['weight'] ?? '').toString(),
    );
    ageController = TextEditingController(text: (data['age'] ?? '').toString());
    activityController = TextEditingController(
      text: (data['activityLevel'] ?? '').toString(),
    );
    feedCountController = TextEditingController(
      text: (data['feedCount'] ?? '').toString(),
    );
    kcalController = TextEditingController(
      text: (data['kcalPer100g'] ?? '').toString(),
    );
    viscosityController = TextEditingController(
      text: (data['viscosityLevel'] ?? '').toString(),
    );

    final rawTimes = data['feedTimes'];
    if (rawTimes is List) {
      feedTimes = rawTimes.map((t) {
        try {
          final parts = t.toString().split(':');
          return TimeOfDay(
            hour: int.parse(parts[0]),
            minute: int.parse(parts[1]),
          );
        } catch (_) {
          return TimeOfDay.now();
        }
      }).toList();
    } else {
      feedTimes = [];
    }
  }

  Future<void> _saveChanges() async {
    final formattedFeedTimes = feedTimes
        .map((t) => '${t.hour}:${t.minute.toString().padLeft(2, '0')}')
        .toList();

    final uid = FirebaseAuth.instance.currentUser?.uid;
    if (uid == null) {
      if (!mounted) return;
      ScaffoldMessenger.of(
        context,
      ).showSnackBar(const SnackBar(content: Text('로그인이 필요합니다.')));
      return;
    }

    try {
      await FirebaseFirestore.instance
          .collection('users')
          .doc(uid)
          .collection('pets')
          .doc(widget.pet.id)
          .update({
            'name': nameController.text.trim(),
            'weight': double.tryParse(weightController.text.trim()) ?? 0,
            'age': int.tryParse(ageController.text.trim()) ?? 0,
            'activityLevel':
                double.tryParse(activityController.text.trim()) ?? 1.0,
            'feedCount': int.tryParse(feedCountController.text.trim()) ?? 0,
            'kcalPer100g': int.tryParse(kcalController.text.trim()) ?? 0,
            'viscosityLevel':
                int.tryParse(viscosityController.text.trim()) ?? 1,
            'feedTimes': formattedFeedTimes,
            'updatedAt': FieldValue.serverTimestamp(),
          });

      // ★ 저장 성공 후: 서버 타깃 자동 재구성 (모든 펫 기준으로 /targets 리빌드)
      await TargetSync.rebuildTargetsFromFirestore(uid);

      if (!mounted) return;
      Navigator.pop(context);
      ScaffoldMessenger.of(
        context,
      ).showSnackBar(const SnackBar(content: Text('수정 완료 및 타깃 재등록 완료')));
    } catch (e) {
      if (!mounted) return;
      ScaffoldMessenger.of(
        context,
      ).showSnackBar(SnackBar(content: Text('저장 실패: $e')));
    }
  }

  Future<void> _pickTime(int index) async {
    final time = await showTimePicker(
      context: context,
      initialTime: feedTimes.length > index
          ? feedTimes[index]
          : TimeOfDay.now(),
    );
    if (time != null) {
      setState(() {
        if (feedTimes.length > index) {
          feedTimes[index] = time;
        } else {
          feedTimes.add(time);
        }
      });
    }
  }

  void _updateFeedTimeInputs() {
    final count = int.tryParse(feedCountController.text.trim()) ?? 0;
    setState(() {
      if (feedTimes.length < count) {
        feedTimes.addAll(
          List.generate(count - feedTimes.length, (_) => TimeOfDay.now()),
        );
      } else if (feedTimes.length > count) {
        feedTimes = feedTimes.sublist(0, count);
      }
    });
  }

  @override
  void dispose() {
    nameController.dispose();
    weightController.dispose();
    ageController.dispose();
    activityController.dispose();
    feedCountController.dispose();
    kcalController.dispose();
    viscosityController.dispose();
    super.dispose();
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(title: const Text('반려동물 정보 수정')),
      body: Padding(
        padding: const EdgeInsets.all(16.0),
        child: ListView(
          children: [
            TextField(
              controller: nameController,
              decoration: const InputDecoration(labelText: '이름'),
            ),
            TextField(
              controller: weightController,
              decoration: const InputDecoration(labelText: '몸무게 (kg)'),
              keyboardType: TextInputType.number,
            ),
            TextField(
              controller: ageController,
              decoration: const InputDecoration(labelText: '나이'),
              keyboardType: TextInputType.number,
            ),
            TextField(
              controller: activityController,
              decoration: const InputDecoration(labelText: '활동 수준'),
              keyboardType: TextInputType.number,
            ),
            TextField(
              controller: feedCountController,
              decoration: const InputDecoration(labelText: '급식 횟수'),
              keyboardType: TextInputType.number,
              onChanged: (_) => _updateFeedTimeInputs(),
            ),
            const SizedBox(height: 12),
            ...List.generate(feedTimes.length, (index) {
              final time = feedTimes[index];
              return ListTile(
                title: Text('급식 시간 ${index + 1}: ${time.format(context)}'),
                trailing: const Icon(Icons.access_time),
                onTap: () => _pickTime(index),
              );
            }),
            TextField(
              controller: kcalController,
              decoration: const InputDecoration(labelText: '100g당 칼로리'),
              keyboardType: TextInputType.number,
            ),
            TextField(
              controller: viscosityController,
              decoration: const InputDecoration(labelText: '유동성 단계'),
              keyboardType: TextInputType.number,
            ),
            const SizedBox(height: 24),
            ElevatedButton(onPressed: _saveChanges, child: const Text('수정 완료')),
          ],
        ),
      ),
    );
  }
}
