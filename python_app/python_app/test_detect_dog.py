# 미리 저장된 영상을 분석하여 result_video에 저장

# # test_videos/의 모든 동영상(.mp4/.mov/.avi/.mkv)을 읽어
# # 결과를 result_videos/원본이름_result.mp4로 저장합니다.

# # 각 박스에는 Dog, MatchScore, ID_Conf(%) , YOLO_Conf(%) , Size를 표시합니다.

# # pet_db.json이 있으면 size 정보도 함께 표기합니다(없어도 동작).




# # test_detect_dogs.py
# # test_videos/의 모든 동영상(.mp4/.mov/.avi/.mkv)을 분석해
# # result_videos/원본이름_result.mp4로 저장합니다.
# # 각 박스에는 Dog, MatchScore, ID_Conf(%), YOLO_Conf(%), Size를 표시합니다.
# # pet_db.json이 있으면 size 정보도 함께 표기합니다(없어도 동작).

# import os
# import json
# import time
# import argparse
# import cv2
# from ultralytics import YOLO
# from test_embedding_utils import load_registered_images, match_dog

# DOG_CLASS_ID = 16  # COCO dataset: dog
# VIDEO_EXTS = (".mp4", ".mov", ".avi", ".mkv")

# # 🔠 한글 → 영어 이름 매핑
# NAME_MAP = {
#     "까미": "Kami",
#     "똘이": "Ddori",
#     "율무": "Yulmu",
#     "하트": "Heart",
#     "설기": "Seolgi",
#     "콩이": "Kong",
#     "삐삐": "Pippi",
#     "쿠키": "Cookie"
# }

# def list_videos(input_dir):
#     """하위 폴더까지 모두 탐색"""
#     files = []
#     if not os.path.isdir(input_dir):
#         print(f"[ERROR] 입력 폴더가 없습니다: {input_dir}")
#         return files
#     for root, _, fnames in os.walk(input_dir):
#         for fname in sorted(fnames):
#             if fname.lower().endswith(VIDEO_EXTS):
#                 files.append(os.path.join(root, fname))
#     return files


# def safe_rect(x1, y1, x2, y2, w, h):
#     x1 = max(0, min(int(x1), w-1))
#     y1 = max(0, min(int(y1), h-1))
#     x2 = max(0, min(int(x2), w-1))
#     y2 = max(0, min(int(y2), h-1))
#     if x2 <= x1: x2 = min(w-1, x1+1)
#     if y2 <= y1: y2 = min(h-1, y1+1)
#     return x1, y1, x2, y2


# def main(
#     input_dir="test_videos",
#     output_dir="result_videos",
#     pet_dir="pet_images",
#     pet_db_path="pet_db.json",
#     yolo_weights="yolov8n.pt",
#     yolo_conf=0.25,
#     device=None
# ):
#     os.makedirs(output_dir, exist_ok=True)

#     # ----- pet_db 로드 -----
#     pet_db = {}
#     if os.path.isfile(pet_db_path):
#         try:
#             with open(pet_db_path, "r", encoding="utf-8") as f:
#                 pet_db = json.load(f)
#                 print(f"[INFO] pet_db 로드: {len(pet_db)}마리")
#         except Exception as e:
#             print("[WARN] pet_db.json 로드 실패:", e)

#     # ----- 등록 이미지 로드 -----
#     registered = load_registered_images(pet_dir, verbose=True)
#     print(f"[INFO] 등록된 강아지 수: {len(registered)}")

#     # ----- YOLO 모델 로드 -----
#     model = YOLO(yolo_weights)
#     if device is not None:
#         try:
#             model.to(device)
#             print(f"[INFO] YOLO device: {device}")
#         except Exception as e:
#             print("[WARN] device 설정 실패(무시):", e)

#     # ----- 입력 영상 리스트 -----
#     videos = list_videos(input_dir)
#     if not videos:
#         print(f"[ERROR] 처리할 영상이 없습니다: {input_dir}")
#         return

#     for vpath in videos:
#         vname = os.path.basename(vpath)
#         out_path = os.path.join(
#             output_dir,
#             os.path.splitext(vname)[0] + "_result.mp4"
#         )
#         print(f"\n[PROCESS] {vname} -> {out_path}")

#         cap = cv2.VideoCapture(vpath)
#         if not cap.isOpened():
#             print(f"[ERROR] 열 수 없는 영상: {vpath}")
#             continue

#         # 원본 영상 정보
#         width  = int(cap.get(cv2.CAP_PROP_FRAME_WIDTH))
#         height = int(cap.get(cv2.CAP_PROP_FRAME_HEIGHT))
#         fps    = cap.get(cv2.CAP_PROP_FPS) or 30.0
#         fourcc = cv2.VideoWriter_fourcc(*"mp4v")
#         writer = cv2.VideoWriter(out_path, fourcc, fps, (width, height))

#         frame_idx = 0
#         t0 = time.time()

#         while True:
#             ret, frame = cap.read()
#             if not ret:
#                 break
#             frame_idx += 1

#             # YOLO 추론
#             results = model(frame, conf=yolo_conf, verbose=False)
#             boxes = results[0].boxes

#             # 기본 안내 문구(탐지 실패 대비)
#             base_text = "No dog detected"
#             base_color = (0, 0, 255)
#             drawn = False

#             if boxes is not None and len(boxes) > 0:
#                 xyxy = boxes.xyxy.cpu().numpy()
#                 cls  = boxes.cls.cpu().numpy()
#                 conf = boxes.conf.cpu().numpy()

#                 for i in range(len(xyxy)):
#                     if int(cls[i]) != DOG_CLASS_ID:
#                         continue
#                     x1, y1, x2, y2 = xyxy[i]
#                     yolo_c = conf[i]
#                     x1, y1, x2, y2 = safe_rect(x1, y1, x2, y2, width, height)
#                     roi = frame[y1:y2, x1:x2]

#                     name, match_score, id_conf = match_dog(roi, registered)
#                     size = pet_db.get(name, {}).get("size", "unknown") if name else "unknown"

#                     # ----- 이름 변환 (한글 → 영어) -----
#                     label_name = NAME_MAP.get(name, name if name else "Unknown")

#                     if name:
#                         color = (0, 255, 0)
#                         label = f"Dog: {label_name} | Match: {match_score} | ID_Conf: {id_conf}% | YOLO_Conf: {int(yolo_c*100)}% | Size: {size}"
#                     else:
#                         color = (0, 0, 255)
#                         label = f"Dog: None | Match: 0 | ID_Conf: 0% | YOLO_Conf: {int(yolo_c*100)}%"

#                     # 시각화
#                     cv2.rectangle(frame, (x1, y1), (x2, y2), color, 2)
#                     cv2.putText(frame, label, (x1, max(20, y1-10)),
#                                 cv2.FONT_HERSHEY_SIMPLEX, 0.6, color, 2)
#                     drawn = True

#             # 박스가 없을 때 기본 문구 출력
#             if not drawn:
#                 cv2.putText(frame, base_text, (20, 40),
#                             cv2.FONT_HERSHEY_SIMPLEX, 1.0, base_color, 2)

#             # FPS 표시
#             elapsed = max(1e-6, (time.time() - t0))
#             fps_est = frame_idx / elapsed
#             cv2.putText(frame, f"FPS: {fps_est:.1f}", (20, height - 20),
#                         cv2.FONT_HERSHEY_SIMPLEX, 0.6, (255, 255, 255), 2)

#             writer.write(frame)

#         cap.release()
#         writer.release()
#         print(f"[DONE] 저장 완료: {out_path}")


# if __name__ == "__main__":
#     ap = argparse.ArgumentParser()
#     ap.add_argument("--input_dir", default="test_videos")
#     ap.add_argument("--output_dir", default="result_videos")
#     ap.add_argument("--pet_dir", default="pet_images")
#     ap.add_argument("--pet_db", dest="pet_db_path", default="pet_db.json")
#     ap.add_argument("--weights", dest="yolo_weights", default="yolov8n.pt")
#     ap.add_argument("--conf", dest="yolo_conf", type=float, default=0.25)
#     ap.add_argument("--device", default=None, help="cuda:0 / mps / cpu 등 (선택)")
#     args = ap.parse_args()

#     main(
#         input_dir=args.input_dir,
#         output_dir=args.output_dir,
#         pet_dir=args.pet_dir,
#         pet_db_path=args.pet_db_path,
#         yolo_weights=args.yolo_weights,
#         yolo_conf=args.yolo_conf,
#         device=args.device
#     )

# # test_detect_dogs.py
# import os, json, time, argparse, cv2, math
# from collections import deque
# from ultralytics import YOLO
# from test_embedding_utils import load_registered_images, match_dog

# DOG_CLASS_ID = 16
# VIDEO_EXTS = (".mp4", ".mov", ".avi", ".mkv")
# NAME_MAP = {
#     "까미": "Kami","똘이": "Ddori","율무": "Yulmu","하트": "Heart",
#     "설기": "Seolgi","콩이": "Kong","삐삐": "Pippi","쿠키": "Cookie"
# }

# from collections import deque
# import math, time

# class IdentityStabilizer:
#     def __init__(self, confirm_frames=5, hold_frames=10, iou_thresh=0.1, ema_alpha=0.4):
#         self.confirm_frames = confirm_frames
#         self.hold_frames = hold_frames
#         self.iou_thresh = iou_thresh
#         self.ema_alpha = ema_alpha
#         self.buf = deque(maxlen=confirm_frames)

#         self.lock_name = None
#         self.lock_conf = 0
#         self.miss = 0
#         self.last_box = None

#         # 확정 관련
#         self.detect_start_time = None
#         self.confirmed_once = False  # 이미 확정 출력했는지

#     @staticmethod
#     def _iou(a, b):
#         if a is None or b is None: return 0.0
#         ax1, ay1, ax2, ay2 = a; bx1, by1, bx2, by2 = b
#         iw = max(0, min(ax2, bx2) - max(ax1, bx1))
#         ih = max(0, min(ay2, by2) - max(ay1, by1))
#         inter = iw * ih
#         area_a = max(0, (ax2-ax1)) * max(0, (ay2-ay1))
#         area_b = max(0, (bx2-bx1)) * max(0, (by2-by1))
#         denom = area_a + area_b - inter + 1e-6
#         return inter / denom

#     def update(self, name, conf, box):
#         """
#         name: 현재 프레임 추정 이름(or None)
#         conf: 현재 프레임 ID 신뢰도(0~100)
#         box:  (x1,y1,x2,y2)
#         return: (display_name, display_conf, locked_bool)
#         """
#         now = time.time()

#         # 첫 탐지 시작 시각 기록
#         if name and self.detect_start_time is None:
#             self.detect_start_time = now

#         # 새 이름이면 버퍼 추가
#         if name:
#             self.buf.append((name, conf))

#         # ---- 이미 락이 잡힌 상태 ----
#         if self.lock_name:
#             if name == self.lock_name and self._iou(self.last_box, box) >= self.iou_thresh:
#                 self.lock_conf = int(self.ema_alpha * conf + (1-self.ema_alpha) * self.lock_conf)
#                 self.miss = 0
#             else:
#                 self.miss += 1
#                 if self.miss >= self.hold_frames:
#                     print(f"[INFO] '{self.lock_name}' 세션 종료.")
#                     self.lock_name, self.lock_conf = None, 0
#                     self.buf.clear()
#                     self.detect_start_time = None
#                     self.confirmed_once = False
#         # ---- 아직 락이 없음 → 확정 시도 ----
#         else:
#             if len(self.buf) == self.buf.maxlen:
#                 names = [n for n,_ in self.buf]
#                 cand = max(set(names), key=names.count)
#                 freq = names.count(cand)
#                 if freq >= math.ceil(self.confirm_frames*0.6):
#                     avg_conf = int(sum(c for n,c in self.buf if n==cand) / freq)
#                     self.lock_name, self.lock_conf = cand, avg_conf
#                     self.miss = 0
#                     self.buf.clear()

