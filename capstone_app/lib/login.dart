import 'package:flutter/material.dart';
import 'package:firebase_auth/firebase_auth.dart';

class LoginPage extends StatefulWidget {
  const LoginPage({super.key});

  @override
  State<LoginPage> createState() => _LoginPageState();
}

class _LoginPageState extends State<LoginPage> {
  final _formKey = GlobalKey<FormState>();
  final _emailCtrl = TextEditingController();
  final _passwordCtrl = TextEditingController();

  Future<void> _login() async {
    try {
      await FirebaseAuth.instance.signInWithEmailAndPassword(
        email: _emailCtrl.text.trim(),
        password: _passwordCtrl.text.trim(),
      );
      ScaffoldMessenger.of(context);
      Navigator.pushReplacementNamed(context, '/home'); // 로그인 후 이동
    } on FirebaseAuthException catch (e) {
      final msg = '로그인 실패: ${e.code}';
      ScaffoldMessenger.of(context).showSnackBar(SnackBar(content: Text(msg)));
    }
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(title: const Text('로그인')),
      body: Padding(
        padding: const EdgeInsets.all(16),
        child: Form(
          key: _formKey,
          child: Column(
            children: [
              TextFormField(
                controller: _emailCtrl,
                decoration: const InputDecoration(labelText: '이메일'),
                validator: (val) =>
                    val!.contains('@') ? null : '올바른 이메일을 입력하세요',
              ),
              TextFormField(
                controller: _passwordCtrl,
                decoration: const InputDecoration(labelText: '비밀번호'),
                obscureText: true,
                validator: (val) =>
                    val!.length >= 6 ? null : '비밀번호는 6자 이상이어야 합니다',
              ),
              const SizedBox(height: 20),
              ElevatedButton(
                onPressed: () {
                  if (_formKey.currentState!.validate()) {
                    _login();
                  }
                },
                child: const Text('로그인'),
              ),
              TextButton(
                onPressed: () {
                  Navigator.pushNamed(context, '/signup'); // 회원가입 페이지로 이동
                },
                child: const Text('회원가입'),
              ),
            ],
          ),
        ),
      ),
    );
  }
}
