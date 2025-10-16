#시연용

# test_detect_dog_2.py
import os
import json
import time
import glob
import argparse

import cv2
import numpy as np
import torch
import torchvision.transforms as T
from torchvision import models
from PIL import Image, ImageDraw, ImageFont
from ultralytics import YOLO

# --- 시리얼 (선택) ---
try:
    import serial
except Exception:
    serial = None

# -------------------------
# 설정
# -------------------------
DOG_CLASS_ID      = 16   # COCO: dog
VIDEO_EXTS        = (".mp4", ".mov", ".avi", ".mkv")

YOLO_MIN_CONF     = 0.15
ID_MIN_CONF       = 5
BREED_MIN_CONF    = 0.25
CONFIRM_N         = 2

DEBUG_PRINT_EVERY = 10
BREED_HIST_MAX    = 12
BREED_HIST_REQ    = 3
BREED_HIST_RATIO  = 0.6

OVERLAY_FONT_SIZE = 20

# 시리얼 아두이노 포트/속도 (환경에 맞게 변경)
SERIAL_PORT = "/dev/cu.usbmodem11201" # 예: macOS /dev/tty.usbmodem*, Windows COM3, Linux /dev/ttyACM0
SERIAL_BAUD = 115200
SERIAL_TIMEOUT = 2.0

# (선택) 품종 → 대표 이름 매핑 (품종만 확정되면 이 이름으로 전송)
DEFAULT_NAME_BY_BREED = {
    # "요크셔테리어": "삐삐",
    # "푸들": "쿠키",
    # "말티즈": "설기",
    # "리트리버": "벤지",
    # "스패니얼": "율무",
    # "사모예드": "모모",
}

# -------------------------
# 유틸
# -------------------------
def list_one_video(input_dir):
    if not os.path.isdir(input_dir):
        print(f"[ERROR] 입력 폴더가 없습니다: {input_dir}")
        return None
    for ext in VIDEO_EXTS:
        cand = sorted(glob.glob(os.path.join(input_dir, f"*{ext}")))
        if cand:
            return cand[0]
    return None

def safe_rect(x1, y1, x2, y2, w, h):
    x1 = max(0, min(int(x1), w-1))
    y1 = max(0, min(int(y1), h-1))
    x2 = max(0, min(int(x2), w-1))
    y2 = max(0, min(int(y2), h-1))
    if x2 <= x1: x2 = min(w-1, x1+1)
    if y2 <= y1: y2 = min(h-1, y1+1)
    return x1, y1, x2, y2

# ---------- ORB 매칭 ----------
import cv2 as _cv2
orb = _cv2.ORB_create(
    nfeatures=800, scaleFactor=1.2, nlevels=8,
    edgeThreshold=15, patchSize=31, fastThreshold=7
)
bf = _cv2.BFMatcher(_cv2.NORM_HAMMING, crossCheck=False)
VALID_EXTS = (".jpg", ".jpeg", ".png", ".bmp", ".webp")

def _read_gray(path):
    return _cv2.imread(path, _cv2.IMREAD_GRAYSCALE)

def load_registered_images(folder="pet_images", verbose=True):
    reg = {}
    if not os.path.isdir(folder):
        if verbose: print(f"[WARN] 등록 폴더가 없습니다: {folder}")
        return reg

    for dog_name in os.listdir(folder):
        dpath = os.path.join(folder, dog_name)
        if not os.path.isdir(dpath): continue
        feats = []
        for fn in os.listdir(dpath):
            if not fn.lower().endswith(VALID_EXTS): continue
            img = _read_gray(os.path.join(dpath, fn))
            if img is None: continue
            img = _cv2.GaussianBlur(img, (3,3), 0)
            clahe = _cv2.createCLAHE(clipLimit=2.0, tileGridSize=(8,8))
            img = clahe.apply(img)
            kp, des = orb.detectAndCompute(img, None)
            if des is not None and len(des) > 0:
                feats.append(des)
        if feats:
            reg[dog_name] = feats
            if verbose: print(f"[INFO] ORB {dog_name}: {len(feats)} images")
        else:
            if verbose: print(f"[WARN] {dog_name}: 사용 가능한 이미지가 없습니다.")
    return reg

