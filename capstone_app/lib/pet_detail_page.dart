import 'package:flutter/material.dart';
import 'package:cloud_firestore/cloud_firestore.dart';

class PetDetailPage extends StatelessWidget {
  const PetDetailPage({super.key});

  @override
  Widget build(BuildContext context) {
    final DocumentSnapshot pet =
        ModalRoute.of(context)!.settings.arguments as DocumentSnapshot;

    return Scaffold(
      appBar: AppBar(title: Text('${pet['name']}의 정보')),
      body: Padding(
        padding: const EdgeInsets.all(16.0),
        child: ListView(
          children: [
            Text('이름: ${pet['name']}'),
            Text('몸무게: ${pet['weight']} kg'),
            Text('나이: ${pet['age']} 세'),
            Text('활동 수준: ${pet['activityLevel']}'),
            Text('급식 횟수: ${pet['feedCount']} 회'),
            Text('급식 시간: ${(pet['feedTimes'] as List).join(', ')}'),
            Text('100g당 칼로리: ${pet['kcalPer100g']} kcal'),
            Text('유동성 단계: ${pet['viscosityLevel']}'),
          ],
        ),
      ),
    );
  }
}