#                     # 💡 확정 완료 시점 출력
#                     if self.detect_start_time and not self.confirmed_once:
#                         elapsed = now - self.detect_start_time
#                         print(f"[CONFIRM] '{cand}' 확정됨 — 소요시간: {elapsed:.2f}초, 평균 신뢰도 {avg_conf}%")
#                         self.confirmed_once = True

#         disp_name = self.lock_name if self.lock_name else (name or None)
#         disp_conf = self.lock_conf if self.lock_name else (conf if name else 0)
#         self.last_box = box if name else self.last_box
#         return disp_name, disp_conf, (self.lock_name is not None)


# def list_videos(input_dir):
#     files = []
#     if not os.path.isdir(input_dir):
#         print(f"[ERROR] 입력 폴더가 없습니다: {input_dir}")
#         return files
#     for root, _, fnames in os.walk(input_dir):
#         for fname in sorted(fnames):
#             if fname.lower().endswith(VIDEO_EXTS):
#                 files.append(os.path.join(root, fname))
#     return files

# def safe_rect(x1, y1, x2, y2, w, h):
#     x1 = max(0, min(int(x1), w-1)); y1 = max(0, min(int(y1), h-1))
#     x2 = max(0, min(int(x2), w-1)); y2 = max(0, min(int(y2), h-1))
#     if x2 <= x1: x2 = min(w-1, x1+1)
#     if y2 <= y1: y2 = min(h-1, y1+1)
#     return x1, y1, x2, y2

# def main(input_dir="test_videos", output_dir="result_videos",
#          pet_dir="pet_images", pet_db_path="pet_db.json",
#          yolo_weights="yolov8n.pt", yolo_conf=0.25, device=None):
#     os.makedirs(output_dir, exist_ok=True)

#     pet_db = {}
#     if os.path.isfile(pet_db_path):
#         try:
#             with open(pet_db_path, "r", encoding="utf-8") as f:
#                 pet_db = json.load(f)
#                 print(f"[INFO] pet_db 로드: {len(pet_db)}마리")
#         except Exception as e:
#             print("[WARN] pet_db.json 로드 실패:", e)

#     registered = load_registered_images(pet_dir, verbose=True)
#     print(f"[INFO] 등록된 강아지 수: {len(registered)}")

#     model = YOLO(yolo_weights)
#     if device is not None:
#         try:
#             model.to(device); print(f"[INFO] YOLO device: {device}")
#         except Exception as e:
#             print("[WARN] device 설정 실패(무시):", e)

#     videos = list_videos(input_dir)
#     if not videos:
#         print(f"[ERROR] 처리할 영상이 없습니다: {input_dir}")
#         return

#     for vpath in videos:
#         vname = os.path.basename(vpath)
#         out_path = os.path.join(output_dir, os.path.splitext(vname)[0] + "_result.mp4")
#         print(f"\n[PROCESS] {vname} -> {out_path}")

#         cap = cv2.VideoCapture(vpath)
#         if not cap.isOpened():
#             print(f"[ERROR] 열 수 없는 영상: {vpath}")
#             continue

#         width  = int(cap.get(cv2.CAP_PROP_FRAME_WIDTH))
#         height = int(cap.get(cv2.CAP_PROP_FRAME_HEIGHT))
#         fps    = cap.get(cv2.CAP_PROP_FPS) or 30.0
#         fourcc = cv2.VideoWriter_fourcc(*"mp4v")
#         writer = cv2.VideoWriter(out_path, fourcc, fps, (width, height))

#         frame_idx = 0
#         t0 = time.time()

#         stabilizer = IdentityStabilizer(confirm_frames=5, hold_frames=10, iou_thresh=0.1, ema_alpha=0.4)

#         while True:
#             ret, frame = cap.read()
#             if not ret: break
#             frame_idx += 1

#             results = model(frame, conf=yolo_conf, verbose=False)
#             boxes = results[0].boxes

#             base_text, base_color = "No dog detected", (0, 0, 255)
#             drawn = False

#             if boxes is not None and len(boxes) > 0:
#                 xyxy = boxes.xyxy.cpu().numpy()
#                 cls  = boxes.cls.cpu().numpy()
#                 conf = boxes.conf.cpu().numpy()

#                 for i in range(len(xyxy)):
#                     if int(cls[i]) != DOG_CLASS_ID: continue
#                     x1, y1, x2, y2 = xyxy[i]
#                     yolo_c = conf[i]
#                     x1, y1, x2, y2 = safe_rect(x1, y1, x2, y2, width, height)
#                     roi = frame[y1:y2, x1:x2]

#                     name, match_score, id_conf = match_dog(roi, registered)
#                     st_name, st_conf, locked = stabilizer.update(name, id_conf, (x1, y1, x2, y2))
#                     size = pet_db.get(st_name, {}).get("size", "unknown") if st_name else "unknown"

#                     if st_name:
#                         color = (0, 255, 0)
#                         label_name = NAME_MAP.get(st_name, st_name)
#                         label = f"Dog: {label_name} | ID_Conf: {st_conf}% | YOLO_Conf: {int(yolo_c*100)}% | Size: {size}"
#                     else:
#                         color = (0, 0, 255)
#                         label = f"Dog: None | ID_Conf: 0% | YOLO_Conf: {int(yolo_c*100)}%"

#                     cv2.rectangle(frame, (x1, y1), (x2, y2), color, 2)
#                     cv2.putText(frame, label, (x1, max(20, y1-10)),
#                                 cv2.FONT_HERSHEY_SIMPLEX, 0.6, color, 2)
#                     drawn = True

#             if not drawn:
#                 cv2.putText(frame, base_text, (20, 40),
#                             cv2.FONT_HERSHEY_SIMPLEX, 1.0, base_color, 2)

#             elapsed = max(1e-6, (time.time() - t0))
#             fps_est = frame_idx / elapsed
#             cv2.putText(frame, f"FPS: {fps_est:.1f}", (20, height - 20),
#                         cv2.FONT_HERSHEY_SIMPLEX, 0.6, (255,255,255), 2)

#             writer.write(frame)

#         cap.release()
#         writer.release()
#         print(f"[DONE] 저장 완료: {out_path}")

# if __name__ == "__main__":
#     ap = argparse.ArgumentParser()
#     ap.add_argument("--input_dir", default="test_videos")
#     ap.add_argument("--output_dir", default="result_videos")
#     ap.add_argument("--pet_dir", default="pet_images")
#     ap.add_argument("--pet_db", dest="pet_db_path", default="pet_db.json")
#     ap.add_argument("--weights", dest="yolo_weights", default="yolov8n.pt")
#     ap.add_argument("--conf", dest="yolo_conf", type=float, default=0.25)
#     ap.add_argument("--device", default=None, help="cuda:0 / mps / cpu")
#     args = ap.parse_args()

#     main(
#         input_dir=args.input_dir,
#         output_dir=args.output_dir,
#         pet_dir=args.pet_dir,
#         pet_db_path=args.pet_db_path,
#         yolo_weights=args.yolo_weights,
#         yolo_conf=args.yolo_conf,
#         device=args.device
#     )


# test_detect_dogs.py
# test_videos/ 하위 모든 영상(.mp4/.mov/.avi/.mkv)을 읽어
# result_videos/원본이름_result.mp4로 저장 + 확정 시각(ms) 요약 출력

# import os
# import json
# import time
# import argparse
# import cv2
# from ultralytics import YOLO

# from test_embedding_utils import load_registered_images, match_dog

# DOG_CLASS_ID = 16  # COCO: dog
# VIDEO_EXTS = (".mp4", ".mov", ".avi", ".mkv")

# # 확정(confirmation) 기준
# YOLO_MIN_CONF = 0.20   # YOLO 확신도 최소 (0~1)
# ID_MIN_CONF   = 10     # ORB ID 신뢰도 최소 (0~100)
# CONFIRM_N     = 3      # 같은 이름이 N프레임 연속이면 '확정'

# def list_videos(input_dir):
#     files = []
#     if not os.path.isdir(input_dir):
#         print(f"[ERROR] 입력 폴더가 없습니다: {input_dir}")
#         return files
#     # 하위 폴더까지 모두 탐색
#     for root, _, fnames in os.walk(input_dir):
#         for fname in sorted(fnames):
#             if fname.lower().endswith(VIDEO_EXTS):
#                 files.append(os.path.join(root, fname))
#     return files

# def safe_rect(x1, y1, x2, y2, w, h):
#     x1 = max(0, min(int(x1), w-1))
#     y1 = max(0, min(int(y1), h-1))
#     x2 = max(0, min(int(x2), w-1))
#     y2 = max(0, min(int(y2), h-1))
#     if x2 <= x1: x2 = min(w-1, x1+1)
#     if y2 <= y1: y2 = min(h-1, y1+1)
#     return x1, y1, x2, y2

# def robust_match(roi_bgr, registered):
#     """match_dog이 (name, score) 또는 (name, score, id_conf) 둘 다 대응."""
#     name, score, id_conf = None, 0, 0
#     try:
#         out = match_dog(roi_bgr, registered)
#         if isinstance(out, tuple):
#             if len(out) == 3:
#                 name, score, id_conf = out
#             elif len(out) == 2:
#                 name, score = out
#                 id_conf = max(0, min(100, int(score)))
#             else:
#                 name = out[0] if len(out) > 0 else None
#                 score = out[1] if len(out) > 1 else 0
#                 id_conf = max(0, min(100, int(score)))
#     except Exception:
#         pass
#     return name, int(score), int(id_conf)

# def main(
#     input_dir="test_videos",
#     output_dir="result_videos",
#     pet_dir="pet_images",
#     pet_db_path="pet_db.json",
#     yolo_weights="yolov8n.pt",
#     yolo_conf=0.25,
#     device=None
# ):
#     os.makedirs(output_dir, exist_ok=True)

#     # 강아지 DB(선택)
#     pet_db = {}
#     if os.path.isfile(pet_db_path):
#         try:
#             with open(pet_db_path, "r", encoding="utf-8") as f:
#                 pet_db = json.load(f)
#                 print(f"[INFO] pet_db 로드: {len(pet_db)}마리")
#         except Exception as e:
#             print("[WARN] pet_db.json 로드 실패:", e)

#     # 등록 이미지 로드
#     registered = load_registered_images(pet_dir, verbose=True)
#     print(f"[INFO] 등록된 강아지 수: {len(registered)}")

#     # YOLO 로드
#     model = YOLO(yolo_weights)
#     if device is not None:
#         try:
#             model.to(device)
#             print(f"[INFO] YOLO device: {device}")
#         except Exception as e:
#             print("[WARN] device 설정 실패(무시하고 자동 선택):", e)

#     videos = list_videos(input_dir)
#     if not videos:
#         print(f"[ERROR] 처리할 영상이 없습니다: {input_dir}")
#         return

#     for vpath in videos:
#         vname = os.path.basename(vpath)
#         out_path = os.path.join(
#             output_dir,
#             os.path.splitext(vname)[0] + "_result.mp4"
#         )
#         print(f"\n[PROCESS] {vname} -> {out_path}")

#         cap = cv2.VideoCapture(vpath)
#         if not cap.isOpened():
#             print(f"[ERROR] 열 수 없는 영상: {vpath}")
#             continue