def load_registered_color_images(folder="pet_images", verbose=True, max_per_name=20, min_side=224):
    reg = {}
    if not os.path.isdir(folder):
        if verbose: print(f"[WARN] 등록 폴더가 없습니다: {folder}")
        return reg

    for dog_name in os.listdir(folder):
        dpath = os.path.join(folder, dog_name)
        if not os.path.isdir(dpath): continue
        imgs = []
        for fn in sorted(os.listdir(dpath)):
            if not fn.lower().endswith(VALID_EXTS): continue
            img = _cv2.imread(os.path.join(dpath, fn), _cv2.IMREAD_COLOR)
            if img is None: continue
            h, w = img.shape[:2]
            if min(h, w) < min_side:
                scale = float(min_side) / max(1, min(h, w))
                img = _cv2.resize(img, (int(w*scale), int(h*scale)))
            imgs.append(img)
            if len(imgs) >= max_per_name: break
        if imgs:
            reg[dog_name] = imgs
            if verbose: print(f"[INFO] COLOR {dog_name}: {len(imgs)} images")
    return reg

def _good_matches(des1, des2, ratio=0.75, max_per=200):
    if des1 is None or des2 is None or len(des1)==0 or len(des2)==0:
        return 0
    matches = bf.knnMatch(des1, des2, k=2)
    good = 0
    for m_n in matches:
        if len(m_n) < 2: continue
        m, n = m_n
        if m.distance < ratio * n.distance:
            good += 1
            if good >= max_per: break
    return good

def match_dog(roi_bgr, registered, min_roi=48):
    if roi_bgr is None or roi_bgr.size == 0:
        return None, 0, 0
    h, w = roi_bgr.shape[:2]
    if min(h, w) < min_roi:
        return None, 0, 0
    gray = _cv2.cvtColor(roi_bgr, _cv2.COLOR_BGR2GRAY)
    gray = _cv2.GaussianBlur(gray, (3,3), 0)
    clahe = _cv2.createCLAHE(clipLimit=2.0, tileGridSize=(8,8))
    gray = clahe.apply(gray)
    kp_roi, des_roi = orb.detectAndCompute(gray, None)
    if des_roi is None or len(des_roi) < 12:
        return None, 0, 0
    best_name, best_score = None, 0
    for name, feat_list in registered.items():
        local_best = 0
        for des_reg in feat_list:
            g = _good_matches(des_roi, des_reg, ratio=0.75)
            if g > local_best: local_best = g
        if local_best > best_score:
            best_score = local_best
            best_name = name
    denom = max(40, len(des_roi))
    conf = int(np.clip(100.0 * (best_score / denom), 0, 100))
    if conf < 20:
        return None, best_score, conf
    return best_name, int(best_score), int(conf)

def robust_match(roi_bgr, registered, allowed_names=None):
    subset = registered
    if allowed_names is not None:
        subset = {k: v for k, v in registered.items() if k in allowed_names}
        if not subset:
            subset = registered
    try:
        out = match_dog(roi_bgr, subset)
        if isinstance(out, tuple):
            if len(out) == 3: return out[0], int(out[1]), int(out[2])
            if len(out) == 2: return out[0], int(out[1]), int(out[1])
            return out[0] if len(out)>0 else None, int(out[1]) if len(out)>1 else 0, int(out[1]) if len(out)>1 else 0
    except Exception:
        pass
    return None, 0, 0

