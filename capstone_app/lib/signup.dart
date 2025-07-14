import 'package:flutter/material.dart';
import 'package:firebase_auth/firebase_auth.dart';

class SignUpPage extends StatefulWidget {
  const SignUpPage({super.key});

  @override
  State<SignUpPage> createState() => _SignUpPageState();
}

class _SignUpPageState extends State<SignUpPage> {
  final _formKey = GlobalKey<FormState>();
  final _emailCtrl = TextEditingController();
  final _passwordCtrl = TextEditingController();
  final _confirmPasswordCtrl = TextEditingController(); // ✅ 추가된 컨트롤러

  Future<void> _register() async {
    try {
      await FirebaseAuth.instance.createUserWithEmailAndPassword(
        email: _emailCtrl.text.trim(),
        password: _passwordCtrl.text.trim(),
      );
      ScaffoldMessenger.of(
        context,
      ).showSnackBar(const SnackBar(content: Text('회원가입 성공!')));
      Navigator.pop(context);
    } on FirebaseAuthException catch (e) {
      String msg = '회원가입 실패: ${e.code}';
      if (e.code == 'email-already-in-use') {
        msg = '이미 사용 중인 이메일입니다.';
      }
      ScaffoldMessenger.of(context).showSnackBar(SnackBar(content: Text(msg)));
    }
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(title: const Text('회원가입')),
      body: Padding(
        padding: const EdgeInsets.all(16),
        child: Form(
          key: _formKey,
          child: Column(
            children: [
              TextFormField(
                controller: _emailCtrl,
                decoration: const InputDecoration(labelText: '이메일'),
                validator: (val) => val!.contains('@') ? null : '이메일 형식이 아닙니다',
              ),
              TextFormField(
                controller: _passwordCtrl,
                decoration: const InputDecoration(labelText: '비밀번호'),
                obscureText: true,
                validator: (val) => val!.length >= 6 ? null : '6자 이상 입력해야 합니다',
              ),
              TextFormField(
                controller: _confirmPasswordCtrl,
                decoration: const InputDecoration(labelText: '비밀번호 확인'),
                obscureText: true,
                validator: (val) =>
                    val == _passwordCtrl.text ? null : '비밀번호가 일치하지 않습니다',
              ),
              const SizedBox(height: 20),
              ElevatedButton(
                onPressed: () {
                  if (_formKey.currentState!.validate()) {
                    _register();
                  }
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