#         width  = int(cap.get(cv2.CAP_PROP_FRAME_WIDTH))
#         height = int(cap.get(cv2.CAP_PROP_FRAME_HEIGHT))
#         fps    = cap.get(cv2.CAP_PROP_FPS) or 30.0
#         fourcc = cv2.VideoWriter_fourcc(*"mp4v")
#         writer = cv2.VideoWriter(out_path, fourcc, fps, (width, height))

#         # 타이밍 기준: "영상 시작부터 확정까지"
#         t0 = time.time()
#         confirmed_ms = None
#         confirmed_name = None
#         # 연속 확인 버퍼
#         confirm_buf = []

#         frame_idx = 0
#         while True:
#             ret, frame = cap.read()
#             if not ret:
#                 break
#             frame_idx += 1

#             results = model(frame, conf=yolo_conf, verbose=False)
#             boxes = results[0].boxes

#             base_text = "No dog detected"
#             base_color = (0, 0, 255)
#             drawn = False

#             best = None  # 가장 신뢰도 높은 박스 선택
#             if boxes is not None and len(boxes) > 0:
#                 xyxy = boxes.xyxy.cpu().numpy()
#                 cls  = boxes.cls.cpu().numpy()
#                 conf = boxes.conf.cpu().numpy()

#                 for i in range(len(xyxy)):
#                     if int(cls[i]) != DOG_CLASS_ID:
#                         continue
#                     x1, y1, x2, y2 = xyxy[i]
#                     yolo_c = float(conf[i])
#                     x1, y1, x2, y2 = safe_rect(x1, y1, x2, y2, width, height)
#                     roi = frame[y1:y2, x1:x2]

#                     name, match_score, id_conf = robust_match(roi, registered)
#                     size = pet_db.get(name, {}).get("size", "unknown") if name else "unknown"

#                     # 가장 신뢰도 높은 후보 저장
#                     if best is None or yolo_c > best["yolo_c"]:
#                         best = {
#                             "name": name,
#                             "yolo_c": yolo_c,
#                             "id_conf": id_conf,
#                             "match_score": match_score,
#                             "rect": (x1, y1, x2, y2),
#                             "size": size
#                         }

#             # 확정 로직 처리
#             if confirmed_ms is None:
#                 if best is not None \
#                    and best["name"] is not None \
#                    and best["yolo_c"] >= YOLO_MIN_CONF \
#                    and best["id_conf"] >= ID_MIN_CONF:
#                     confirm_buf.append(best["name"])
#                     if len(confirm_buf) > CONFIRM_N:
#                         confirm_buf.pop(0)
#                     if len(confirm_buf) == CONFIRM_N and len(set(confirm_buf)) == 1:
#                         confirmed_ms = int((time.time() - t0) * 1000)
#                         confirmed_name = confirm_buf[-1]
#                         print(f"[SUMMARY] {vname} confirmed_at={confirmed_ms}ms "
#                               f"(name={confirmed_name}, frames={CONFIRM_N})")
#                 else:
#                     # 조건 미충족이면 버퍼 초기화(끊김 방지하려면 주석 처리 가능)
#                     confirm_buf.clear()

#             # 시각화
#             if best is not None:
#                 x1, y1, x2, y2 = best["rect"]
#                 if confirmed_ms is not None:
#                     color = (0, 255, 0)
#                     label = (f"CONFIRMED: {confirmed_name} | "
#                              f"ID_Conf: {best['id_conf']}% | YOLO_Conf: {int(best['yolo_c']*100)}% "
#                              f"| Size: {best['size']}")
#                 else:
#                     color = (0, 165, 255)  # 주황: 후보 단계
#                     label = (f"Candidate: {best['name'] or 'None'} | "
#                              f"ID_Conf: {best['id_conf']}% | YOLO_Conf: {int(best['yolo_c']*100)}% "
#                              f"| Size: {best['size']}")
#                 cv2.rectangle(frame, (x1, y1), (x2, y2), color, 2)
#                 cv2.putText(frame, label, (x1, max(20, y1-10)),
#                             cv2.FONT_HERSHEY_SIMPLEX, 0.6, color, 2)
#                 drawn = True

#             if not drawn:
#                 cv2.putText(frame, base_text, (20, 40),
#                             cv2.FONT_HERSHEY_SIMPLEX, 1.0, base_color, 2)

#             # FPS 표시(대략)
#             elapsed = max(1e-6, (time.time() - t0))
#             fps_est = frame_idx / elapsed
#             cv2.putText(frame, f"FPS: {fps_est:.1f}", (20, height - 20),
#                         cv2.FONT_HERSHEY_SIMPLEX, 0.6, (255, 255, 255), 2)

#             writer.write(frame)

#         cap.release()
#         writer.release()
#         print(f"[DONE] 저장 완료: {out_path}")
#         # 확정이 끝까지 한 번도 안 되면 None으로 남음
#         if confirmed_ms is None:
#             print(f"[SUMMARY] {vname} confirmed_at=None (no stable confirmation)")

# if __name__ == "__main__":
#     ap = argparse.ArgumentParser()
#     ap.add_argument("--input_dir", default="test_videos")
#     ap.add_argument("--output_dir", default="result_videos")
#     ap.add_argument("--pet_dir", default="pet_images")
#     ap.add_argument("--pet_db", dest="pet_db_path", default="pet_db.json")
#     ap.add_argument("--weights", dest="yolo_weights", default="yolov8n.pt")
#     ap.add_argument("--conf", dest="yolo_conf", type=float, default=0.25)
#     ap.add_argument("--device", default=None, help="cuda:0 / mps / cpu 등 (선택)")
#     args = ap.parse_args()

#     main(
#         input_dir=args.input_dir,
#         output_dir=args.output_dir,
#         pet_dir=args.pet_dir,
#         pet_db_path=args.pet_db_path,
#         yolo_weights=args.yolo_weights,
#         yolo_conf=args.yolo_conf,
#         device=args.device
#     )


# # test_detect_dog.py
# import os
# import json
# import time
# import argparse
# import cv2
# import numpy as np
# import torch
# import torchvision.transforms as T
# from torchvision import models
# from ultralytics import YOLO

# from test_embedding_utils import (
#     load_registered_images,        # ORB descriptors
#     load_registered_color_images,  # BGR images for breed embedding
#     match_dog
# )

# DOG_CLASS_ID = 16  # COCO: dog
# VIDEO_EXTS = (".mp4", ".mov", ".avi", ".mkv")

# # 확정(confirmation) 기준
# YOLO_MIN_CONF    = 0.20   # YOLO 확신도 최소 (0~1)
# ID_MIN_CONF      = 10     # ORB ID 신뢰도 최소 (0~100)
# BREED_MIN_CONF   = 0.35   # 품종 임베딩 정규화 점수(0~1)
# CONFIRM_N        = 3      # 같은 결과가 N프레임 연속이면 '확정'

# # -------------------- 유틸 --------------------
# def list_videos(input_dir):
#     files = []
#     if not os.path.isdir(input_dir):
#         print(f"[ERROR] 입력 폴더가 없습니다: {input_dir}")
#         return files
#     for root, _, fnames in os.walk(input_dir):
#         for fname in sorted(fnames):
#             if fname.lower().endswith(VIDEO_EXTS):
#                 files.append(os.path.join(root, fname))
#     return files

# def safe_rect(x1, y1, x2, y2, w, h):
#     x1 = max(0, min(int(x1), w-1))
#     y1 = max(0, min(int(y1), h-1))
#     x2 = max(0, min(int(x2), w-1))
#     y2 = max(0, min(int(y2), h-1))
#     if x2 <= x1: x2 = min(w-1, x1+1)
#     if y2 <= y1: y2 = min(h-1, y1+1)
#     return x1, y1, x2, y2

# def robust_match(roi_bgr, registered, allowed_names=None):
#     """
#     match_dog이 (name, score) 또는 (name, score, id_conf) 둘 다 대응.
#     allowed_names가 주어지면 그 이름들만 대상으로 제한.
#     """
#     subset = registered
#     if allowed_names is not None:
#         subset = {k: v for k, v in registered.items() if k in allowed_names}

#     name, score, id_conf = None, 0, 0
#     try:
#         out = match_dog(roi_bgr, subset)
#         if isinstance(out, tuple):
#             if len(out) == 3:
#                 name, score, id_conf = out
#             elif len(out) == 2:
#                 name, score = out
#                 id_conf = max(0, min(100, int(score)))
#             else:
#                 name = out[0] if len(out) > 0 else None
#                 score = out[1] if len(out) > 1 else 0
#                 id_conf = max(0, min(100, int(score)))
#     except Exception:
#         pass
#     return name, int(score), int(id_conf)

# # --- BreedIndexer (full) ---
# import cv2
# import numpy as np
# import torch
# import torchvision.transforms as T
# from torchvision import models
# from PIL import Image
# from collections import defaultdict

# class BreedIndexer:
#     """
#     - 등록 컬러 이미지로 품종별 임베딩(ResNet50 feature) 센트로이드 계산
#     - ROI에서 임베딩 추출 후 코사인 유사도로 품종 예측
#     - 한글 breed 라벨도 그대로 사용 가능 (문자열 식별자)
#     """
#     def __init__(self, color_images_by_name, pet_db, device='cpu'):
#         self.device = torch.device(
#             device if device else ('cuda' if torch.cuda.is_available() else 'cpu')
#         )

#         # --- torchvision 버전 호환: 신형(Weights API) / 구형(pretrained=True) 모두 지원 ---
#         ResNet50_Weights = getattr(models, "ResNet50_Weights", None)
#         if ResNet50_Weights is not None:
#             # 신형 API (torchvision >= 0.13~)
#             weights = ResNet50_Weights.DEFAULT  # 혹은 .IMAGENET1K_V2
#             self.model = models.resnet50(weights=weights)
#             self.model.fc = torch.nn.Identity()
#             self.model.eval().to(self.device)

#             # Weights가 제공하는 권장 전처리(Resize/CenterCrop/ToTensor/Normalize 등) 사용
#             self.transform = weights.transforms()
#             self._expects_pil = True  # weights.transforms()는 PIL 입력이 안전
#         else:
#             # 구형 API (옛 torchvision)
#             self.model = models.resnet50(pretrained=True)
#             self.model.fc = torch.nn.Identity()
#             self.model.eval().to(self.device)

#             # 표준 ImageNet 정규화 파이프라인
#             self.transform = T.Compose([
#                 T.ToPILImage(),         # PIL 변환 포함
#                 T.Resize(256),
#                 T.CenterCrop(224),
#                 T.ToTensor(),
#                 T.Normalize(mean=[0.485, 0.456, 0.406],
#                             std=[0.229, 0.224, 0.225]),
#             ])
#             self._expects_pil = False  # 위 파이프라인은 np→PIL도 수용

#         # 이름 ↔ 품종 매핑
#         self.name_to_breed = {}
#         self.breed_to_names = defaultdict(list)
#         for name, meta in (pet_db or {}).items():
#             breed = (meta or {}).get('breed')
#             if not breed:
#                 continue
#             self.name_to_breed[name] = breed
#             self.breed_to_names[breed].append(name)

#         # --- 품종별 센트로이드 계산 ---
#         # 모든 이름의 모든 이미지 임베딩을 같은 품종에 모아서 평균 → 단순/견고
#         breed_vecs = defaultdict(list)

#         with torch.no_grad():
#             for name, imgs in (color_images_by_name or {}).items():
#                 breed = self.name_to_breed.get(name)
#                 if not breed or not imgs:
#                     continue
#                 for bgr in imgs:
#                     emb = self._embed(bgr)  # L2-normalized
#                     if emb is not None:
#                         breed_vecs[breed].append(emb)