# ---------- 한글 라벨 ----------
def draw_text_korean(frame_bgr, text, org, font_size=OVERLAY_FONT_SIZE, color_bgr=(0,165,255), stroke=2):
    from PIL import Image, ImageDraw, ImageFont
    font_paths = [
        "/System/Library/Fonts/AppleSDGothicNeo.ttc",
        "/Library/Fonts/AppleGothic.ttf",
        "/usr/share/fonts/truetype/nanum/NanumGothic.ttf",
        "C:/Windows/Fonts/malgun.ttf",
    ]
    font = None
    for p in font_paths:
        try:
            font = ImageFont.truetype(p, font_size)
            break
        except Exception:
            continue
    if font is None:
        cv2.putText(frame_bgr, text, org, cv2.FONT_HERSHEY_SIMPLEX, 0.6, color_bgr, 2, cv2.LINE_AA)
        return frame_bgr
    img_pil = Image.fromarray(cv2.cvtColor(frame_bgr, cv2.COLOR_BGR2RGB))
    draw = ImageDraw.Draw(img_pil)
    x, y = org
    if stroke and stroke > 0:
        for dx in (-stroke, 0, stroke):
            for dy in (-stroke, 0, stroke):
                if dx == 0 and dy == 0: continue
                draw.text((x+dx, y+dy), text, font=font, fill=(0,0,0))
    b,g,r = color_bgr
    draw.text((x, y), text, font=font, fill=(r,g,b))
    return cv2.cvtColor(np.array(img_pil), cv2.COLOR_RGB2BGR)

def draw_petdb_overlay(frame, name, breed, pet_db, org=(20, 60), line_gap=24, color_bgr=(0,255,0)):
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

# ---------- 품종 인덱서 ----------
class BreedIndexer:
    def __init__(self, color_images_by_name, pet_db, device='cpu'):
        self.device = torch.device(device if device else ('cuda' if torch.cuda.is_available() else 'cpu'))
        ResNet50_Weights = getattr(models, "ResNet50_Weights", None)
        if ResNet50_Weights is not None:
            weights = ResNet50_Weights.DEFAULT
            self.model = models.resnet50(weights=weights)
            self.model.fc = torch.nn.Identity()
            self.model.eval().to(self.device)
            self.transform = weights.transforms()
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
        self.name_to_breed = {}
        self.breed_to_names = {}
        for name, meta in (pet_db or {}).items():
            breed = (meta or {}).get('breed')
            if not breed: continue
            self.name_to_breed[name] = breed
            self.breed_to_names.setdefault(breed, []).append(name)

        breed_vecs = {breed: [] for breed in self.breed_to_names.keys()}
        with torch.no_grad():
            for name, imgs in (color_images_by_name or {}).items():
                breed = self.name_to_breed.get(name)
                if not breed or not imgs: continue
                for bgr in imgs:
                    emb = self._embed(bgr)
                    if emb is not None:
                        breed_vecs[breed].append(emb)
        self.breed_centroids = {}
        for breed, vecs in breed_vecs.items():
            if not vecs: continue
            mean_emb = np.mean(np.stack(vecs, axis=0), axis=0)
            mean_emb /= (np.linalg.norm(mean_emb) + 1e-8)
            self.breed_centroids[breed] = mean_emb.astype(np.float32)

    def _embed(self, roi_bgr):
        if roi_bgr is None or roi_bgr.size == 0: return None
        rgb = cv2.cvtColor(roi_bgr, cv2.COLOR_BGR2RGB)
        img_pil = Image.fromarray(rgb)
        ten = self.transform(img_pil).unsqueeze(0).to(self.device)
        with torch.no_grad():
            feat = self.model(ten)
            emb = torch.nn.functional.normalize(feat, dim=1).squeeze(0).cpu().numpy()
        return emb

    def predict_breed(self, roi_bgr, topk=3):
        if not self.breed_centroids: return []
        q = self._embed(roi_bgr)
        if q is None: return []
        scores = []
        for breed, c in self.breed_centroids.items():
            sim = float(np.dot(q, c))
            scores.append((breed, sim))
        scores.sort(key=lambda x: x[1], reverse=True)
        sims = np.array([s for _, s in scores], dtype=np.float32)
        if len(sims) >= 2:
            mn, mx = float(sims.min()), float(sims.max())
            den = (mx - mn) + 1e-8
            return [(b, (s - mn) / den) for (b, s) in scores[:max(1, topk)]]
        else:
            return [(scores[0][0], 1.0)]

    def names_in_breed(self, breed):
        return list(self.breed_to_names.get(breed, []))

