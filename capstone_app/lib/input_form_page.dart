import 'package:flutter/material.dart';
import 'package:cloud_firestore/cloud_firestore.dart';
import 'package:firebase_auth/firebase_auth.dart';
import 'package:flutter_bluetooth_serial/flutter_bluetooth_serial.dart';
import 'dart:typed_data'; // Uint8List를 사용하기 위해 추가

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
  int _viscosityLevel = 1;
  List<TimeOfDay> _feedTimes = [];
  String? _imageUrl;

  BluetoothConnection? _connection;
  BluetoothDevice? _device;
  List<BluetoothDevice> _devicesList = [];
  bool _isBluetoothConnected = false;

  // Firebase 데이터 전송 및 Bluetooth로 전송
  Future<void> _submit() async {
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
        if (_imageUrl != null)
          'imageUrl': _imageUrl, // 사진 URL이 있을 경우 Firestore에 저장
      };

      try {
        final user = FirebaseAuth.instance.currentUser;
        if (user == null) {
          ScaffoldMessenger.of(
            context,
          ).showSnackBar(const SnackBar(content: Text('로그인이 필요합니다')));
          return;
        }

        // Firestore에 반려동물 데이터 저장
        await FirebaseFirestore.instance
            .collection('users')
            .doc(user.uid)
            .collection('pets')
            .add(petData);

        ScaffoldMessenger.of(
          context,
        ).showSnackBar(const SnackBar(content: Text('저장 완료')));

        // Bluetooth로 데이터 전송
        String data =
            "Name: ${_nameCtrl.text}, Weight: ${_weightCtrl.text}, Age: ${_ageCtrl.text}";
        await _sendDataToArduino(data); // 데이터를 아두이노로 전송

        Navigator.pop(context); // 페이지를 닫고 이전 페이지로 돌아갑니다
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

  // Bluetooth 장치 목록 가져오기
  Future<void> _getPairedDevices() async {
    List<BluetoothDevice> devices = await FlutterBluetoothSerial.instance
        .getBondedDevices();
    setState(() {
      _devicesList = devices;
    });
  }

  // Bluetooth 연결
  Future<void> _connectToDevice(BluetoothDevice device) async {
    try {
      BluetoothConnection.toAddress(device.address).then((connection) {
        print('Connected to the device');
        setState(() {
          _connection = connection;
          _isBluetoothConnected = true;
        });
      });
    } catch (e) {
      print('Connection failed: $e');
    }
  }

  // 아두이노로 데이터 전송
  Future<void> _sendDataToArduino(String data) async {
    if (_connection != null && _connection!.isConnected) {
      _connection!.output.add(
        Uint8List.fromList(data.codeUnits),
      ); // 데이터를 바이트로 변환하여 전송
      await _connection!.output.allSent;
      print("Data sent to Arduino: $data");
    } else {
      print("No connection to Arduino");
    }
  }

  // Bluetooth 연결 버튼 클릭
  Future<void> _connectAndSendData(BluetoothDevice device) async {
    await _connectToDevice(device);
    // 연결 후 데이터 보내기
    String data =
        "Name: ${_nameCtrl.text}, Weight: ${_weightCtrl.text}, Age: ${_ageCtrl.text}";
    await _sendDataToArduino(data);
  }

  // 급식 시간 선택
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
        break;
      }
    }
    setState(() {});
  }

  // 급식 시간 수정
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
  void initState() {
    super.initState();
    _getPairedDevices(); // Bluetooth 장치 목록을 가져옵니다
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(
        title: const Text('반려동물 정보 입력'),
        actions: [
          IconButton(
            icon: const Icon(Icons.camera_alt),
            tooltip: '사진 촬영',
            onPressed: () async {
              final result = await Navigator.pushNamed(context, '/camera');
              if (result is String) {
                setState(() {
                  _imageUrl = result; // 사진 URL을 _imageUrl에 저장
                });
                ScaffoldMessenger.of(context).showSnackBar(
                  const SnackBar(content: Text('📸 사진이 업로드되었습니다')),
                );
              }
            },
          ),
        ],
      ),
      body: Padding(
        padding: const EdgeInsets.all(16),
        child: Form(
          key: _formKey,
          child: ListView(
            children: [
              if (_imageUrl?.isNotEmpty ?? false) ...[
                Image.network(
                  _imageUrl!,
                  fit: BoxFit.contain,
                  height: 300,
                  width: double.infinity,
                ),
              ],
              // 반려동물 이름 입력 필드
              TextFormField(
                controller: _nameCtrl,
                decoration: const InputDecoration(labelText: '이름'),
                validator: (val) => val!.isEmpty ? '이름을 입력하세요' : null,
              ),
              // 몸무게 입력 필드
              TextFormField(
                controller: _weightCtrl,
                decoration: const InputDecoration(labelText: '몸무게 (kg)'),
                keyboardType: TextInputType.number,
                validator: (val) =>
                    double.tryParse(val!) == null ? '숫자를 입력하세요' : null,
              ),
              // 나이 입력 필드
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
              // 급식 횟수 입력 필드
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
              // 급식 시간 선택 버튼
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
              // 급식 시간 리스트
              ..._feedTimes.asMap().entries.map(
                (e) => ListTile(
                  title: Text('급식 ${e.key + 1}: ${e.value.format(context)}'),
                  trailing: const Icon(Icons.edit),
                  onTap: () => _editFeedTime(e.key),
                ),
              ),
              // 사료 칼로리 입력 필드
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
              ElevatedButton(
                onPressed: _submit,
                child: const Text('저장 및 아두이노로 데이터 보내기'),
              ),
              // Bluetooth 장치 목록 표시
              if (_devicesList.isNotEmpty) ...[
                const SizedBox(height: 20),
                const Text('Bluetooth 장치 목록:'),
                Column(
                  children: _devicesList.map((device) {
                    return ListTile(
                      title: Text(device.name ?? 'Unknown'),
                      onTap: () async {
                        await _connectAndSendData(device);
                      },
                    );
                  }).toList(),
                ),
              ],
            ],
          ),
        ),
      ),
    );
  }
}