#         self.breed_centroids = {}
#         for breed, vecs in breed_vecs.items():
#             if not vecs:
#                 continue
#             mean_emb = np.mean(np.stack(vecs, axis=0), axis=0)
#             nrm = np.linalg.norm(mean_emb) + 1e-8
#             self.breed_centroids[breed] = (mean_emb / nrm).astype(np.float32)

#     def _embed(self, roi_bgr):
#         """
#         ROI(BGR, np.ndarray HxWx3) -> 2048차원 L2-normalized 임베딩 (np.ndarray)
#         """
#         if roi_bgr is None or roi_bgr.size == 0:
#             return None
#         # OpenCV는 BGR, torchvision은 RGB/PIL이 안전
#         rgb = cv2.cvtColor(roi_bgr, cv2.COLOR_BGR2RGB)

#         # 변환 파이프라인이 PIL을 기대하는 경우를 고려해 항상 PIL로 전달(가장 안전)
#         img_pil = Image.fromarray(rgb)

#         ten = self.transform(img_pil).unsqueeze(0).to(self.device)
#         with torch.no_grad():
#             feat = self.model(ten)  # [1, 2048]
#             emb = torch.nn.functional.normalize(feat, dim=1).squeeze(0).cpu().numpy()
#         return emb  # L2-normalized

#     def predict_breed(self, roi_bgr, topk=3):
#         """
#         입력 ROI에 대해 품종 상위 k개를 반환.
#         return: List[(breed: str, conf: float 0~1)]
#         conf는 모든 품종의 코사인 유사도를 min-max 정규화한 상대 점수.
#         """
#         if not self.breed_centroids:
#             return []
#         q = self._embed(roi_bgr)
#         if q is None:
#             return []
#         scores = []
#         for breed, c in self.breed_centroids.items():
#             sim = float(np.dot(q, c))  # cosine (둘 다 L2-normalized)
#             scores.append((breed, sim))
#         scores.sort(key=lambda x: x[1], reverse=True)

#         # min-max 정규화(상대적 0~1). 품종이 1개뿐이면 1.0
#         sims = np.array([s for _, s in scores], dtype=np.float32)
#         if len(sims) >= 2:
#             mn, mx = float(sims.min()), float(sims.max())
#             den = (mx - mn) + 1e-8
#             norm = [(b, (s - mn) / den) for (b, s) in scores[:max(1, topk)]]
#         else:
#             norm = [(scores[0][0], 1.0)]
#         return norm

#     def names_in_breed(self, breed):
#         """해당 품종에 등록된 이름 리스트 반환."""
#         return list(self.breed_to_names.get(breed, []))


# # -------------------- 메인 --------------------
# def main(
#     input_dir="test_videos",
#     output_dir="result_videos",
#     pet_dir="pet_images",
#     pet_db_path="pet_db.json",
#     yolo_weights="yolov8n.pt",
#     yolo_conf=0.25,
#     device=None
# ):
#     os.makedirs(output_dir, exist_ok=True)

#     # 강아지 DB(선택)
#     pet_db = {}
#     if os.path.isfile(pet_db_path):
#         try:
#             with open(pet_db_path, "r", encoding="utf-8") as f:
#                 pet_db = json.load(f)
#                 print(f"[INFO] pet_db 로드: {len(pet_db)}마리")
#         except Exception as e:
#             print("[WARN] pet_db.json 로드 실패:", e)

#     # 등록 이미지 로드
#     registered_orb = load_registered_images(pet_dir, verbose=True)
#     print(f"[INFO] 등록된 강아지(ORB) 수: {len(registered_orb)}")

#     # 품종 임베딩용 컬러 이미지 로드
#     registered_color = load_registered_color_images(pet_dir, verbose=True)
#     print(f"[INFO] 등록된 강아지(컬러) 수: {len(registered_color)}")

#     # Breed Indexer
#     breed_index = BreedIndexer(registered_color, pet_db, device=device or 'cpu')

#     # YOLO 로드
#     model = YOLO(yolo_weights)
#     if device is not None:
#         try:
#             model.to(device)
#             print(f"[INFO] YOLO device: {device}")
#         except Exception as e:
#             print("[WARN] device 설정 실패(무시하고 자동 선택):", e)

#     videos = list_videos(input_dir)
#     if not videos:
#         print(f"[ERROR] 처리할 영상이 없습니다: {input_dir}")
#         return

#     for vpath in videos:
#         vname = os.path.basename(vpath)
#         out_path = os.path.join(
#             output_dir,
#             os.path.splitext(vname)[0] + "_result.mp4"
#         )
#         print(f"\n[PROCESS] {vname} -> {out_path}")

#         cap = cv2.VideoCapture(vpath)
#         if not cap.isOpened():
#             print(f"[ERROR] 열 수 없는 영상: {vpath}")
#             continue

#         width  = int(cap.get(cv2.CAP_PROP_FRAME_WIDTH))
#         height = int(cap.get(cv2.CAP_PROP_FRAME_HEIGHT))
#         fps    = cap.get(cv2.CAP_PROP_FPS) or 30.0
#         fourcc = cv2.VideoWriter_fourcc(*"mp4v")
#         writer = cv2.VideoWriter(out_path, fourcc, fps, (width, height))

#         # 타이밍 기준: "영상 시작부터 확정까지"
#         t0 = time.time()
#         confirmed_ms   = None
#         confirmed_name = None
#         confirmed_breed= None

#         # 연속 확인 버퍼(품종/이름 분리)
#         breed_confirm_buf = []
#         name_confirm_buf  = []

#         frame_idx = 0
#         while True:
#             ret, frame = cap.read()
#             if not ret:
#                 break
#             frame_idx += 1

#             results = model(frame, conf=yolo_conf, verbose=False)
#             boxes = results[0].boxes

#             base_text = "No dog detected"
#             base_color = (0, 0, 255)
#             drawn = False

#             best = None  # 가장 신뢰도 높은 박스 선택
#             if boxes is not None and len(boxes) > 0:
#                 xyxy = boxes.xyxy.cpu().numpy()
#                 cls  = boxes.cls.cpu().numpy()
#                 conf = boxes.conf.cpu().numpy()

#                 for i in range(len(xyxy)):
#                     if int(cls[i]) != DOG_CLASS_ID:
#                         continue
#                     x1, y1, x2, y2 = xyxy[i]
#                     yolo_c = float(conf[i])
#                     x1, y1, x2, y2 = safe_rect(x1, y1, x2, y2, width, height)

#                     # 박스에 12~15% 여백을 추가(특징 더 확보)
#                     pad_x = int(0.12 * (x2 - x1))
#                     pad_y = int(0.12 * (y2 - y1))
#                     x1p = max(0, x1 - pad_x); y1p = max(0, y1 - pad_y)
#                     x2p = min(width-1, x2 + pad_x); y2p = min(height-1, y2 + pad_y)

#                     roi = frame[y1p:y2p, x1p:x2p]

#                     # 1) 품종 예측 (top-1만 사용)
#                     breed_scores = breed_index.predict_breed(roi, topk=3)
#                     top_breed, breed_conf = (breed_scores[0] if breed_scores else (None, 0.0))

#                     # 2) 이름 매칭(품종으로 후보 제한)
#                     allowed = None
#                     if top_breed and breed_conf >= BREED_MIN_CONF:
#                         allowed = set(breed_index.names_in_breed(top_breed))
#                     name, match_score, id_conf = robust_match(roi, registered_orb, allowed_names=allowed)

#                     size = pet_db.get(name, {}).get("size", "unknown") if name else "unknown"

#                     # 가장 신뢰도 높은 후보 저장
#                     cand = {
#                         "name": name,
#                         "yolo_c": yolo_c,
#                         "id_conf": id_conf,
#                         "match_score": match_score,
#                         "rect": (x1, y1, x2, y2),
#                         "rect_pad": (x1p, y1p, x2p, y2p),
#                         "size": size,
#                         "breed": top_breed,
#                         "breed_conf": breed_conf
#                     }
#                     if best is None or yolo_c > best["yolo_c"]:
#                         best = cand

#             # 확정 로직 처리
#             if best is not None and confirmed_ms is None:
#                 # (a) 품종 확정
#                 if best["breed"] and best["breed_conf"] >= BREED_MIN_CONF and best["yolo_c"] >= YOLO_MIN_CONF:
#                     breed_confirm_buf.append(best["breed"])
#                     if len(breed_confirm_buf) > CONFIRM_N:
#                         breed_confirm_buf.pop(0)
#                     if len(breed_confirm_buf) == CONFIRM_N and len(set(breed_confirm_buf)) == 1:
#                         confirmed_breed = breed_confirm_buf[-1]

#                 # (b) 이름 확정(품종이 확정되었거나, id_conf가 충분히 높을 때)
#                 if best["name"] and best["id_conf"] >= ID_MIN_CONF and best["yolo_c"] >= YOLO_MIN_CONF:
#                     name_confirm_buf.append(best["name"])
#                     if len(name_confirm_buf) > CONFIRM_N:
#                         name_confirm_buf.pop(0)
#                     if len(name_confirm_buf) == CONFIRM_N and len(set(name_confirm_buf)) == 1:
#                         confirmed_name = name_confirm_buf[-1]
#                         confirmed_ms = int((time.time() - t0) * 1000)
#                         print(f"[SUMMARY] {vname} confirmed_at={confirmed_ms}ms "
#                               f"(breed={confirmed_breed or best['breed']}, name={confirmed_name}, frames={CONFIRM_N})")
#                 else:
#                     # 조건 미충족이면 이름 버퍼만 초기화(품종 버퍼는 유지 가능)
#                     name_confirm_buf.clear()

#             # 시각화
#             if best is not None:
#                 x1, y1, x2, y2 = best["rect"]
#                 if confirmed_ms is not None:
#                     color = (0, 255, 0)
#                     label = (f"CONFIRMED: {confirmed_name} ({confirmed_breed or best['breed']}) | "
#                              f"ID_Conf: {best['id_conf']}% | YOLO: {int(best['yolo_c']*100)}% "
#                              f"| BreedConf: {int(best['breed_conf']*100)}% | Size: {best['size']}")
#                 else:
#                     color = (0, 165, 255)  # 후보 단계
#                     btop = f"{best['breed']}({int(best['breed_conf']*100)}%)" if best['breed'] else "None"
#                     label = (f"Candidate: {best['name'] or 'None'} | Breed: {btop} | "
#                              f"ID_Conf: {best['id_conf']}% | YOLO: {int(best['yolo_c']*100)}% | Size: {best['size']}")
#                 cv2.rectangle(frame, (x1, y1), (x2, y2), color, 2)
#                 cv2.putText(frame, label, (x1, max(20, y1-10)),
#                             cv2.FONT_HERSHEY_SIMPLEX, 0.6, color, 2)
#                 drawn = True

#             if not drawn:
#                 cv2.putText(frame, base_text, (20, 40),
#                             cv2.FONT_HERSHEY_SIMPLEX, 1.0, base_color, 2)

#             # FPS 표시(대략)
#             elapsed = max(1e-6, (time.time() - t0))
#             fps_est = frame_idx / elapsed
#             cv2.putText(frame, f"FPS: {fps_est:.1f}", (20, height - 20),
#                         cv2.FONT_HERSHEY_SIMPLEX, 0.6, (255, 255, 255), 2)

#             writer.write(frame)

