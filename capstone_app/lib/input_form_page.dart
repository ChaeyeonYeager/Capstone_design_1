import 'package:flutter/material.dart';
import 'package:cloud_firestore/cloud_firestore.dart';
import 'package:firebase_auth/firebase_auth.dart';

class InputFormPage extends StatefulWidget {
  const InputFormPage({super.key});

  @override
  State<InputFormPage> createState() => _InputFormPageState();
}

class _InputFormPageState extends State<InputFormPage> {
  final _formKey = GlobalKey<FormState>();

  final _nameCtrl = TextEditingController();
  final _weightCtrl = TextEditingController();
  final _ageCtrl = TextEditingController();
  final _kcalCtrl = TextEditingController();
  final _feedCountCtrl = TextEditingController();

  double _activityLevel = 1.2;
  int _viscosityLevel = 1; // 유동성 단계
  List<TimeOfDay> _feedTimes = [];

  void _submit() async {
    final count = int.tryParse(_feedCountCtrl.text);
    if (_formKey.currentState!.validate() &&
        count != null &&
        _feedTimes.length == count) {
      final petData = {
        'name': _nameCtrl.text.trim(),
        'weight': double.parse(_weightCtrl.text.trim()),
        'age': int.parse(_ageCtrl.text.trim()),
        'activityLevel': _activityLevel,
        'feedCount': count,
        'feedTimes': _feedTimes.map((t) => '${t.hour}:${t.minute}').toList(),
        'kcalPer100g': double.parse(_kcalCtrl.text.trim()),
        'viscosityLevel': _viscosityLevel,
        'createdAt': FieldValue.serverTimestamp(),
      };

      try {
        final user = FirebaseAuth.instance.currentUser;
        if (user == null) {
          ScaffoldMessenger.of(
            context,
          ).showSnackBar(const SnackBar(content: Text('로그인이 필요합니다')));
          return;
        }

        await FirebaseFirestore.instance
            .collection('users')
            .doc(user.uid)
            .collection('pets')
            .add(petData);

        ScaffoldMessenger.of(
          context,
        ).showSnackBar(const SnackBar(content: Text('저장 완료')));
        Navigator.pop(context);
      } catch (e) {
        ScaffoldMessenger.of(
          context,
        ).showSnackBar(SnackBar(content: Text('저장 실패: $e')));
      }
    } else {
      ScaffoldMessenger.of(
        context,
      ).showSnackBar(const SnackBar(content: Text('모든 필드를 올바르게 입력해주세요')));
    }
  }

  Future<void> _selectFeedTimes(int count) async {
    final currentCount = _feedTimes.length;

    for (int i = currentCount; i < count; i++) {
      final time = await showTimePicker(
        context: context,
        initialTime: const TimeOfDay(hour: 8, minute: 0),
      );
      if (time != null) {
        _feedTimes.add(time);
      } else {
        break; // 사용자가 취소하면 중단
      }
    }

    setState(() {});
  }

  Future<void> _editFeedTime(int index) async {
    final picked = await showTimePicker(
      context: context,
      initialTime: _feedTimes[index],
    );
    if (picked != null) {
      setState(() {
        _feedTimes[index] = picked;
      });
    }
  }

  @override
  void dispose() {
    _nameCtrl.dispose();
    _weightCtrl.dispose();
    _ageCtrl.dispose();
    _kcalCtrl.dispose();
    _feedCountCtrl.dispose();
    super.dispose();
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(title: const Text('반려동물 정보 입력')),
      body: Padding(
        padding: const EdgeInsets.all(16),
        child: Form(
          key: _formKey,
          child: ListView(
            children: [
              TextFormField(
                controller: _nameCtrl,
                decoration: const InputDecoration(labelText: '이름'),
                validator: (val) => val!.isEmpty ? '이름을 입력하세요' : null,
              ),
              TextFormField(
                controller: _weightCtrl,
                decoration: const InputDecoration(labelText: '몸무게 (kg)'),
                keyboardType: TextInputType.number,
                validator: (val) =>
                    double.tryParse(val!) == null ? '숫자를 입력하세요' : null,
              ),
              TextFormField(
                controller: _ageCtrl,
                decoration: const InputDecoration(labelText: '나이 (세)'),
                keyboardType: TextInputType.number,
                validator: (val) =>
                    int.tryParse(val!) == null ? '정수를 입력하세요' : null,
              ),
              const SizedBox(height: 10),
              const Text('활동 수준'),
              DropdownButton<double>(
                value: _activityLevel,
                onChanged: (val) => setState(() => _activityLevel = val!),
                items: const [
                  DropdownMenuItem(value: 1.0, child: Text('활동량 적음')),
                  DropdownMenuItem(value: 1.2, child: Text('보통')),
                  DropdownMenuItem(value: 1.4, child: Text('활동량 많음')),
                ],
              ),
              TextFormField(
                controller: _feedCountCtrl,
                decoration: const InputDecoration(labelText: '급식 횟수'),
                keyboardType: TextInputType.number,
                validator: (val) {
                  final num = int.tryParse(val!);
                  return (num == null || num < 1 || num > 6)
                      ? '1~6 사이로 입력'
                      : null;
                },
              ),
              ElevatedButton(
                onPressed: () {
                  final count = int.tryParse(_feedCountCtrl.text);
                  if (count != null && count > 0 && count <= 6) {
                    _selectFeedTimes(count);
                  } else {
                    ScaffoldMessenger.of(context).showSnackBar(
                      const SnackBar(content: Text('급식 횟수를 먼저 정확히 입력하세요')),
                    );
                  }
                },
                child: Text(
                  '급식 시간 선택 (${_feedTimes.length} / ${_feedCountCtrl.text})',
                ),
              ),
              if (_feedTimes.isNotEmpty)
                Column(
                  crossAxisAlignment: CrossAxisAlignment.start,
                  children: _feedTimes
                      .asMap()
                      .entries
                      .map(
                        (e) => ListTile(
                          title: Text(
                            '급식 ${e.key + 1}: ${e.value.format(context)}',
                          ),
                          trailing: const Icon(Icons.edit),
                          onTap: () => _editFeedTime(e.key),
                        ),
                      )
                      .toList(),
                ),
              TextFormField(
                controller: _kcalCtrl,
                decoration: const InputDecoration(labelText: '100g당 사료 칼로리'),
                keyboardType: TextInputType.number,
                validator: (val) =>
                    double.tryParse(val!) == null ? '숫자를 입력하세요' : null,
              ),
              const SizedBox(height: 10),
              const Text('유동성 단계'),
              DropdownButton<int>(
                value: _viscosityLevel,
                onChanged: (val) => setState(() => _viscosityLevel = val!),
                items: const [
                  DropdownMenuItem(value: 0, child: Text('덜 부드러움 (고형 사료 위주)')),
                  DropdownMenuItem(value: 1, child: Text('보통 (일반 분쇄)')),
                  DropdownMenuItem(value: 2, child: Text('매우 부드러움 (유동식)')),
                ],
              ),
              const SizedBox(height: 20),
              ElevatedButton(onPressed: _submit, child: const Text('저장하기')),
            ],
          ),
        ),
      ),
    );
  }
}
