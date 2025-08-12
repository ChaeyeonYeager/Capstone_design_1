import 'dart:typed_data';
import 'package:flutter/foundation.dart';
import 'package:flutter/material.dart';
import 'package:camera/camera.dart';
import 'package:firebase_storage/firebase_storage.dart';
import 'dart:io' show File; // 모바일에서만 사용

class CameraCapturePage extends StatefulWidget {
  const CameraCapturePage({super.key});

  @override
  State<CameraCapturePage> createState() => _CameraCapturePageState();
}

class _CameraCapturePageState extends State<CameraCapturePage> {
  CameraController? _controller;
  XFile? _capturedImage;
  Uint8List? _imageBytes;
  bool _isCameraReady = false;
  bool _isUploading = false;

  @override
  void initState() {
    super.initState();
    _initCamera();
  }

  Future<void> _initCamera() async {
    final cameras = await availableCameras();
    _controller = CameraController(cameras.first, ResolutionPreset.medium);
    await _controller!.initialize();
    setState(() {
      _isCameraReady = true;
    });
  }

  Future<void> _takePicture() async {
    if (!_controller!.value.isInitialized) return;
    final image = await _controller!.takePicture();
    final bytes = await image.readAsBytes(); // ✅ 모든 플랫폼에서 지원

    setState(() {
      _capturedImage = image;
      _imageBytes = bytes;
    });
  }

  Future<void> _uploadToFirebase() async {
    if (_capturedImage == null) return;

    setState(() => _isUploading = true);

    try {
      final fileName = 'pets/${DateTime.now().millisecondsSinceEpoch}.jpg';
      final ref = FirebaseStorage.instance.ref().child(fileName);

      print('🔥 [업로드 시작] $fileName');

      if (kIsWeb) {
        // ✅ 웹: 바이트로 업로드
        await ref.putData(
          _imageBytes!,
          SettableMetadata(contentType: 'image/jpeg'),
        );
      } else {
        // ✅ 모바일: 파일로 업로드
        final file = File(_capturedImage!.path);
        await ref.putFile(file);
      }

      print('✅ [업로드 성공]');

      final url = await ref.getDownloadURL();
      print('✅ [URL 획득]: $url');

      if (context.mounted) {
        Navigator.pop(context, {
          'url': url,
          'bytes': _imageBytes, // 웹용 Image.memory(), 모바일에서도 fallback 가능
        });
      }
    } catch (e) {
      print('❌ 업로드 실패: $e');
      setState(() => _isUploading = false);
    }
  }

  @override
  void dispose() {
    _controller?.dispose();
    super.dispose();
  }

  @override
  Widget build(BuildContext context) {
    if (!_isCameraReady) {
      return const Scaffold(body: Center(child: CircularProgressIndicator()));
    }

    return Scaffold(
      appBar: AppBar(title: const Text("사진 촬영")),
      body: _capturedImage == null
          ? CameraPreview(_controller!)
          : Column(
              children: [
                Expanded(
                  child: kIsWeb
                      ? Image.memory(_imageBytes!) // ✅ 웹에서 바이트로 렌더링
                      : Image.file(File(_capturedImage!.path)), // ✅ 모바일은 File
                ),
                if (_isUploading)
                  const Padding(
                    padding: EdgeInsets.all(16),
                    child: CircularProgressIndicator(),
                  )
                else
                  Row(
                    mainAxisAlignment: MainAxisAlignment.spaceEvenly,
                    children: [
                      ElevatedButton.icon(
                        onPressed: () async {
                          setState(() {
                            _capturedImage = null;
                            _imageBytes = null;
                            _isCameraReady = false;
                          });

                          await _controller?.dispose(); // 기존 컨트롤러 정리
                          await _initCamera(); // 카메라 재초기화
                        },
                        icon: const Icon(Icons.refresh),
                        label: const Text("다시 찍기"),
                      ),
                      ElevatedButton.icon(
                        onPressed: _uploadToFirebase,
                        icon: const Icon(Icons.upload),
                        label: const Text("확인 & 업로드"),
                      ),
                    ],
                  ),
              ],
            ),
      floatingActionButton: _capturedImage == null
          ? FloatingActionButton(
              onPressed: _takePicture,
              child: const Icon(Icons.camera_alt),
            )
          : null,
    );
  }
}