#         cap.release()
#         writer.release()
#         print(f"[DONE] 저장 완료: {out_path}")
#         if confirmed_ms is None:
#             print(f"[SUMMARY] {vname} confirmed_at=None (no stable confirmation)")

# if __name__ == "__main__":
#     ap = argparse.ArgumentParser()
#     ap.add_argument("--input_dir", default="test_videos")
#     ap.add_argument("--output_dir", default="result_videos")
#     ap.add_argument("--pet_dir", default="pet_images")
#     ap.add_argument("--pet_db", dest="pet_db_path", default="pet_db.json")
#     ap.add_argument("--weights", dest="yolo_weights", default="yolov8n.pt")
#     ap.add_argument("--conf", dest="yolo_conf", type=float, default=0.25)
#     ap.add_argument("--device", default=None, help="cuda:0 / mps / cpu / cuda:1 ...")
#     args = ap.parse_args()

#     main(
#         input_dir=args.input_dir,
#         output_dir=args.output_dir,
#         pet_dir=args.pet_dir,
#         pet_db_path=args.pet_db_path,
#         yolo_weights=args.yolo_weights,
#         yolo_conf=args.yolo_conf,
#         device=args.device
#     )


# # test_detect_dog.py
# import os
# import json
# import time
# import argparse

# import cv2
# import numpy as np
# import torch
# import torchvision.transforms as T
# from torchvision import models
# from PIL import Image, ImageDraw, ImageFont
# from ultralytics import YOLO

# from test_embedding_utils import (
#     load_registered_images,        # ORB descriptors
#     load_registered_color_images,  # BGR images for breed embedding
#     match_dog
# )

# # =========================
# # 설정 값 (완화된 기본값)
# # =========================
# DOG_CLASS_ID      = 16   # COCO: dog
# VIDEO_EXTS        = (".mp4", ".mov", ".avi", ".mkv")

# YOLO_MIN_CONF     = 0.15  # YOLO 확신도 최소 (0~1)
# ID_MIN_CONF       = 5     # ORB ID 신뢰도 최소 (0~100)
# BREED_MIN_CONF    = 0.25  # 품종 임베딩 정규화 점수(0~1)
# CONFIRM_N         = 2     # 같은 결과가 N프레임 연속이면 '확정'

# DEBUG_PRINT_EVERY = 10    # N프레임마다 디버깅 로그 출력
# BREED_HIST_MAX    = 12    # 품종 히스토리 최대 길이
# BREED_HIST_REQ    = 3     # 최소 등장 횟수
# BREED_HIST_RATIO  = 0.6   # 최빈 품종이 차지해야 하는 비율

# # (선택) 품종 → 기본 대표 이름(정책)
# DEFAULT_NAME_BY_BREED = {
#     # "요크셔테리어": "삐삐",
#     # "푸들": "쿠키",
#     # "말티즈": "설기",
#     # "리트리버": "벤지",
#     # "스패니얼": "율무",
#     # "사모예드": "모모",
# }

# # =========================
# # 유틸
# # =========================
# def list_videos(input_dir):
#     files = []
#     if not os.path.isdir(input_dir):
#         print(f"[ERROR] 입력 폴더가 없습니다: {input_dir}")
#         return files
#     for root, _, fnames in os.walk(input_dir):
#         for fname in sorted(fnames):
#             if fname.lower().endswith(VIDEO_EXTS):
#                 files.append(os.path.join(root, fname))
#     return files

# def safe_rect(x1, y1, x2, y2, w, h):
#     x1 = max(0, min(int(x1), w-1))
#     y1 = max(0, min(int(y1), h-1))
#     x2 = max(0, min(int(x2), w-1))
#     y2 = max(0, min(int(y2), h-1))
#     if x2 <= x1: x2 = min(w-1, x1+1)
#     if y2 <= y1: y2 = min(h-1, y1+1)
#     return x1, y1, x2, y2

# def robust_match(roi_bgr, registered, allowed_names=None):
#     """
#     match_dog이 (name, score) 또는 (name, score, id_conf) 둘 다 대응.
#     allowed_names가 주어지면 그 이름들만 대상으로 제한.
#     """
#     subset = registered
#     if allowed_names is not None:
#         subset = {k: v for k, v in registered.items() if k in allowed_names}
#         if not subset:
#             subset = registered  # 품종에 해당 이름 폴더가 비면 전체로 후퇴

#     name, score, id_conf = None, 0, 0
#     try:
#         out = match_dog(roi_bgr, subset)
#         if isinstance(out, tuple):
#             if len(out) == 3:
#                 name, score, id_conf = out
#             elif len(out) == 2:
#                 name, score = out
#                 id_conf = max(0, min(100, int(score)))
#             else:
#                 name = out[0] if len(out) > 0 else None
#                 score = out[1] if len(out) > 1 else 0
#                 id_conf = max(0, min(100, int(score)))
#     except Exception:
#         pass
#     return name, int(score), int(id_conf)

# def draw_text_korean(frame_bgr, text, org, font_size=20, color_bgr=(0,165,255), stroke=2):
#     """cv2.putText가 한글을 못 찍는 문제 해결: PIL로 렌더링."""
#     font_paths = [
#         "/System/Library/Fonts/AppleSDGothicNeo.ttc",  # macOS
#         "/Library/Fonts/AppleGothic.ttf",
#         "/usr/share/fonts/truetype/nanum/NanumGothic.ttf",  # Linux
#         "C:/Windows/Fonts/malgun.ttf",  # Windows
#     ]
#     font = None
#     for p in font_paths:
#         try:
#             font = ImageFont.truetype(p, font_size)
#             break
#         except Exception:
#             continue
#     if font is None:
#         # 폰트가 없으면 영어만이라도 표기
#         cv2.putText(frame_bgr, text, org, cv2.FONT_HERSHEY_SIMPLEX, 0.6, color_bgr, 2, cv2.LINE_AA)
#         return frame_bgr

#     img_pil = Image.fromarray(cv2.cvtColor(frame_bgr, cv2.COLOR_BGR2RGB))
#     draw = ImageDraw.Draw(img_pil)
#     x, y = org
#     # 외곽선
#     if stroke and stroke > 0:
#         for dx in (-stroke, 0, stroke):
#             for dy in (-stroke, 0, stroke):
#                 if dx == 0 and dy == 0: 
#                     continue
#                 draw.text((x+dx, y+dy), text, font=font, fill=(0,0,0))
#     # 본문
#     b,g,r = color_bgr
#     draw.text((x, y), text, font=font, fill=(r,g,b))
#     return cv2.cvtColor(np.array(img_pil), cv2.COLOR_RGB2BGR)

# def pick_name_by_breed(breed, breed_index, pet_db):
#     """품종만 확정된 경우 대표 이름을 정책적으로 선택."""
#     # 정책 맵 우선
#     if breed in DEFAULT_NAME_BY_BREED and DEFAULT_NAME_BY_BREED[breed] in pet_db:
#         return DEFAULT_NAME_BY_BREED[breed]
#     # 해당 품종에 등록된 이름들
#     cand = breed_index.names_in_breed(breed)
#     if not cand:
#         return None
#     if len(cand) == 1:
#         return cand[0]
#     return sorted(cand)[0]  # 사전순

# # =========================
# # 품종 인덱서
# # =========================
# class BreedIndexer:
#     """
#     - 등록 컬러 이미지로 품종별 임베딩(ResNet50 feature) 센트로이드 계산
#     - ROI에서 임베딩 추출 후 코사인 유사도로 품종 예측
#     - 한글 breed 라벨도 그대로 사용 가능 (문자열 식별자)
#     """
#     def __init__(self, color_images_by_name, pet_db, device='cpu'):
#         self.device = torch.device(device if device else ('cuda' if torch.cuda.is_available() else 'cpu'))

#         # torchvision 버전 호환
#         ResNet50_Weights = getattr(models, "ResNet50_Weights", None)
#         if ResNet50_Weights is not None:
#             weights = ResNet50_Weights.DEFAULT  # 또는 IMAGENET1K_V2
#             self.model = models.resnet50(weights=weights)
#             self.model.fc = torch.nn.Identity()
#             self.model.eval().to(self.device)
#             self.transform = weights.transforms()  # 권장 전처리
#         else:
#             self.model = models.resnet50(pretrained=True)
#             self.model.fc = torch.nn.Identity()
#             self.model.eval().to(self.device)
#             self.transform = T.Compose([
#                 T.ToPILImage(),
#                 T.Resize(256),
#                 T.CenterCrop(224),
#                 T.ToTensor(),
#                 T.Normalize(mean=[0.485, 0.456, 0.406],
#                             std=[0.229, 0.224, 0.225]),
#             ])

#         # 이름 ↔ 품종 매핑
#         self.name_to_breed = {}
#         self.breed_to_names = {}
#         for name, meta in (pet_db or {}).items():
#             breed = (meta or {}).get('breed')
#             if not breed:
#                 continue
#             self.name_to_breed[name] = breed
#             self.breed_to_names.setdefault(breed, []).append(name)

#         # 품종별 센트로이드
#         breed_vecs = {}
#         for breed in self.breed_to_names.keys():
#             breed_vecs[breed] = []

#         with torch.no_grad():
#             for name, imgs in (color_images_by_name or {}).items():
#                 breed = self.name_to_breed.get(name)
#                 if not breed or not imgs:
#                     continue
#                 for bgr in imgs:
#                     emb = self._embed(bgr)
#                     if emb is not None:
#                         breed_vecs[breed].append(emb)

#         self.breed_centroids = {}
#         for breed, vecs in breed_vecs.items():
#             if not vecs:
#                 continue
#             mean_emb = np.mean(np.stack(vecs, axis=0), axis=0)
#             mean_emb /= (np.linalg.norm(mean_emb) + 1e-8)
#             self.breed_centroids[breed] = mean_emb.astype(np.float32)

#     def _embed(self, roi_bgr):
#         """ROI(BGR) -> 2048차원 L2-normalized 임베딩"""
#         if roi_bgr is None or roi_bgr.size == 0:
#             return None
#         rgb = cv2.cvtColor(roi_bgr, cv2.COLOR_BGR2RGB)
#         img_pil = Image.fromarray(rgb)
#         ten = self.transform(img_pil).unsqueeze(0).to(self.device)
#         with torch.no_grad():
#             feat = self.model(ten)  # [1, 2048]
#             emb = torch.nn.functional.normalize(feat, dim=1).squeeze(0).cpu().numpy()
#         return emb

#     def predict_breed(self, roi_bgr, topk=3):
#         """List[(breed, conf 0~1)] 반환. conf는 min-max 정규화 상대 점수."""
#         if not self.breed_centroids:
#             return []
#         q = self._embed(roi_bgr)
#         if q is None:
#             return []
#         scores = []
#         for breed, c in self.breed_centroids.items():
#             sim = float(np.dot(q, c))  # cosine
#             scores.append((breed, sim))
#         scores.sort(key=lambda x: x[1], reverse=True)

#         sims = np.array([s for _, s in scores], dtype=np.float32)
#         if len(sims) >= 2:
#             mn, mx = float(sims.min()), float(sims.max())
#             den = (mx - mn) + 1e-8
#             norm = [(b, (s - mn) / den) for (b, s) in scores[:max(1, topk)]]
#         else:
#             norm = [(scores[0][0], 1.0)]
#         return norm

#     def names_in_breed(self, breed):
#         return list(self.breed_to_names.get(breed, []))

# # =========================
# # 메인 루프
# # =========================
# def main(
#     input_dir="test_videos",
#     output_dir="result_videos",
#     pet_dir="pet_images",
#     pet_db_path="pet_db.json",
#     yolo_weights="yolov8n.pt",
#     yolo_conf=0.25,
#     device=None
# ):
#     os.makedirs(output_dir, exist_ok=True)

