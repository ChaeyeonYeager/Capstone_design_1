import 'package:flutter/material.dart';
import 'package:cloud_firestore/cloud_firestore.dart';
import 'package:flutter_bluetooth_serial/flutter_bluetooth_serial.dart';
import 'dart:typed_data'; // Uint8List를 사용하기 위해 추가

class BluetoothPage extends StatefulWidget {
  const BluetoothPage({super.key});

  @override
  State<BluetoothPage> createState() => _BluetoothPageState();
}

class _BluetoothPageState extends State<BluetoothPage> {
  BluetoothConnection? _connection;
  bool isConnecting = true;
  bool get isConnected => _connection != null && _connection!.isConnected;

  final TextEditingController _textController = TextEditingController();

  // Bluetooth 장치 목록 변수 추가
  List<BluetoothDevice> _devicesList = [];

  // Firebase에서 데이터 가져오는 함수
  Future<Map<String, dynamic>> getDataFromFirestore(String petId) async {
    try {
      final petDocRef = FirebaseFirestore.instance
          .collection('pets')
          .doc(petId); // petId에 해당하는 문서 참조
      final snapshot = await petDocRef.get();
      if (snapshot.exists) {
        return snapshot.data() as Map<String, dynamic>;
      } else {
        print("No data found!");
        return {};
      }
    } catch (e) {
      print("Error fetching data from Firestore: $e");
      return {};
    }
  }

  // Bluetooth 장치와 연결하는 함수
  Future<void> _connectToDevice(String deviceAddress) async {
    try {
      await BluetoothConnection.toAddress(deviceAddress).then((conn) {
        _connection = conn;
        setState(() => isConnecting = false);

        print("✅ 연결됨");

        _connection!.input!
            .listen((data) {
              String received = String.fromCharCodes(data);
              print("📩 받은 데이터: $received");
            })
            .onDone(() {
              print("❌ 연결 끊김");
            });
      });
    } catch (error) {
      print("연결 실패: $error");
    }
  }

  // Bluetooth로 Firebase 데이터 전송
  Future<void> _sendData(String petId) async {
    // Firestore에서 데이터를 가져옴
    Map<String, dynamic> petData = await getDataFromFirestore(petId);

    // Firebase에서 가져온 각 데이터를 개별적으로 받기
    String name = petData['name'] ?? '정보 없음';
    double weight = petData['weight'] ?? 0.0;
    int age = petData['age'] ?? 0;
    String activityLevel = petData['activityLevel'] ?? '-';
    int feedCount = petData['feedCount'] ?? 0;
    List feedTimes = petData['feedTimes'] ?? [];
    double kcalPer100g = petData['kcalPer100g'] ?? 0.0;
    String viscosityLevel = petData['viscosityLevel'] ?? '-';

    // 데이터를 포맷하여 전송할 문자열 준비
    String data =
        """
    이름: $name
    몸무게: $weight kg
    나이: $age 세
    활동 수준: $activityLevel
    급식 횟수: $feedCount 회
    급식 시간: ${feedTimes.isNotEmpty ? feedTimes.join(', ') : '등록되지 않음'}
    100g당 칼로리: $kcalPer100g kcal
    유동성 단계: $viscosityLevel
  """;

    // Bluetooth 연결이 되어 있으면 데이터를 전송
    if (isConnected) {
      _connection!.output.add(
        Uint8List.fromList(data.codeUnits),
      ); // 데이터를 바이트로 변환하여 전송
      _textController.clear();
      print("📤 전송: $data");
    } else {
      print("Bluetooth 연결이 되어 있지 않습니다.");
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

  // Bluetooth 연결 버튼 클릭
  Future<void> _connectAndSendData(BluetoothDevice device) async {
    // 연결 전에 device의 주소를 통해 연결을 시도하고 데이터 보내기
    await _connectToDevice(device.address);

    // 연결 후 데이터 보내기
    String petId = _textController.text.trim(); // 입력한 petId를 가져옵니다
    if (petId.isNotEmpty) {
      await _sendData(petId); // _sendData에 petId를 전달하여 Firebase에서 데이터 가져오기
    } else {
      print("Pet ID가 비어 있습니다.");
    }
  }

  @override
  void initState() {
    super.initState();
    _getPairedDevices(); // Bluetooth 장치 목록을 가져옵니다
  }

  @override
  void dispose() {
    _connection?.dispose();
    super.dispose();
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(title: const Text("Bluetooth 연결")),
      body: Padding(
        padding: const EdgeInsets.all(16),
        child: Column(
          children: [
            Text(
              isConnecting
                  ? "🔄 연결 중..."
                  : (isConnected ? "✅ 연결됨" : "❌ 연결 안 됨"),
            ),
            const SizedBox(height: 20),

            // 데이터 전송 버튼
            ElevatedButton(
              onPressed: () {
                String petId = _textController.text.trim(); // 입력한 petId를 가져옵니다
                if (petId.isNotEmpty) {
                  _sendData(
                    petId,
                  ); // _sendData에 petId를 전달하여 Firebase에서 데이터 가져오기
                } else {
                  ScaffoldMessenger.of(context).showSnackBar(
                    const SnackBar(content: Text('Pet ID를 입력해주세요')),
                  );
                }
              },
              child: const Text("데이터 전송"),
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
                      await _connectAndSendData(device); // 장치 선택 후 데이터 전송
                    },
                  );
                }).toList(),
              ),
            ],
          ],
        ),
      ),
    );
  }
}
