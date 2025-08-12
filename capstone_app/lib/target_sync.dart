import 'package:cloud_firestore/cloud_firestore.dart';
import 'package:http/http.dart' as http;

const String kServerBase = 'http://192.168.0.207:8000'; // ★ 수정
const String kStatusUrl  = '$kServerBase/status';

class TargetSync {
  /// 모든 펫 문서를 모아 서버 타깃을 "완전히 재구성"한다.
  ///  - 첫 펫: reset=true 로 /targets
  ///  - 이후 펫: reset=false 로 /targets
  static Future<void> rebuildTargetsFromFirestore(String userId) async {
    final petsSnap = await FirebaseFirestore.instance
        .collection('users')
        .doc(userId)
        .collection('pets')
        .get();

    bool first = true;

    for (final doc in petsSnap.docs) {
      final data = doc.data();
      final name = (data['name'] ?? 'pet').toString();

      final faceUrl = (data['faceImageUrl'] ?? '').toString();
      final bodyUrl = (data['bodyImageUrl'] ?? '').toString();
      final urls = [
        if (faceUrl.isNotEmpty) faceUrl,
        if (bodyUrl.isNotEmpty) bodyUrl,
      ];
      if (urls.isEmpty) continue;

      final uri = Uri.parse('$kServerBase/targets');
      final req = http.MultipartRequest('POST', uri)
        ..fields['urls'] = urls.join(',')
        ..fields['label'] = name
        ..fields['reset'] = first.toString(); // 첫 펫만 reset

      final resp = await req.send();
      if (resp.statusCode == 200) {
        // print('✅ 등록: $name (reset=$first)');
      } else {
        final body = await resp.stream.bytesToString();
        // print('❌ 등록 실패: $name  (${resp.statusCode}) $body');
      }
      first = false;
    }
  }
}
