import cv2
import serial
import time
import json
from ultralytics import YOLO
from embedding_utils import load_registered_images, match_dog

# ------------------------------
# 설정
# ------------------------------
SERIAL_PORT = "usbserial"   # 👉 아두이노 포트 (맥: /dev/cu.usbmodem*, 윈도우: COM3 같은 형식)
BAUD_RATE = 9600
CAMERA_INDEX = 0               # 노트북 내장 웹캠 → 0, 외장 USB 웹캠 → 1 또는 2

# ------------------------------
# 아두이노 연결
# ------------------------------
try:
    ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=1)
    time.sleep(2)  # 아두이노 리셋 대기
    print("[INFO] Arduino connected")
except Exception as e:
    print("[ERROR] Arduino 연결 실패:", e)
    ser = None

# ------------------------------
# 강아지 정보 DB 불러오기
# ------------------------------
with open("pet_db.json", "r", encoding="utf-8") as f:
    pet_db = json.load(f)

# ------------------------------
# 등록 이미지 불러오기
# ------------------------------
registered_dogs = load_registered_images("pet_images")
print(f"[INFO] 등록된 강아지 수: {len(registered_dogs)}")

# ------------------------------
# YOLO 모델 로드
# ------------------------------
model = YOLO("yolov8n.pt")  # 초경량 YOLOv8 모델

# ------------------------------
# 실시간 카메라 스트림
# ------------------------------
cap = cv2.VideoCapture(CAMERA_INDEX)

if not cap.isOpened():
    print("[ERROR] 카메라를 열 수 없습니다. CAMERA_INDEX 확인 필요.")
    exit()

while True:
    ret, frame = cap.read()
    if not ret:
        print("[ERROR] 프레임을 가져올 수 없습니다.")
        break

    # 기본 출력 텍스트 (탐지 실패 대비)
    display_text = "Match: None | Confidence: 0%"
    display_color = (0, 0, 255)  # 빨강

    # YOLO 탐지
    results = model(frame)
    detections = results[0].boxes.data.cpu().numpy()  # [x1,y1,x2,y2,conf,class]

    found_match = False

    for det in detections:
        x1, y1, x2, y2, conf, cls = det
        if int(cls) == 16:  # class 16 = dog
            roi = frame[int(y1):int(y2), int(x1):int(x2)]

            # ORB 매칭
            name, score = match_dog(roi, registered_dogs)

            if name:  # 매칭 성공
                dog_info = pet_db.get(name, {})
                size = dog_info.get("size", "unknown")
                display_text = f"Match: {name} ({int(score)}) | Size: {size}"
                display_color = (0, 255, 0)  # 초록

                # Arduino 전송
                if ser:
                    send_data = f"{name},{int(score)},{size}\n"
                    ser.write(send_data.encode("utf-8"))
                    print("[SEND]", send_data.strip())
            else:  # 매칭 실패 (YOLO 탐지 성공했는데 등록DB 없음)
                display_text = "Match: None | Confidence: 0%"
                display_color = (0, 0, 255)  # 빨강

            # 박스 + 텍스트 표시
            cv2.rectangle(frame, (int(x1), int(y1)), (int(x2), int(y2)), display_color, 2)
            cv2.putText(frame, display_text, (int(x1), int(y1)-10),
                        cv2.FONT_HERSHEY_SIMPLEX, 0.7, display_color, 2)

            found_match = True

    # YOLO 탐지 자체가 없을 때 (강아지 인식 X)
    if not found_match:
        cv2.putText(frame, display_text, (20, 40),
                    cv2.FONT_HERSHEY_SIMPLEX, 1.0, display_color, 2)

    # 결과 윈도우 출력
    cv2.imshow("Dog Feeder", frame)

    # 종료 키 → q
    if cv2.waitKey(1) & 0xFF == ord("q"):
        break

# 종료 처리
cap.release()
if ser:
    ser.close()
cv2.destroyAllWindows()