def pick_name_by_breed(breed, breed_index, pet_db):
    if breed in DEFAULT_NAME_BY_BREED and DEFAULT_NAME_BY_BREED[breed] in pet_db:
        return DEFAULT_NAME_BY_BREED[breed]
    cand = breed_index.names_in_breed(breed)
    if not cand: return None
    if len(cand) == 1: return cand[0]
    return sorted(cand)[0]

# ---------- 시리얼 전송 ----------
def open_serial():
    if serial is None:
        print("[SERIAL] pyserial 미설치 → 콘솔 출력만 사용")
        return None
    try:
        ser = serial.Serial(SERIAL_PORT, SERIAL_BAUD, timeout=SERIAL_TIMEOUT)
        time.sleep(1.0)
        print(f"[SERIAL] opened {SERIAL_PORT} @ {SERIAL_BAUD}")
        return ser
    except Exception as e:
        print("[SERIAL][ERR]", e)
        return None

def send_to_arduino(ser, name, score_pct, size, weight, activeLvl, calPerKg, feedingCount):
    # 아두이노 스케치가 기대하는 CSV 형식:
    # (이름,점수,체급,체중,활동수준,칼로리,급여횟수)\n
    line = f"{name},{score_pct:.1f},{size},{float(weight):.3f},{float(activeLvl):.3f},{float(calPerKg):.3f},{int(feedingCount)}\n"
    if ser:
        try:
            ser.write(line.encode("utf-8"))
            ser.flush()
            print("[SERIAL][SEND]", line.strip())
        except Exception as e:
            print("[SERIAL][ERR]", e, "-> 콘솔 출력 fallback:", line.strip())
    else:
        print("[SERIAL][FAKE SEND]", line.strip())


def read_from_arduino(ser):
    """아두이노로부터 들어오는 시리얼 데이터를 읽어 터미널에 출력합니다."""
    if ser and ser.in_waiting > 0:
        try:
            # 아두이노가 보낸 한 줄을 읽고, utf-8로 디코딩합니다.
            line = ser.readline().decode('utf-8', errors='ignore').strip()
            if line:
                print(f"[Arduino] {line}")
        except Exception as e:
            print(f"[SERIAL][READ_ERR] {e}")

# -------------------------
# 메인 (시연용: 단일 영상, 저장 X)
# -------------------------