#     # pet DB
#     pet_db = {}
#     if os.path.isfile(pet_db_path):
#         try:
#             with open(pet_db_path, "r", encoding="utf-8") as f:
#                 pet_db = json.load(f)
#                 print(f"[INFO] pet_db 로드: {len(pet_db)}마리")
#         except Exception as e:
#             print("[WARN] pet_db.json 로드 실패:", e)

#     # 등록 이미지
#     registered_orb = load_registered_images(pet_dir, verbose=True)
#     print(f"[INFO] 등록된 강아지(ORB) 수: {len(registered_orb)}")

#     registered_color = load_registered_color_images(pet_dir, verbose=True)
#     print(f"[INFO] 등록된 강아지(컬러) 수: {len(registered_color)}")

#     # 품종 인덱서
#     breed_index = BreedIndexer(registered_color, pet_db, device=device or 'cpu')

#     # YOLO
#     model = YOLO(yolo_weights)
#     if device is not None:
#         try:
#             model.to(device)
#             print(f"[INFO] YOLO device: {device}")
#         except Exception as e:
#             print("[WARN] device 설정 실패(무시하고 자동 선택):", e)

#     videos = list_videos(input_dir)
#     if not videos:
#         print(f"[ERROR] 처리할 영상이 없습니다: {input_dir}")
#         return

#     for vpath in videos:
#         vname = os.path.basename(vpath)
#         out_path = os.path.join(output_dir, os.path.splitext(vname)[0] + "_result.mp4")
#         print(f"\n[PROCESS] {vname} -> {out_path}")

#         cap = cv2.VideoCapture(vpath)
#         if not cap.isOpened():
#             print(f"[ERROR] 열 수 없는 영상: {vpath}")
#             continue

#         width  = int(cap.get(cv2.CAP_PROP_FRAME_WIDTH))
#         height = int(cap.get(cv2.CAP_PROP_FRAME_HEIGHT))
#         fps    = cap.get(cv2.CAP_PROP_FPS) or 30.0
#         fourcc = cv2.VideoWriter_fourcc(*"mp4v")
#         writer = cv2.VideoWriter(out_path, fourcc, fps, (width, height))

#         t0 = time.time()
#         confirmed_ms    = None
#         confirmed_name  = None
#         confirmed_breed = None

#         # 버퍼
#         breed_confirm_buf = []
#         name_confirm_buf  = []
#         breed_history     = []

#         frame_idx = 0
#         while True:
#             ret, frame = cap.read()
#             if not ret:
#                 break
#             frame_idx += 1

#             results = model(frame, conf=yolo_conf, verbose=False)
#             boxes = results[0].boxes

#             base_text = "No dog detected"
#             base_color = (0, 0, 255)
#             drawn = False

#             best = None  # 가장 신뢰도 높은 박스
#             if boxes is not None and len(boxes) > 0:
#                 xyxy = boxes.xyxy.cpu().numpy()
#                 cls  = boxes.cls.cpu().numpy()
#                 conf = boxes.conf.cpu().numpy()

#                 for i in range(len(xyxy)):
#                     if int(cls[i]) != DOG_CLASS_ID:
#                         continue
#                     x1, y1, x2, y2 = xyxy[i]
#                     yolo_c = float(conf[i])
#                     x1, y1, x2, y2 = safe_rect(x1, y1, x2, y2, width, height)

#                     # 여백 추가(특징 확보)
#                     pad_x = int(0.12 * (x2 - x1))
#                     pad_y = int(0.12 * (y2 - y1))
#                     x1p = max(0, x1 - pad_x); y1p = max(0, y1 - pad_y)
#                     x2p = min(width-1, x2 + pad_x); y2p = min(height-1, y2 + pad_y)
#                     roi = frame[y1p:y2p, x1p:x2p]

#                     # 1) 품종 예측
#                     breed_scores = breed_index.predict_breed(roi, topk=3)
#                     top_breed, breed_conf = (breed_scores[0] if breed_scores else (None, 0.0))

#                     # 2) 이름 매칭(품종으로 후보 제한)
#                     allowed = None
#                     if top_breed and breed_conf >= BREED_MIN_CONF:
#                         allowed = set(breed_index.names_in_breed(top_breed))
#                     name, match_score, id_conf = robust_match(roi, registered_orb, allowed_names=allowed)

#                     size = pet_db.get(name, {}).get("size", "unknown") if name else "unknown"

#                     cand = {
#                         "name": name,
#                         "yolo_c": yolo_c,
#                         "id_conf": id_conf,
#                         "match_score": match_score,
#                         "rect": (x1, y1, x2, y2),
#                         "rect_pad": (x1p, y1p, x2p, y2p),
#                         "size": size,
#                         "breed": top_breed,
#                         "breed_conf": breed_conf
#                     }
#                     if best is None or yolo_c > best["yolo_c"]:
#                         best = cand

#             # 품종 히스토리 누적
#             if best and best["breed"]:
#                 breed_history.append(best["breed"])
#                 if len(breed_history) > BREED_HIST_MAX:
#                     breed_history.pop(0)

#             # 히스토리 기반 품종 확정(이름과 별개)
#             if confirmed_breed is None and len(breed_history) >= BREED_HIST_REQ:
#                 from collections import Counter
#                 cnt = Counter(breed_history)
#                 top_breed_hist, top_cnt = cnt.most_common(1)[0]
#                 if top_cnt >= max(BREED_HIST_REQ, int(BREED_HIST_RATIO * len(breed_history))):
#                     if best and best["breed"] == top_breed_hist and best["breed_conf"] >= BREED_MIN_CONF:
#                         confirmed_breed = top_breed_hist

#             # 원래 확정 로직
#             if best is not None and confirmed_ms is None:
#                 # (a) 품종 버퍼 기반 확정
#                 if best["breed"] and best["breed_conf"] >= BREED_MIN_CONF and best["yolo_c"] >= YOLO_MIN_CONF:
#                     breed_confirm_buf.append(best["breed"])
#                     if len(breed_confirm_buf) > CONFIRM_N:
#                         breed_confirm_buf.pop(0)
#                     if len(breed_confirm_buf) == CONFIRM_N and len(set(breed_confirm_buf)) == 1:
#                         confirmed_breed = breed_confirm_buf[-1]

#                 # (b) 이름 확정
#                 if best["name"] and best["id_conf"] >= ID_MIN_CONF and best["yolo_c"] >= YOLO_MIN_CONF:
#                     name_confirm_buf.append(best["name"])
#                     if len(name_confirm_buf) > CONFIRM_N:
#                         name_confirm_buf.pop(0)
#                     if len(name_confirm_buf) == CONFIRM_N and len(set(name_confirm_buf)) == 1:
#                         confirmed_name = name_confirm_buf[-1]
#                         confirmed_ms = int((time.time() - t0) * 1000)
#                         print(f"[SUMMARY] {vname} confirmed_at={confirmed_ms}ms "
#                               f"(breed={confirmed_breed or best['breed']}, name={confirmed_name}, frames={CONFIRM_N})")
#                 else:
#                     name_confirm_buf.clear()

#             # 품종만 확정되면 대표 이름으로 즉시 확정 (fallback)
#             if confirmed_breed is not None and confirmed_name is None and best is not None:
#                 fallback = pick_name_by_breed(confirmed_breed, breed_index, pet_db)
#                 if fallback:
#                     confirmed_name = fallback
#                     confirmed_ms = int((time.time() - t0) * 1000)
#                     print(f"[SUMMARY] {vname} breed_confirmed={confirmed_breed} -> name_fallback={confirmed_name}")

#             # 디버그 로그
#             if best is not None and (frame_idx % DEBUG_PRINT_EVERY == 0):
#                 print(f"[DBG] f={frame_idx} yolo={best['yolo_c']:.2f} "
#                       f"breed={best['breed']}({best['breed_conf']:.2f}) "
#                       f"name={best['name']} id={best['id_conf']}")

#             # 시각화
#             if best is not None:
#                 x1, y1, x2, y2 = best["rect"]
#                 if confirmed_ms is not None:
#                     color = (0, 255, 0)
#                     label = (f"확정: {confirmed_name} ({confirmed_breed or best['breed']}) | "
#                              f"ID:{best['id_conf']}% | YOLO:{int(best['yolo_c']*100)}% "
#                              f"| Breed:{int(best['breed_conf']*100)}% | Size:{best['size']}")
#                 else:
#                     color = (0, 165, 255)
#                     btop = f"{best['breed']}({int(best['breed_conf']*100)}%)" if best['breed'] else "None"
#                     label = (f"후보: {best['name'] or '없음'} | 품종: {btop} | "
#                              f"ID:{best['id_conf']}% | YOLO:{int(best['yolo_c']*100)}% | Size:{best['size']}")
#                 cv2.rectangle(frame, (x1, y1), (x2, y2), color, 2)
#                 frame = draw_text_korean(frame, label, (x1, max(20, y1-20)), font_size=20, color_bgr=color)
#                 drawn = True

#             if not drawn:
#                 frame = draw_text_korean(frame, base_text, (20, 40), font_size=22, color_bgr=base_color)

#             # FPS 표시
#             elapsed = max(1e-6, (time.time() - t0))
#             fps_est = frame_idx / elapsed
#             cv2.putText(frame, f"FPS: {fps_est:.1f}", (20, height - 20),
#                         cv2.FONT_HERSHEY_SIMPLEX, 0.6, (255, 255, 255), 2)

#             writer.write(frame)

#         cap.release()
#         writer.release()
#         print(f"[DONE] 저장 완료: {out_path}")

#         # 요약
#         if confirmed_ms is None and confirmed_breed is None:
#             print(f"[SUMMARY] {vname} confirmed_at=None (no stable confirmation)")
#         elif confirmed_ms is None and confirmed_breed is not None:
#             print(f"[SUMMARY] {vname} breed_confirmed={confirmed_breed} (name not confirmed)")

# if __name__ == "__main__":
#     ap = argparse.ArgumentParser()
#     ap.add_argument("--input_dir", default="test_videos")
#     ap.add_argument("--output_dir", default="result_videos")
#     ap.add_argument("--pet_dir", default="pet_images")
#     ap.add_argument("--pet_db", dest="pet_db_path", default="pet_db.json")
#     ap.add_argument("--weights", dest="yolo_weights", default="yolov8n.pt")
#     ap.add_argument("--conf", dest="yolo_conf", type=float, default=0.25)
#     ap.add_argument("--device", default=None, help="cuda:0 / mps / cpu 등 (선택)")
#     args = ap.parse_args()

#     main(
#         input_dir=args.input_dir,
#         output_dir=args.output_dir,
#         pet_dir=args.pet_dir,
#         pet_db_path=args.pet_db_path,
#         yolo_weights=args.yolo_weights,
#         yolo_conf=args.yolo_conf,
#         device=args.device
#     )

# test_detect_dog.py
import os
import json
import time
import argparse

import cv2
import numpy as np
import torch
import torchvision.transforms as T
from torchvision import models
from PIL import Image, ImageDraw, ImageFont
from ultralytics import YOLO

from test_embedding_utils import (
    load_registered_images,        # ORB descriptors
    load_registered_color_images,  # BGR images for breed embedding
    match_dog
)

# =========================
# 설정 값 (완화된 기본값)
# =========================
DOG_CLASS_ID      = 16   # COCO: dog
VIDEO_EXTS        = (".mp4", ".mov", ".avi", ".mkv")

