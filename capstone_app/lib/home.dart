import 'package:flutter/material.dart';
import 'package:firebase_auth/firebase_auth.dart';
import 'package:cloud_firestore/cloud_firestore.dart';

class HomePage extends StatelessWidget {
  const HomePage({super.key});

  void _logout(BuildContext context) async {
    await FirebaseAuth.instance.signOut();
    Navigator.pushReplacementNamed(context, '/');
  }

  @override
  Widget build(BuildContext context) {
    final user = FirebaseAuth.instance.currentUser;
    final emailPrefix = user?.email?.split('@').first ?? '사용자';

    return Scaffold(
      appBar: AppBar(
        title: Text('$emailPrefix님'),
        actions: [
          IconButton(
            icon: const Icon(Icons.logout),
            onPressed: () => _logout(context),
          ),
        ],
      ),
      body: StreamBuilder<QuerySnapshot>(
        stream: FirebaseFirestore.instance
            .collection('users')
            .doc(user?.uid)
            .collection('pets')
            .orderBy('createdAt', descending: true)
            .snapshots(),
        builder: (context, snapshot) {
          if (!snapshot.hasData) {
            return const Center(child: CircularProgressIndicator());
          }

          final pets = snapshot.data!.docs;
          if (pets.isEmpty) {
            return const Center(child: Text('등록된 반려동물이 없습니다.'));
          }

          return ListView.builder(
            itemCount: pets.length,
            itemBuilder: (context, index) {
              final pet = pets[index];
              final name = pet['name'];

              return ListTile(
                title: Text(name),
                onTap: () {
                  Navigator.pushNamed(
                    context,
                    '/detail',
                    arguments: pet, // pet 문서 전체를 넘김
                  );
                },
              );
            },
          );
        },
      ),
      floatingActionButton: Column(
        mainAxisSize: MainAxisSize.min,
        children: [
          FloatingActionButton(
            heroTag: 'camera',
            onPressed: () {
              // 카메라 관련 기능 또는 라우트
              Navigator.pushNamed(context, '/camera');
            },
            tooltip: '카메라',
            child: const Icon(Icons.camera_alt),
          ),
          const SizedBox(height: 16),
          FloatingActionButton(
            heroTag: 'add',
            onPressed: () {
              Navigator.pushNamed(context, '/add');
            },
            tooltip: '추가하기',
            child: const Icon(Icons.add),
          ),
        ],
      ),
      floatingActionButtonLocation: FloatingActionButtonLocation.endFloat,
    );
  }
}
