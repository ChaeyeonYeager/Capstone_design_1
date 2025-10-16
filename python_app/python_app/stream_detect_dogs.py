# 실제 스트리밍

# # stream_detect_dogs.py
# # iPhone(ManyCam) 스트림을 하드코딩 소스로 받아 실시간 분석 + 화면 오버레이

# import cv2
# import json
# import time
# import os
# from ultralytics import YOLO

# # test_* 유틸을 쓰는 환경과 일반 유틸을 쓰는 환경 모두 대응
# try:
#     from test_embedding_utils import load_registered_images, match_dog
# except ImportError:
#     from embedding_utils import load_registered_images, match_dog

# DOG_CLASS_ID = 16  # COCO: dog

# # ------------------------------
# # 하드코딩: iPhone(ManyCam) 스트림 URL
# # 예) "http://192.168.0.21:8080/video"  또는  "rtsp://<ip>:<port>/..."
# # ------------------------------
# IP_STREAM_URL = "http://10.96.41.72:4747/video"  # <-- 본인 환경에 맞게 변경

# # 해상도(선택): None이면 기본
# FORCE_WIDTH  = None
# FORCE_HEIGHT = None

# # YOLO 설정
# YOLO_WEIGHTS = "yolov8n.pt"
# YOLO_CONF    = 0.25
# YOLO_DEVICE  = None  # "cuda:0" / "mps" / "cpu" 등 필요시 지정


# def open_stream(url, width=None, height=None):
#     """URL 스트림 열기; 해상도 강제 설정 옵션."""
#     cap = cv2.VideoCapture(url)
#     if not cap.isOpened():
#         return None
#     if width:
#         cap.set(cv2.CAP_PROP_FRAME_WIDTH, int(width))
#     if height:
#         cap.set(cv2.CAP_PROP_FRAME_HEIGHT, int(height))
#     return cap


# def safe_rect(x1, y1, x2, y2, w, h):
#     x1 = max(0, min(int(x1), w - 1))
#     y1 = max(0, min(int(y1), h - 1))
#     x2 = max(0, min(int(x2), w - 1))
#     y2 = max(0, min(int(y2), h - 1))
#     if x2 <= x1:
#         x2 = min(w - 1, x1 + 1)
#     if y2 <= y1:
#         y2 = min(h - 1, y1 + 1)
#     return x1, y1, x2, y2


# def robust_match(roi, registered):
#     """
#     match_dog 반환 형태가 (name, score, id_conf) 또는 (name, score) 두 가지일 수 있어
#     안전하게 처리. id_conf가 없으면 간단히 점수→%로 히ュー리스틱 정규화.
#     """
#     name, score, id_conf = None, 0, 0
#     try:
#         out = match_dog(roi, registered)
#         if len(out) == 3:
#             name, score, id_conf = out
#         elif len(out) == 2:
#             name, score = out
#             # 히ュー리스틱: 특징점 매칭 수를 0~100%로 클램프
#             id_conf = max(0, min(100, int(score)))
#         else:
#             # 예외적 포맷일 경우 방어
#             name, score = out[0], out[1] if len(out) > 1 else 0
#             id_conf = max(0, min(100, int(score)))
#     except Exception:
#         pass
#     return name, int(score), int(id_conf)


# def main():
#     # pet_db 로드 (있으면 size 표기)
#     pet_db = {}
#     if os.path.isfile("pet_db.json"):
#         try:
#             with open("pet_db.json", "r", encoding="utf-8") as f:
#                 pet_db = json.load(f)
#                 print(f"[INFO] pet_db 로드: {len(pet_db)}마리")
#         except Exception as e:
#             print("[WARN] pet_db.json 로드 실패:", e)

#     # 등록 이미지 로드
#     try:
#         registered = load_registered_images("pet_images", verbose=True)
#     except TypeError:
#         registered = load_registered_images("pet_images")
#     print(f"[INFO] 등록된 강아지 수: {len(registered)}")

#     # YOLO 로드
#     model = YOLO(YOLO_WEIGHTS)
#     if YOLO_DEVICE is not None:
#         try:
#             model.to(YOLO_DEVICE)
#             print(f"[INFO] YOLO device: {YOLO_DEVICE}")
#         except Exception as e:
#             print("[WARN] device 설정 실패(자동 선택):", e)

#     # 스트림 오픈
#     cap = open_stream(IP_STREAM_URL, width=FORCE_WIDTH, height=FORCE_HEIGHT)
#     if not cap or not cap.isOpened():
#         print(f"[ERROR] 스트림을 열 수 없습니다: {IP_STREAM_URL}")
#         print(" - ManyCam 앱에서 'Stream over Wi-Fi' 주소 확인 후 IP_STREAM_URL 수정하세요.")
#         return

