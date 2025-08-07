import 'package:flutter/material.dart';
import 'package:cloud_firestore/cloud_firestore.dart';

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
    final data = widget.pet;
    nameController = TextEditingController(text: data['name']);
    weightController = TextEditingController(text: data['weight'].toString());
    ageController = TextEditingController(text: data['age'].toString());
    activityController = TextEditingController(
      text: data['activityLevel'].toString(),
    );
    feedCountController = TextEditingController(
      text: data['feedCount'].toString(),
    );
    kcalController = TextEditingController(
      text: data['kcalPer100g'].toString(),
    );
    viscosityController = TextEditingController(
      text: data['viscosityLevel'].toString(),
    );

    // 초기 급식 시간 세팅 (Firestore에 저장된 값)
    final rawTimes = widget.pet['feedTimes'];
    if (rawTimes != null && rawTimes is List) {
      feedTimes = rawTimes.map((t) {
        final parts = (t as String).split(':');
        return TimeOfDay(
          hour: int.parse(parts[0]),
          minute: int.parse(parts[1]),
        );
      }).toList();
    }
  }

  void _saveChanges() async {
    final formattedFeedTimes = feedTimes
        .map((t) => '${t.hour}:${t.minute.toString().padLeft(2, '0')}')
        .toList();

    await FirebaseFirestore.instance
        .collection('users')
        .doc(widget.pet.reference.parent.parent!.id)
        .collection('pets')
        .doc(widget.pet.id)
        .update({
          'name': nameController.text,
          'weight': double.tryParse(weightController.text) ?? 0,
          'age': int.tryParse(ageController.text) ?? 0,
          'activityLevel': double.tryParse(activityController.text) ?? 1.0,
          'feedCount': int.tryParse(feedCountController.text) ?? 0,
          'kcalPer100g': int.tryParse(kcalController.text) ?? 0,
          'viscosityLevel': int.tryParse(viscosityController.text) ?? 1,
          'feedTimes': formattedFeedTimes,
        });

    if (context.mounted) {
      Navigator.pop(context);
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
    final count = int.tryParse(feedCountController.text) ?? 0;
    setState(() {
      if (feedTimes.length < count) {
        // 추가
        feedTimes.addAll(
          List.generate(count - feedTimes.length, (_) => TimeOfDay.now()),
        );
      } else if (feedTimes.length > count) {
        // 제거
        feedTimes = feedTimes.sublist(0, count);
      }
    });
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