YOLO_MIN_CONF     = 0.15  # YOLO 확신도 최소 (0~1)
ID_MIN_CONF       = 5     # ORB ID 신뢰도 최소 (0~100)
BREED_MIN_CONF    = 0.25  # 품종 임베딩 정규화 점수(0~1)
CONFIRM_N         = 2     # 같은 결과가 N프레임 연속이면 '확정'

DEBUG_PRINT_EVERY = 10    # N프레임마다 디버깅 로그 출력
BREED_HIST_MAX    = 12    # 품종 히스토리 최대 길이
BREED_HIST_REQ    = 3     # 최소 등장 횟수
BREED_HIST_RATIO  = 0.6   # 최빈 품종이 차지해야 하는 비율

STOP_ON_CONFIRM   = True  # 이름/품종 확정되면 즉시 저장 후 다음 영상으로
OVERLAY_FONT_SIZE = 20    # 한글 오버레이 글자 크기

# (선택) 품종 → 기본 대표 이름(정책)
DEFAULT_NAME_BY_BREED = {
    # "요크셔테리어": "삐삐",
    # "푸들": "쿠키",
    # "말티즈": "설기",
    # "리트리버": "벤지",
    # "스패니얼": "율무",
    # "사모예드": "모모",
}

# =========================
# 유틸
# =========================
def list_videos(input_dir):
    files = []
    if not os.path.isdir(input_dir):
        print(f"[ERROR] 입력 폴더가 없습니다: {input_dir}")
        return files
    for root, _, fnames in os.walk(input_dir):
        for fname in sorted(fnames):
            if fname.lower().endswith(VIDEO_EXTS):
                files.append(os.path.join(root, fname))
    return files

def safe_rect(x1, y1, x2, y2, w, h):
    x1 = max(0, min(int(x1), w-1))
    y1 = max(0, min(int(y1), h-1))
    x2 = max(0, min(int(x2), w-1))
    y2 = max(0, min(int(y2), h-1))
    if x2 <= x1: x2 = min(w-1, x1+1)
    if y2 <= y1: y2 = min(h-1, y1+1)
    return x1, y1, x2, y2

def robust_match(roi_bgr, registered, allowed_names=None):
    """
    match_dog이 (name, score) 또는 (name, score, id_conf) 둘 다 대응.
    allowed_names가 주어지면 그 이름들만 대상으로 제한.
    """
    subset = registered
    if allowed_names is not None:
        subset = {k: v for k, v in registered.items() if k in allowed_names}
        if not subset:
            subset = registered  # 품종에 해당 이름 폴더가 비면 전체로 후퇴

    name, score, id_conf = None, 0, 0
    try:
        out = match_dog(roi_bgr, subset)
        if isinstance(out, tuple):
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

def draw_text_korean(frame_bgr, text, org, font_size=OVERLAY_FONT_SIZE, color_bgr=(0,165,255), stroke=2):
    """cv2.putText가 한글을 못 찍는 문제 해결: PIL로 렌더링."""
    font_paths = [
        "/System/Library/Fonts/AppleSDGothicNeo.ttc",  # macOS
        "/Library/Fonts/AppleGothic.ttf",
        "/usr/share/fonts/truetype/nanum/NanumGothic.ttf",  # Linux
        "C:/Windows/Fonts/malgun.ttf",  # Windows
    ]
    font = None
    for p in font_paths:
        try:
            font = ImageFont.truetype(p, font_size)
            break
        except Exception:
            continue
    if font is None:
        # 폰트가 없으면 영어만이라도 표기
        cv2.putText(frame_bgr, text, org, cv2.FONT_HERSHEY_SIMPLEX, 0.6, color_bgr, 2, cv2.LINE_AA)
        return frame_bgr

    img_pil = Image.fromarray(cv2.cvtColor(frame_bgr, cv2.COLOR_BGR2RGB))
    draw = ImageDraw.Draw(img_pil)
    x, y = org
    # 외곽선
    if stroke and stroke > 0:
        for dx in (-stroke, 0, stroke):
            for dy in (-stroke, 0, stroke):
                if dx == 0 and dy == 0:
                    continue
                draw.text((x+dx, y+dy), text, font=font, fill=(0,0,0))
    # 본문
    b,g,r = color_bgr
    draw.text((x, y), text, font=font, fill=(r,g,b))
    return cv2.cvtColor(np.array(img_pil), cv2.COLOR_RGB2BGR)

def draw_petdb_overlay(frame, name, breed, pet_db, org=(20, 20), line_gap=24, color_bgr=(0,255,0)):
    """확정 후 pet_db의 주요 정보를 화면에 여러 줄로 오버레이."""
    info = pet_db.get(name, {})
    fields = [
        f"이름: {name}",
        f"품종: {breed}",
        f"체구: {info.get('size', 'unknown')}",
        f"몸무게: {info.get('weight', '?')}kg",
        f"나이: {info.get('age', '?')}세",
        f"활동계수: {info.get('activeLvl', '?')}",
        f"칼로리/일(kg당): {info.get('calPerKg', '?')}",
        f"급여 횟수: {info.get('feedingCount', '?')}회",
    ]
    x, y = org
    for i, line in enumerate(fields):
        frame = draw_text_korean(frame, line, (x, y + i*line_gap), font_size=OVERLAY_FONT_SIZE, color_bgr=color_bgr)
    return frame

def pick_name_by_breed(breed, breed_index, pet_db):
    """품종만 확정된 경우 대표 이름을 정책적으로 선택."""
    # 1) 정책 맵 우선
    if breed in DEFAULT_NAME_BY_BREED and DEFAULT_NAME_BY_BREED[breed] in pet_db:
        return DEFAULT_NAME_BY_BREED[breed]
    # 2) 해당 품종에 등록된 이름들
    cand = breed_index.names_in_breed(breed)
    if not cand:
        return None
    if len(cand) == 1:
        return cand[0]
    return sorted(cand)[0]  # 사전순

# 확정 시 전송/로깅 훅 (원하면 HTTP/MQTT로 교체)
def on_confirm(breed, name, pet_db, confirmed_ms):
    payload = pet_db.get(name) or {}
    print("[SEND]", {"breed": breed, "name": name, "ms": confirmed_ms, **payload})
    return payload  # [RESULT]에 사용

# =========================
# 품종 인덱서
# =========================
class BreedIndexer:
    """
    - 등록 컬러 이미지로 품종별 임베딩(ResNet50 feature) 센트로이드 계산
    - ROI에서 임베딩 추출 후 코사인 유사도로 품종 예측
    - 한글 breed 라벨도 그대로 사용 가능 (문자열 식별자)
    """
    def __init__(self, color_images_by_name, pet_db, device='cpu'):
        self.device = torch.device(device if device else ('cuda' if torch.cuda.is_available() else 'cpu'))

        # torchvision 버전 호환
        ResNet50_Weights = getattr(models, "ResNet50_Weights", None)
        if ResNet50_Weights is not None:
            weights = ResNet50_Weights.DEFAULT  # 또는 IMAGENET1K_V2
            self.model = models.resnet50(weights=weights)
            self.model.fc = torch.nn.Identity()
            self.model.eval().to(self.device)
            self.transform = weights.transforms()  # 권장 전처리
        else:
            self.model = models.resnet50(pretrained=True)
            self.model.fc = torch.nn.Identity()
            self.model.eval().to(self.device)
            self.transform = T.Compose([
                T.ToPILImage(),
                T.Resize(256),
                T.CenterCrop(224),
                T.ToTensor(),
                T.Normalize(mean=[0.485, 0.456, 0.406],
                            std=[0.229, 0.224, 0.225]),
            ])

        # 이름 ↔ 품종 매핑
        self.name_to_breed = {}
        self.breed_to_names = {}
        for name, meta in (pet_db or {}).items():
            breed = (meta or {}).get('breed')
            if not breed:
                continue
            self.name_to_breed[name] = breed
            self.breed_to_names.setdefault(breed, []).append(name)

        # 품종별 센트로이드
        breed_vecs = {breed: [] for breed in self.breed_to_names.keys()}

        with torch.no_grad():
            for name, imgs in (color_images_by_name or {}).items():
                breed = self.name_to_breed.get(name)
                if not breed or not imgs:
                    continue
                for bgr in imgs:
                    emb = self._embed(bgr)
                    if emb is not None:
                        breed_vecs[breed].append(emb)

        self.breed_centroids = {}
        for breed, vecs in breed_vecs.items():
            if not vecs:
                continue
            mean_emb = np.mean(np.stack(vecs, axis=0), axis=0)
            mean_emb /= (np.linalg.norm(mean_emb) + 1e-8)
            self.breed_centroids[breed] = mean_emb.astype(np.float32)

    def _embed(self, roi_bgr):
        """ROI(BGR) -> 2048차원 L2-normalized 임베딩"""
        if roi_bgr is None or roi_bgr.size == 0:
            return None
        rgb = cv2.cvtColor(roi_bgr, cv2.COLOR_BGR2RGB)
        img_pil = Image.fromarray(rgb)
        ten = self.transform(img_pil).unsqueeze(0).to(self.device)
        with torch.no_grad():
            feat = self.model(ten)  # [1, 2048]
            emb = torch.nn.functional.normalize(feat, dim=1).squeeze(0).cpu().numpy()
        return emb

    def predict_breed(self, roi_bgr, topk=3):
        """List[(breed, conf 0~1)] 반환. conf는 min-max 정규화 상대 점수."""
        if not self.breed_centroids:
            return []
        q = self._embed(roi_bgr)
        if q is None:
            return []
        scores = []
        for breed, c in self.breed_centroids.items():
            sim = float(np.dot(q, c))  # cosine
            scores.append((breed, sim))
        scores.sort(key=lambda x: x[1], reverse=True)

        sims = np.array([s for _, s in scores], dtype=np.float32)
        if len(sims) >= 2:
            mn, mx = float(sims.min()), float(sims.max())
            den = (mx - mn) + 1e-8
            norm = [(b, (s - mn) / den) for (b, s) in scores[:max(1, topk)]]
        else:
            norm = [(scores[0][0], 1.0)]
        return norm

    def names_in_breed(self, breed):
        return list(self.breed_to_names.get(breed, []))