#     print("[INFO] 스트리밍 시작. 창 활성화 후 'q' 키로 종료.")
#     t0 = time.time()
#     frame_idx = 0

#     while True:
#         ok, frame = cap.read()
#         if not ok:
#             print("[WARN] 프레임을 읽지 못했습니다.")
#             break
#         frame_idx += 1
#         h, w = frame.shape[:2]

#         # 기본 안내(탐지 없음)
#         drawn = False
#         base_text  = "No dog detected"
#         base_color = (0, 0, 255)

#         # YOLO 추론
#         results = model(frame, conf=YOLO_CONF, verbose=False)
#         boxes = results[0].boxes

#         if boxes is not None and len(boxes) > 0:
#             xyxy = boxes.xyxy.cpu().numpy()
#             cls  = boxes.cls.cpu().numpy()
#             conf = boxes.conf.cpu().numpy()

#             for i in range(len(xyxy)):
#                 if int(cls[i]) != DOG_CLASS_ID:
#                     continue

#                 x1, y1, x2, y2 = xyxy[i]
#                 yolo_conf = float(conf[i])
#                 x1, y1, x2, y2 = safe_rect(x1, y1, x2, y2, w, h)
#                 roi = frame[y1:y2, x1:x2]

#                 # ORB 매칭
#                 name, match_score, id_conf = robust_match(roi, registered)
#                 size = pet_db.get(name, {}).get("size", "unknown") if name else "unknown"

#                 if name:
#                     color = (0, 255, 0)
#                     label = (
#                         f"Dog: {name} | Match: {match_score} | "
#                         f"ID_Conf: {id_conf}% | YOLO_Conf: {int(yolo_conf*100)}% | Size: {size}"
#                     )
#                 else:
#                     color = (0, 0, 255)
#                     label = (
#                         f"Dog: None | Match: 0 | "
#                         f"ID_Conf: 0% | YOLO_Conf: {int(yolo_conf*100)}%"
#                     )

#                 cv2.rectangle(frame, (x1, y1), (x2, y2), color, 2)
#                 cv2.putText(frame, label, (x1, max(20, y1 - 10)),
#                             cv2.FONT_HERSHEY_SIMPLEX, 0.6, color, 2)
#                 drawn = True

#         # 박스가 하나도 없었던 프레임에도 안내 출력
#         if not drawn:
#             cv2.putText(frame, base_text, (20, 40),
#                         cv2.FONT_HERSHEY_SIMPLEX, 1.0, base_color, 2)

#         # FPS
#         elapsed = max(1e-6, time.time() - t0)
#         fps_est = frame_idx / elapsed
#         cv2.putText(frame, f"FPS: {fps_est:.1f}", (20, h - 20),
#                     cv2.FONT_HERSHEY_SIMPLEX, 0.6, (255, 255, 255), 2)

#         # 화면 출력
#         cv2.imshow("Dog Feeder (ManyCam Stream)", frame)
#         if cv2.waitKey(1) & 0xFF == ord("q"):
#             break

#     cap.release()
#     cv2.destroyAllWindows()
#     print("[INFO] 종료")


# if __name__ == "__main__":
#     main()

import cv2
import json
import time
import os
import serial
from ultralytics import YOLO

# test_* 유틸을 쓰는 환경과 일반 유틸을 쓰는 환경 모두 대응
try:
    from test_embedding_utils import load_registered_images, match_dog
except ImportError:
    from embedding_utils import load_registered_images, match_dog

DOG_CLASS_ID = 16  # COCO: dog

# ------------------------------
# DroidCam Wi-Fi 스트림 URL 하드코딩 (앱에서 보이는 IP/Port 그대로 입력)
# ------------------------------
IP_STREAM_URL = "http://172.30.1.72:4747/video"   # ← 본인 환경에 맞게 수정

# 해상도 강제(선택) - None이면 기본
FORCE_WIDTH  = None
FORCE_HEIGHT = None

# YOLO 설정
YOLO_WEIGHTS = "yolov8n.pt"
YOLO_CONF    = 0.25
YOLO_DEVICE  = None  # "cuda:0" / "mps" / "cpu"

# 오탐 방지 임계치 (원하면 낮추거나 0으로)
YOLO_MIN_CONF = 0.30   # YOLO 확신도 최소
ID_MIN_CONF   = 30     # ORB ID 신뢰도(%) 최소 (간단 휴리스틱)

# 세션/디바운스: 강아지 미검출 프레임이 N회 연속이면 다음 검출에 다시 1회 전송
CLEAR_FRAMES = 10

# ------------------------------
# Arduino 시리얼 설정 (환경에 맞게 수정)
# mac: "/dev/cu.usbmodem*", windows: "COM3" 등
# ------------------------------
SERIAL_PORT = "/dev/cu.usbmodem11301"
BAUD_RATE   = 115200