def main(
    input_dir="test_video_2",
    pet_dir="pet_images",
    pet_db_path="pet_db.json",
    yolo_weights="yolov8n.pt",
    yolo_conf=0.25,
    device=None,
    show_window=True
):
    # DB/이미지
    pet_db = {}
    if os.path.isfile(pet_db_path):
        with open(pet_db_path, "r", encoding="utf-8") as f:
            pet_db = json.load(f)
            print(f"[INFO] pet_db 로드: {len(pet_db)}마리")

    registered_orb = load_registered_images(pet_dir, verbose=True)
    registered_color = load_registered_color_images(pet_dir, verbose=True)
    breed_index = BreedIndexer(registered_color, pet_db, device=device or 'cpu')

    # YOLO
    model = YOLO(yolo_weights)
    if device is not None:
        try:
            model.to(device)
            print(f"[INFO] YOLO device: {device}")
        except Exception as e:
            print("[WARN] device 설정 실패(자동 선택):", e)

    # 단일 영상
    vpath = list_one_video(input_dir)
    if not vpath:
        print(f"[ERROR] 처리할 영상이 없습니다: {input_dir}")
        return
    vname = os.path.basename(vpath)
    print(f"\n[PROCESS] {vname}")

    # 시리얼
    ser = open_serial()

    cap = cv2.VideoCapture(vpath)
    if not cap.isOpened():
        print(f"[ERROR] 열 수 없는 영상: {vpath}")
        return

    width  = int(cap.get(cv2.CAP_PROP_FRAME_WIDTH))
    height = int(cap.get(cv2.CAP_PROP_FRAME_HEIGHT))
    fps    = cap.get(cv2.CAP_PROP_FPS) or 30.0

    t0 = time.time()
    confirmed_ms    = None
    confirmed_name  = None
    confirmed_breed = None

    breed_confirm_buf = []
    name_confirm_buf  = []
    breed_history     = []

    frame_idx = 0
    best = None # [수정] best 변수를 루프 밖에서 초기화

    while True:
        ret, frame = cap.read()
        if not ret:
            break
        frame_idx += 1

        # [수정] 루프 시작 시 best 초기화
        best_this_frame = None

        results = model(frame, conf=yolo_conf, verbose=False)
        boxes = results[0].boxes

        # ... (기존 객체 탐지 로직은 그대로) ...
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

                pad_x = int(0.12 * (x2 - x1))
                pad_y = int(0.12 * (y2 - y1))
                x1p = max(0, x1 - pad_x); y1p = max(0, y1 - pad_y)
                x2p = min(width-1, x2 + pad_x); y2p = min(height-1, y2 + pad_y)
                roi = frame[y1p:y2p, x1p:x2p]

                breed_scores = breed_index.predict_breed(roi, topk=3)
                top_breed, breed_conf = (breed_scores[0] if breed_scores else (None, 0.0))
                
                allowed = None
                if top_breed and breed_conf >= BREED_MIN_CONF:
                    allowed = set(breed_index.names_in_breed(top_breed))
                name, match_score, id_conf = robust_match(roi, registered_orb, allowed_names=allowed)

                size = pet_db.get(name, {}).get("size", "unknown") if name else "unknown"

                cand = {
                    "name": name, "yolo_c": yolo_c, "id_conf": id_conf, "match_score": match_score,
                    "rect": (x1, y1, x2, y2), "rect_pad": (x1p, y1p, x2p, y2p), "size": size,
                    "breed": top_breed, "breed_conf": breed_conf
                }
                if best_this_frame is None or yolo_c > best_this_frame["yolo_c"]:
                    best_this_frame = cand
        
        # 현재 프레임의 최선 후보를 'best'에 저장하여 루프 종료 후에도 사용
        if best_this_frame:
            best = best_this_frame

        # ... (기존 확정 로직은 그대로) ...
        # [주의] 아래 로직에서 'best' 대신 'best_this_frame'을 사용해야 합니다.
        if best_this_frame is not None and confirmed_ms is None:
            # (a) 품종 버퍼 기반 확정
            if best_this_frame["breed"] and best_this_frame["breed_conf"] >= BREED_MIN_CONF and best_this_frame["yolo_c"] >= YOLO_MIN_CONF:
                breed_confirm_buf.append(best_this_frame["breed"])
                if len(breed_confirm_buf) > CONFIRM_N: breed_confirm_buf.pop(0)
                if len(breed_confirm_buf) == CONFIRM_N and len(set(breed_confirm_buf)) == 1:
                    confirmed_breed = breed_confirm_buf[-1]

            # (b) 이름 확정
            if best_this_frame["name"] and best_this_frame["id_conf"] >= ID_MIN_CONF and best_this_frame["yolo_c"] >= YOLO_MIN_CONF:
                name_confirm_buf.append(best_this_frame["name"])
                if len(name_confirm_buf) > CONFIRM_N: name_confirm_buf.pop(0)
                if len(name_confirm_buf) == CONFIRM_N and len(set(name_confirm_buf)) == 1:
                    confirmed_name = name_confirm_buf[-1]
                    confirmed_ms = int((time.time() - t0) * 1000)
                    info = (pet_db.get(confirmed_name) or {})
                    send_to_arduino(ser, confirmed_name, (best_this_frame['breed_conf']*100.0), info.get('size', 'unknown'),
                                    float(info.get('weight', 0)), float(info.get('activeLvl', 0)),
                                    float(info.get('calPerKg', 0)), int(info.get('feedingCount', 0)))
                    print(f"[SUMMARY] {vname} confirmed_at={confirmed_ms}ms (name={confirmed_name})")
            else:
                name_confirm_buf.clear()

        if confirmed_breed is not None and confirmed_name is None and best_this_frame is not None:
            fallback = pick_name_by_breed(confirmed_breed, breed_index, pet_db)
            if fallback:
                confirmed_name = fallback
                confirmed_ms = int((time.time() - t0) * 1000)
                info = (pet_db.get(confirmed_name) or {})
                send_to_arduino(ser, confirmed_name, (best_this_frame['breed_conf']*100.0), info.get('size', 'unknown'),
                                float(info.get('weight', 0)), float(info.get('activeLvl', 0)),
                                float(info.get('calPerKg', 0)), int(info.get('feedingCount', 0)))
                print(f"[SUMMARY] {vname} breed_confirmed={confirmed_breed} -> name_fallback={confirmed_name}")

        # ... (시각화 및 나머지 로직은 거의 동일) ...

        if show_window:
            read_from_arduino(ser)
            cv2.imshow("DEMO", frame)
            key = cv2.waitKey(int(1000.0/max(1.0, fps))) & 0xFF
            if key in (27, ord('q')): break

        if confirmed_ms is not None:
            if show_window:
                cv2.imshow("DEMO", frame)
                cv2.waitKey(500)
            break

    cap.release()
    if show_window:
        cv2.destroyAllWindows()

    # <<< [수정됨] 수신 대기 루프 부분
    if confirmed_ms is not None and ser is not None:
        print("\n[INFO] 강아지 정보 전송 완료. 아두이노 급식 프로세스 모니터링 시작...")
        print("[INFO] 약 60초간 응답을 기다립니다. (강제 종료: Ctrl+C)")
        
        monitoring_start_time = time.time()
        while time.time() - monitoring_start_time < 60.0: 
            read_from_arduino(ser)
            time.sleep(0.1)
        
        print("\n[INFO] 모니터링 시간이 경과하여 프로그램을 종료합니다.")
    
    if ser:
        try:
            ser.close()
            print("[SERIAL] Port closed.")
        except Exception:
            pass

    if confirmed_ms is None and confirmed_breed is None:
        print(f"[SUMMARY] {vname} confirmed_at=None (no stable confirmation)")
    else:
        print("\n[RESULT]")
        print(f"{vname} 결과")
        print(f"- 확정 시간: {confirmed_ms if confirmed_ms is not None else 'N/A'} ms")
        print(f"- 이름: {confirmed_name or 'N/A'}")
        print(f"- 품종: {confirmed_breed or (best['breed'] if best else 'N/A')}")
        info = ({} if confirmed_name is None else pet_db.get(confirmed_name, {}))
        for k in ["size", "weight", "age", "activeLvl", "calPerKg", "feedingCount"]:
            if k in info:
                print(f"- {k}: {info[k]}")
        print("")

# ... (스크립트의 나머지 부분은 그대로) ...

if __name__ == "__main__":
    ap = argparse.ArgumentParser()
    ap.add_argument("--input_dir", default="test_video_2", help="단일 영상이 들어있는 폴더")
    ap.add_argument("--pet_dir", default="pet_images")
    ap.add_argument("--pet_db", dest="pet_db_path", default="pet_db.json")
    ap.add_argument("--weights", dest="yolo_weights", default="yolov8n.pt")
    ap.add_argument("--conf", dest="yolo_conf", type=float, default=0.25)
    ap.add_argument("--device", default=None, help="cuda:0 / mps / cpu 등")
    ap.add_argument("--noshow", action="store_true", help="창 표시 안 함")
    args = ap.parse_args()

    main(
        input_dir=args.input_dir,
        pet_dir=args.pet_dir,
        pet_db_path=args.pet_db_path,
        yolo_weights=args.yolo_weights,
        yolo_conf=args.yolo_conf,
        device=args.device,
        show_window=not args.noshow
    )
