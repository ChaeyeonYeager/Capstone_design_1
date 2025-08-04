import 'package:flutter/material.dart';
import 'package:cloud_firestore/cloud_firestore.dart';
import 'pet_edit.dart';

class PetDetailPage extends StatelessWidget {
  const PetDetailPage({super.key});

  @override
  Widget build(BuildContext context) {
    final DocumentSnapshot pet =
        ModalRoute.of(context)!.settings.arguments as DocumentSnapshot;

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
        final feedTimes = petData['feedTimes'];

        return Scaffold(
          appBar: AppBar(
            title: Text('${petData['name'] ?? '반려동물'}의 정보'),
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
                Text('이름: ${petData['name'] ?? '정보 없음'}'),
                Text('몸무게: ${petData['weight'] ?? '-'} kg'),
                Text('나이: ${petData['age'] ?? '-'} 세'),
                Text('활동 수준: ${petData['activityLevel'] ?? '-'}'),
                Text('급식 횟수: ${petData['feedCount'] ?? '-'} 회'),
                Text(
                  '급식 시간: ${feedTimes is List ? feedTimes.join(', ') : '등록되지 않음'}',
                ),
                Text('100g당 칼로리: ${petData['kcalPer100g'] ?? '-'} kcal'),
                Text('유동성 단계: ${petData['viscosityLevel'] ?? '-'}'),
              ],
            ),
          ),
        );
      },
    );
  }
}
