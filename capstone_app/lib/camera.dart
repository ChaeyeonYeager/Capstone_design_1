import 'dart:typed_data';
import 'package:flutter/material.dart';
import 'package:camera/camera.dart';
import 'package:firebase_storage/firebase_storage.dart';
import 'package:firebase_auth/firebase_auth.dart';
import 'dart:io' show File;

class CameraCapturePage extends StatefulWidget {
  const CameraCapturePage({super.key});

  @override
  State<CameraCapturePage> createState() => _CameraCapturePageState();
}

class _CameraCapturePageState extends State<CameraCapturePage> {
  CameraController? _controller;
  XFile? _capturedImage;
  bool _isCameraReady = false;
  bool _isUploading = false;
  Uint8List? _imageBytes;
  double _uploadProgress = 0.0;

  @override
  void initState() {
    super.initState();
    _initializeCamera();
  }

  Future<void> _initializeCamera() async {
    try {
      final cameras = await availableCameras();
      _controller = CameraController(cameras.first, ResolutionPreset.medium);
      await _controller!.initialize();
      setState(() => _isCameraReady = true);
    } catch (e) {
      print("카메라 초기화 실패: $e");
    }
  }

  Future<void> _takePicture() async {
    if (!(_controller?.value.isInitialized ?? false)) return;
    try {
      final image = await _controller!.takePicture();
      final bytes = await image.readAsBytes();
      setState(() {
        _capturedImage = image;
        _imageBytes = bytes;
      });
    } catch (e) {
      print("사진 촬영 실패: $e");
    }
  }

  // part: 'face' | 'body'
  Future<void> _uploadToFirebase(String part) async {
    if (_imageBytes == null) return;
    setState(() => _isUploading = true);
    try {
      final user = FirebaseAuth.instance.currentUser;
      if (user == null) {
        ScaffoldMessenger.of(context).showSnackBar(
          const SnackBar(content: Text('로그인이 필요합니다.')),
        );
        return;
      }

      // ✅ 필요시 아래 줄을 instanceFor(bucket: 'gs://xxx.appspot.com')로 교체하세요.
      final storage = FirebaseStorage.instance;
      final fileName =
          'images/$part/${DateTime.now().millisecondsSinceEpoch}.jpg';
      final ref = storage.ref().child(fileName);

      final task = ref.putData(
        _imageBytes!,
        SettableMetadata(
          contentType: 'image/jpeg',
          cacheControl: 'public,max-age=3600',
        ),
      );

      task.snapshotEvents.listen((s) {
        setState(() {
          _uploadProgress =
              s.totalBytes > 0 ? (s.bytesTransferred / s.totalBytes * 100) : 0;
        });
      });

      await task;
      final url = await ref.getDownloadURL();

      if (context.mounted) {
        Navigator.pop(context, {
          'url': url,
          'bytes': _imageBytes, // 웹 미리보기용
          'part': part,         // ✅ 어떤 파트인지 돌려줌
        });
      }
    } catch (e) {
      print('업로드 실패: $e');
      ScaffoldMessenger.of(context).showSnackBar(
        const SnackBar(content: Text('업로드 실패')),
      );
    } finally {
      if (mounted) setState(() => _isUploading = false);
    }
  }

  Future<void> _resetCamera() async {
    await _controller?.dispose();
    await _initializeCamera();
    setState(() {
      _capturedImage = null;
      _imageBytes = null;
      _uploadProgress = 0;
    });
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
      appBar: AppBar(title: const Text("카메라 사진 촬영")),
      body: _capturedImage == null
          ? CameraPreview(_controller!)
          : Column(
              children: [
                Expanded(
                  child: _imageBytes != null
                      ? Image.memory(_imageBytes!)
                      : const Center(child: CircularProgressIndicator()),
                ),
                if (_isUploading)
                  Column(
                    children: [
                      const Padding(
                        padding: EdgeInsets.all(16),
                        child: CircularProgressIndicator(),
                      ),
                      Text("업로드 진행률: ${_uploadProgress.toStringAsFixed(2)}%"),
                      const SizedBox(height: 8),
                    ],
                  )
                else
                  Padding(
                    padding: const EdgeInsets.symmetric(vertical: 12),
                    child: Row(
                      mainAxisAlignment: MainAxisAlignment.spaceEvenly,
                      children: [
                        OutlinedButton.icon(
                          onPressed: _resetCamera,
                          icon: const Icon(Icons.refresh),
                          label: const Text("다시 찍기"),
                        ),
                        ElevatedButton.icon(
                          onPressed: () => _uploadToFirebase('face'),
                          icon: const Icon(Icons.tag_faces),
                          label: const Text("얼굴로 업로드"),
                        ),
                        ElevatedButton.icon(
                          onPressed: () => _uploadToFirebase('body'),
                          icon: const Icon(Icons.pets),
                          label: const Text("몸통으로 업로드"),
                        ),
                      ],
                    ),
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