# 코드 상단에 추가
NAME_MAP = {
    "까미": "Kami",
    "똘이": "Ddori",
    "율무": "Yulmu",
    "하트": "Heart",
    "설기": "Seolgi",
    "콩이": "Kong",
    "삐삐": "Pippi",
    "쿠키": "Cookie"
}




def open_stream(url, width=None, height=None):
    cap = cv2.VideoCapture(url)
    if not cap.isOpened():
        return None
    if width:
        cap.set(cv2.CAP_PROP_FRAME_WIDTH, int(width))
    if height:
        cap.set(cv2.CAP_PROP_FRAME_HEIGHT, int(height))
    return cap


def safe_rect(x1, y1, x2, y2, w, h):
    x1 = max(0, min(int(x1), w - 1))
    y1 = max(0, min(int(y1), h - 1))
    x2 = max(0, min(int(x2), w - 1))
    y2 = max(0, min(int(y2), h - 1))
    if x2 <= x1:
        x2 = min(w - 1, x1 + 1)
    if y2 <= y1:
        y2 = min(h - 1, y1 + 1)
    return x1, y1, x2, y2


def robust_match(roi, registered):
    """
    match_dog 반환이 (name, score, id_conf) 또는 (name, score)일 수 있어 안전 처리.
    id_conf가 없으면 score를 0~100%로 클램프.
    """
    name, score, id_conf = None, 0, 0
    try:
        out = match_dog(roi, registered)
        if len(out) == 3:
            name, score, id_conf = out
        elif len(out) == 2:
            name, score = out
            id_conf = max(0, min(100, int(score)))
        else:
            name = out[0] if len(out) > 0 else None
            score = out[1] if len(out) > 1 else 0
            id_conf = max(0, min(100, int(score)))
    except Exception:
        pass
    return name, int(score), int(id_conf)


def read_arduino_lines_nonblocking(ser):
    """아두이노에서 오는 로그를 비차단으로 읽어 터미널에 뿌림."""
    if not ser:
        return
    try:
        n = ser.in_waiting
        if n > 0:
            data = ser.read(n)
            # 줄 단위로 출력
            for line in data.decode(errors="ignore").splitlines():
                line = line.strip()
                if line:
                    print(f"[ARDUINO] {line}")
    except Exception:
        pass