# =========================
# 메인 루프
# =========================
def main(
    input_dir="test_videos",
    output_dir="result_videos",
    pet_dir="pet_images",
    pet_db_path="pet_db.json",
    yolo_weights="yolov8n.pt",
    yolo_conf=0.25,
    device=None
):
    os.makedirs(output_dir, exist_ok=True)

    # pet DB
    pet_db = {}
    if os.path.isfile(pet_db_path):
        try:
            with open(pet_db_path, "r", encoding="utf-8") as f:
                pet_db = json.load(f)
                print(f"[INFO] pet_db 로드: {len(pet_db)}마리")
        except Exception as e:
            print("[WARN] pet_db.json 로드 실패:", e)

    # 등록 이미지
    registered_orb = load_registered_images(pet_dir, verbose=True)
    print(f"[INFO] 등록된 강아지(ORB) 수: {len(registered_orb)}")

    registered_color = load_registered_color_images(pet_dir, verbose=True)
    print(f"[INFO] 등록된 강아지(컬러) 수: {len(registered_color)}")

    # 품종 인덱서
    breed_index = BreedIndexer(registered_color, pet_db, device=device or 'cpu')

    # YOLO
    model = YOLO(yolo_weights)
    if device is not None:
        try:
            model.to(device)
            print(f"[INFO] YOLO device: {device}")
        except Exception as e:
            print("[WARN] device 설정 실패(무시하고 자동 선택):", e)

    videos = list_videos(input_dir)
    if not videos:
        print(f"[ERROR] 처리할 영상이 없습니다: {input_dir}")
        return

    for vpath in videos:
        vname = os.path.basename(vpath)
        out_path = os.path.join(output_dir, os.path.splitext(vname)[0] + "_result.mp4")
        print(f"\n[PROCESS] {vname} -> {out_path}")

        cap = cv2.VideoCapture(vpath)
        if not cap.isOpened():
            print(f"[ERROR] 열 수 없는 영상: {vpath}")
            continue

        width  = int(cap.get(cv2.CAP_PROP_FRAME_WIDTH))
        height = int(cap.get(cv2.CAP_PROP_FRAME_HEIGHT))
        fps    = cap.get(cv2.CAP_PROP_FPS) or 30.0
        fourcc = cv2.VideoWriter_fourcc(*"mp4v")
        writer = cv2.VideoWriter(out_path, fourcc, fps, (width, height))

        t0 = time.time()
        confirmed_ms    = None
        confirmed_name  = None
        confirmed_breed = None

        # 버퍼 & 히스토리
        breed_confirm_buf = []
        name_confirm_buf  = []
        breed_history     = []

        # 결과 보관
        final_result = {
            "video": vname,
            "name": None,
            "breed": None,
            "ms": None,
            "payload": {}
        }

        frame_idx = 0
        while True:
            ret, frame = cap.read()
            if not ret:
                break
            frame_idx += 1

            results = model(frame, conf=yolo_conf, verbose=False)
            boxes = results[0].boxes

            base_text = "No dog detected"
            base_color = (0, 0, 255)
            drawn = False

            best = None  # 가장 신뢰도 높은 박스
            if boxes is not None and len(boxes) > 0:
                xyxy = boxes.xyxy.cpu().numpy()
                cls  = boxes.cls.cpu().numpy()
                conf = boxes.conf.cpu().numpy()

                for i in range(len(xyxy)):
                    if int(cls[i]) != DOG_CLASS_ID:
                        continue
                    x1, y1, x2, y2 = xyxy[i]
                    yolo_c = float(conf[i])
                    x1, y1, x2, y2 = safe_rect(x1, y1, x2, y2, width, height)

                    # 여백 추가(특징 확보)
                    pad_x = int(0.12 * (x2 - x1))
                    pad_y = int(0.12 * (y2 - y1))
                    x1p = max(0, x1 - pad_x); y1p = max(0, y1 - pad_y)
                    x2p = min(width-1, x2 + pad_x); y2p = min(height-1, y2 + pad_y)
                    roi = frame[y1p:y2p, x1p:x2p]

                    # 1) 품종 예측
                    breed_scores = breed_index.predict_breed(roi, topk=3)
                    top_breed, breed_conf = (breed_scores[0] if breed_scores else (None, 0.0))

                    # 2) 이름 매칭(품종으로 후보 제한)
                    allowed = None
                    if top_breed and breed_conf >= BREED_MIN_CONF:
                        allowed = set(breed_index.names_in_breed(top_breed))
                    name, match_score, id_conf = robust_match(roi, registered_orb, allowed_names=allowed)

                    size = pet_db.get(name, {}).get("size", "unknown") if name else "unknown"

                    cand = {
                        "name": name,
                        "yolo_c": yolo_c,
                        "id_conf": id_conf,
                        "match_score": match_score,
                        "rect": (x1, y1, x2, y2),
                        "rect_pad": (x1p, y1p, x2p, y2p),
                        "size": size,
                        "breed": top_breed,
                        "breed_conf": breed_conf
                    }
                    if best is None or yolo_c > best["yolo_c"]:
                        best = cand

            # 품종 히스토리 누적
            if best and best["breed"]:
                breed_history.append(best["breed"])
                if len(breed_history) > BREED_HIST_MAX:
                    breed_history.pop(0)

            # 히스토리 기반 품종 확정(이름과 별개)
            if confirmed_breed is None and len(breed_history) >= BREED_HIST_REQ:
                from collections import Counter
                cnt = Counter(breed_history)
                top_breed_hist, top_cnt = cnt.most_common(1)[0]
                if top_cnt >= max(BREED_HIST_REQ, int(BREED_HIST_RATIO * len(breed_history))):
                    if best and best["breed"] == top_breed_hist and best["breed_conf"] >= BREED_MIN_CONF:
                        confirmed_breed = top_breed_hist

            # 원래 확정 로직
            if best is not None and confirmed_ms is None:
                # (a) 품종 버퍼 기반 확정
                if best["breed"] and best["breed_conf"] >= BREED_MIN_CONF and best["yolo_c"] >= YOLO_MIN_CONF:
                    breed_confirm_buf.append(best["breed"])
                    if len(breed_confirm_buf) > CONFIRM_N:
                        breed_confirm_buf.pop(0)
                    if len(breed_confirm_buf) == CONFIRM_N and len(set(breed_confirm_buf)) == 1:
                        confirmed_breed = breed_confirm_buf[-1]

                # (b) 이름 확정
                if best["name"] and best["id_conf"] >= ID_MIN_CONF and best["yolo_c"] >= YOLO_MIN_CONF:
                    name_confirm_buf.append(best["name"])
                    if len(name_confirm_buf) > CONFIRM_N:
                        name_confirm_buf.pop(0)
                    if len(name_confirm_buf) == CONFIRM_N and len(set(name_confirm_buf)) == 1:
                        confirmed_name = name_confirm_buf[-1]
                        confirmed_ms = int((time.time() - t0) * 1000)
                        payload = on_confirm(confirmed_breed or best['breed'], confirmed_name, pet_db, confirmed_ms)
                        final_result.update({
                            "name": confirmed_name,
                            "breed": confirmed_breed or best['breed'],
                            "ms": confirmed_ms,
                            "payload": payload
                        })
                        print(f"[SUMMARY] {vname} confirmed_at={confirmed_ms}ms "
                              f"(breed={final_result['breed']}, name={confirmed_name}, frames={CONFIRM_N})")
                        if STOP_ON_CONFIRM:
                            # 마지막 프레임에 pet_db 정보 오버레이 후 저장하고 탈출
                            x1, y1, x2, y2 = best["rect"]
                            cv2.rectangle(frame, (x1, y1), (x2, y2), (0,255,0), 2)
                            frame = draw_petdb_overlay(frame, confirmed_name, final_result['breed'], pet_db, org=(20, 60))
                            writer.write(frame)
                            break
                else:
                    name_confirm_buf.clear()

            # 품종만 확정되면 대표 이름으로 즉시 확정 (fallback)
            if confirmed_breed is not None and confirmed_name is None and best is not None:
                fallback = pick_name_by_breed(confirmed_breed, breed_index, pet_db)
                if fallback:
                    confirmed_name = fallback
                    confirmed_ms = int((time.time() - t0) * 1000)
                    payload = on_confirm(confirmed_breed, confirmed_name, pet_db, confirmed_ms)
                    final_result.update({
                        "name": confirmed_name,
                        "breed": confirmed_breed,
                        "ms": confirmed_ms,
                        "payload": payload
                    })
                    print(f"[SUMMARY] {vname} breed_confirmed={confirmed_breed} -> name_fallback={confirmed_name}")
                    if STOP_ON_CONFIRM:
                        x1, y1, x2, y2 = best["rect"]
                        cv2.rectangle(frame, (x1, y1), (x2, y2), (0,255,0), 2)
                        frame = draw_petdb_overlay(frame, confirmed_name, confirmed_breed, pet_db, org=(20, 60))
                        writer.write(frame)
                        break

            # 디버그 로그
            if best is not None and (frame_idx % DEBUG_PRINT_EVERY == 0):
                print(f"[DBG] f={frame_idx} yolo={best['yolo_c']:.2f} "
                      f"breed={best['breed']}({best['breed_conf']:.2f}) "
                      f"name={best['name']} id={best['id_conf']}")

            # 시각화
            if best is not None:
                x1, y1, x2, y2 = best["rect"]
                if confirmed_ms is not None:
                    color = (0, 255, 0)
                    label = (f"확정: {confirmed_name} ({confirmed_breed or best['breed']}) | "
                             f"ID:{best['id_conf']}% | YOLO:{int(best['yolo_c']*100)}% "
                             f"| Breed:{int(best['breed_conf']*100)}% | Size:{best['size']}")
                    cv2.rectangle(frame, (x1, y1), (x2, y2), color, 2)
                    frame = draw_text_korean(frame, label, (x1, max(20, y1-20)), font_size=20, color_bgr=color)
                    # 확정된 정보 상세 오버레이 (박스 아래쪽이 좁으면 좌측 상단 등으로 조정)
                    frame = draw_petdb_overlay(frame, confirmed_name, confirmed_breed or best['breed'], pet_db, org=(20, 60))
                else:
                    color = (0, 165, 255)
                    btop = f"{best['breed']}({int(best['breed_conf']*100)}%)" if best['breed'] else "None"
                    label = (f"후보: {best['name'] or '없음'} | 품종: {btop} | "
                             f"ID:{best['id_conf']}% | YOLO:{int(best['yolo_c']*100)}% | Size:{best['size']}")
                    cv2.rectangle(frame, (x1, y1), (x2, y2), color, 2)
                    frame = draw_text_korean(frame, label, (x1, max(20, y1-20)), font_size=20, color_bgr=color)
                drawn = True

            if not drawn:
                frame = draw_text_korean(frame, base_text, (20, 40), font_size=22, color_bgr=base_color)

            # FPS 표시
            elapsed = max(1e-6, (time.time() - t0))
            fps_est = frame_idx / elapsed
            cv2.putText(frame, f"FPS: {fps_est:.1f}", (20, height - 20),
                        cv2.FONT_HERSHEY_SIMPLEX, 0.6, (255, 255, 255), 2)

            writer.write(frame)

        cap.release()
        writer.release()
        print(f"[DONE] 저장 완료: {out_path}")

        # [RESULT] 요약
        if final_result["name"] is None and confirmed_breed is None:
            print(f"[SUMMARY] {vname} confirmed_at=None (no stable confirmation)")
        else:
            print("\n[RESULT]")
            print(f"{vname} 결과")
            print(f"- 확정 시간: {final_result.get('ms') if final_result.get('ms') is not None else 'N/A'} ms")
            print(f"- 이름: {final_result.get('name') or 'N/A'}")
            print(f"- 품종: {final_result.get('breed') or (confirmed_breed or 'N/A')}")
            payload = final_result.get("payload", {})
            for k in ["size", "weight", "age", "activeLvl", "calPerKg", "feedingCount"]:
                if k in payload:
                    print(f"- {k}: {payload[k]}")
            print("")  # 빈 줄

if __name__ == "__main__":
    ap = argparse.ArgumentParser()
    ap.add_argument("--input_dir", default="test_videos")
    ap.add_argument("--output_dir", default="result_videos")
    ap.add_argument("--pet_dir", default="pet_images")
    ap.add_argument("--pet_db", dest="pet_db_path", default="pet_db.json")
    ap.add_argument("--weights", dest="yolo_weights", default="yolov8n.pt")
    ap.add_argument("--conf", dest="yolo_conf", type=float, default=0.25)
    ap.add_argument("--device", default=None, help="cuda:0 / mps / cpu 등 (선택)")
    args = ap.parse_args()

    main(
        input_dir=args.input_dir,
        output_dir=args.output_dir,
        pet_dir=args.pet_dir,
        pet_db_path=args.pet_db_path,
        yolo_weights=args.yolo_weights,
        yolo_conf=args.yolo_conf,
        device=args.device
    )