def main():
    # ----- Arduino 연결 -----
    ser = None
    try:
        ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=0)  # timeout=0 → non-blocking
        time.sleep(2)
        print("[INFO] Arduino connected:", SERIAL_PORT)
    except Exception as e:
        print("[WARN] Arduino 연결 실패:", e)
        ser = None

    # ----- pet_db 로드 (있으면 size/weight 등 사용) -----
    pet_db = {}
    if os.path.isfile("pet_db.json"):
        try:
            with open("pet_db.json", "r", encoding="utf-8") as f:
                pet_db = json.load(f)
                print(f"[INFO] pet_db 로드: {len(pet_db)}마리")
        except Exception as e:
            print("[WARN] pet_db.json 로드 실패:", e)

    # ----- 등록 이미지 로드 -----
    try:
        registered = load_registered_images("pet_images", verbose=True)
    except TypeError:
        registered = load_registered_images("pet_images")
    print(f"[INFO] 등록된 강아지 수: {len(registered)}")

    # ----- YOLO 로드 -----
    model = YOLO(YOLO_WEIGHTS)
    if YOLO_DEVICE is not None:
        try:
            model.to(YOLO_DEVICE)
            print(f"[INFO] YOLO device:", YOLO_DEVICE)
        except Exception as e:
            print("[WARN] device 설정 실패(자동):", e)

    # ----- 스트림 오픈 -----
    cap = open_stream(IP_STREAM_URL, width=FORCE_WIDTH, height=FORCE_HEIGHT)
    if not cap or not cap.isOpened():
        print(f"[ERROR] 스트림을 열 수 없습니다: {IP_STREAM_URL}")
        print(" - DroidCam 앱에서 보이는 IP/Port를 확인해 IP_STREAM_URL을 수정하세요.")
        return

    print("[INFO] 스트리밍 시작. 'q' 로 종료.")
    program_start = time.time()
    frame_idx = 0

    # ====== 세션 관리 변수 ======
    session_active = False        # 현재 '보이는 중' 세션
    clear_counter  = 0            # 연속 '없음' 프레임 수
    last_sent_name = None         # 마지막 전송 대상 이름(선택)

    # 최초 검출 시각 기록용 (터미널 타임로그)
    t_detect_first = None  # float(Unix epoch seconds)

    while True:
        ok, frame = cap.read()
        if not ok:
            print("[WARN] 프레임을 읽지 못했습니다.")
            break
        frame_idx += 1
        h, w = frame.shape[:2]

        # 아두이노 로그 비차단 수신
        read_arduino_lines_nonblocking(ser)

        drawn = False
        base_text  = "No dog detected"
        base_color = (0, 0, 255)

        # YOLO 추론
        results = model(frame, conf=YOLO_CONF, verbose=False)
        boxes = results[0].boxes

        best = None  # 트리거 판단에 사용할 최고 신뢰 후보
        if boxes is not None and len(boxes) > 0:
            xyxy = boxes.xyxy.cpu().numpy()
            cls  = boxes.cls.cpu().numpy()
            conf = boxes.conf.cpu().numpy()

            for i in range(len(xyxy)):
                if int(cls[i]) != DOG_CLASS_ID:
                    continue

                x1, y1, x2, y2 = xyxy[i]
                yolo_conf = float(conf[i])
                x1, y1, x2, y2 = safe_rect(x1, y1, x2, y2, w, h)
                roi = frame[y1:y2, x1:x2]

                name, match_score, id_conf = robust_match(roi, registered)
                size = pet_db.get(name, {}).get("size", "unknown") if name else "unknown"

                if name:
                    color = (0, 255, 0)
                    label_name = NAME_MAP.get(name, name)  # ← 매핑된 영어 이름 표시
                    label = (
                        f"Dog: {label_name} | Match: {match_score} | "
                        f"ID_Conf: {id_conf}% | YOLO_Conf: {int(yolo_conf*100)}% | Size: {size}"
                    )
                else:
                    color = (0, 0, 255)
                    label = (
                        f"Dog: None | Match: 0 | "
                        f"ID_Conf: 0% | YOLO_Conf: {int(yolo_conf*100)}%"
                    )

                cv2.rectangle(frame, (x1, y1), (x2, y2), color, 2)
                cv2.putText(frame, label, (x1, max(20, y1 - 10)),
                            cv2.FONT_HERSHEY_SIMPLEX, 0.6, color, 2)
                drawn = True

                if best is None or yolo_conf > best["yolo_conf"]:
                    best = {
                        "name": name,
                        "match_score": match_score,
                        "id_conf": id_conf,
                        "yolo_conf": yolo_conf,
                        "size": size
                    }

        # ====== 트리거 로직 ======
        if best is not None:
            clear_counter = 0  # 강아지 보임

            # 최초 인식 타임로그: 세션이 아직 안 열려 있고, 임계치를 만족하면 기록 및 전송
            if (not session_active) and \
               (best["name"] is not None) and \
               (best["yolo_conf"] >= YOLO_MIN_CONF) and \
               (best["id_conf"]   >= ID_MIN_CONF):

                session_active = True
                last_sent_name = best["name"]

                # 1) 최초 인식 시각 로그
                now = time.time()
                t_detect_first = now
                human = time.strftime("%Y-%m-%d %H:%M:%S", time.localtime(now))
                since_start_ms = int((now - program_start) * 1000)
                print(f"[DETECT] first_seen t_detect = {since_start_ms} ms ({human}) | "
                      f"name={best['name']}, id_conf={best['id_conf']}%, yolo={int(best['yolo_conf']*100)}%")

                # 2) CSV 전송(아두이노)
                info = pet_db.get(best["name"], {})
                size       = info.get("size", best["size"] or "unknown")
                weight     = info.get("weight", 0.0)
                activeLvl  = info.get("activeLvl", 1.6)
                calPerKg   = info.get("calPerKg", 3500)
                feedingCnt = info.get("feedingCount", 2)

                csv = f"{best['name']},{best['id_conf']},{size},{weight},{activeLvl},{calPerKg},{feedingCnt}\n"
                if ser:
                    ser.write(csv.encode("utf-8"))
                print("[TRIGGER] SEND →", csv.strip())

        else:
            # 이번 프레임엔 강아지 없음
            clear_counter += 1
            if clear_counter >= CLEAR_FRAMES:
                # 세션 리셋 → 다음에 보이면 다시 1회 전송
                if session_active:
                    print("[SESSION] cleared")
                session_active = False
                last_sent_name = None
                clear_counter = 0
                t_detect_first = None

        # 탐지 없으면 안내 문구
        if not drawn:
            cv2.putText(frame, base_text, (20, 40),
                        cv2.FONT_HERSHEY_SIMPLEX, 1.0, base_color, 2)

        # FPS
        elapsed = max(1e-6, (time.time() - program_start))
        fps_est = frame_idx / elapsed
        cv2.putText(frame, f"FPS: {fps_est:.1f}", (20, h - 20),
                    cv2.FONT_HERSHEY_SIMPLEX, 0.6, (255, 255, 255), 2)

        cv2.imshow("Dog Feeder (DroidCam Stream)", frame)
        if cv2.waitKey(1) & 0xFF == ord("q"):
            break

    cap.release()
    if ser:
        ser.close()
    cv2.destroyAllWindows()
    print("[INFO] 종료")


if __name__ == "__main__":
    main()
