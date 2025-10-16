# # 기존 ORB 매칭 로직을 그대로 활용하되, 테스트용으로 점수 정규화(0~100%)를 추가했습니다.

# # 등록 이미지 폴더 구조는 동일: pet_images/강아지이름/*.jpg

# # test_embedding_utils.py
# import cv2
# import os
# import numpy as np

# # ORB 특징 추출기 + 매칭기 (테스트에 충분한 기본값)
# orb = cv2.ORB_create(nfeatures=600)
# bf = cv2.BFMatcher(cv2.NORM_HAMMING, crossCheck=True)

# VALID_EXTS = (".jpg", ".jpeg", ".png", ".bmp", ".webp")

# def _read_gray(path):
#     img = cv2.imread(path, cv2.IMREAD_GRAYSCALE)
#     return img

# def load_registered_images(folder="pet_images", verbose=True):
#     """
#     강아지별 폴더에서 등록 이미지의 ORB descriptor 목록을 불러옵니다.
#     return: { dog_name: [descriptor(np.ndarray), ...], ... }
#     """
#     registered = {}
#     if not os.path.isdir(folder):
#         if verbose:
#             print(f"[WARN] 등록 폴더가 없습니다: {folder}")
#         return registered

#     for dog_name in os.listdir(folder):
#         dog_path = os.path.join(folder, dog_name)
#         if not os.path.isdir(dog_path):
#             continue

#         dog_features = []
#         for file in os.listdir(dog_path):
#             if file.lower().endswith(VALID_EXTS):
#                 img = _read_gray(os.path.join(dog_path, file))
#                 if img is None:
#                     continue
#                 kp, des = orb.detectAndCompute(img, None)
#                 if des is not None and len(des) > 0:
#                     dog_features.append(des)

#         if dog_features:
#             registered[dog_name] = dog_features
#             if verbose:
#                 print(f"[INFO] {dog_name}: {len(dog_features)} images loaded")
#         else:
#             if verbose:
#                 print(f"[WARN] {dog_name}: 사용 가능한 이미지가 없습니다.")

#     return registered


# def match_dog(roi_bgr, registered, min_roi=48):
#     """
#     ROI(BGR)와 등록 DB를 비교하여 가장 유사한 강아지 반환.
#     return:
#       best_name(str or None),
#       best_score(int, 매칭 개수),
#       conf_pct(int, 0~100 정규화 신뢰도)
#     """
#     if roi_bgr is None or roi_bgr.size == 0:
#         return None, 0, 0

#     h, w = roi_bgr.shape[:2]
#     if min(h, w) < min_roi:
#         # 너무 작은 ROI는 매칭 신뢰도가 급격히 떨어짐
#         return None, 0, 0

#     gray = cv2.cvtColor(roi_bgr, cv2.COLOR_BGR2GRAY)

#     # 약간의 대비 향상(조명 편차 완화)
#     gray = cv2.equalizeHist(gray)

#     kp_roi, des_roi = orb.detectAndCompute(gray, None)
#     if des_roi is None or len(des_roi) == 0:
#         return None, 0, 0

#     best_name, best_score = None, 0

#     for name, features_list in registered.items():
#         # 등록 이미지들 중 '최선' 점수를 사용
#         max_match = 0
#         for des_reg in features_list:
#             if des_reg is None or len(des_reg) == 0:
#                 continue
#             matches = bf.match(des_roi, des_reg)
#             # 거리 순 정렬 후 상위 80% 정도만 신뢰 매칭으로 집계
#             if not matches:
#                 continue
#             matches = sorted(matches, key=lambda m: m.distance)
#             keep = int(max(1, round(len(matches) * 0.8)))
#             score = keep  # 간단히 개수로 점수화
#             if score > max_match:
#                 max_match = score
#         if max_match > best_score:
#             best_score = max_match
#             best_name = name

#     # 정규화 신뢰도: ROI 키포인트 수를 기준으로 0~100%
#     denom = max(1, len(des_roi))
#     conf_pct = int(np.clip(100.0 * (best_score / denom), 0, 100))
#     return best_name, int(best_score), int(conf_pct)

# # test_embedding_utils.py
# import cv2, os, numpy as np

# # ORB 파라미터 보강
# orb = cv2.ORB_create(
#     nfeatures=800, scaleFactor=1.2, nlevels=8,
#     edgeThreshold=15, patchSize=31, fastThreshold=7
# )
# # knnMatch 쓸 거라 crossCheck=False
# bf = cv2.BFMatcher(cv2.NORM_HAMMING, crossCheck=False)

# VALID_EXTS = (".jpg", ".jpeg", ".png", ".bmp", ".webp")

# def _read_gray(path):
#     return cv2.imread(path, cv2.IMREAD_GRAYSCALE)

# def load_registered_images(folder="pet_images", verbose=True):
#     reg = {}
#     if not os.path.isdir(folder):
#         if verbose: print(f"[WARN] 등록 폴더가 없습니다: {folder}")
#         return reg

#     for dog_name in os.listdir(folder):
#         dpath = os.path.join(folder, dog_name)
#         if not os.path.isdir(dpath): continue
#         feats = []
#         for fn in os.listdir(dpath):
#             if not fn.lower().endswith(VALID_EXTS): continue
#             img = _read_gray(os.path.join(dpath, fn))
#             if img is None: continue
#             # 조명·노이즈 완화
#             img = cv2.GaussianBlur(img, (3,3), 0)
#             clahe = cv2.createCLAHE(clipLimit=2.0, tileGridSize=(8,8))
#             img = clahe.apply(img)
#             kp, des = orb.detectAndCompute(img, None)
#             if des is not None and len(des) > 0:
#                 feats.append(des)
#         if feats:
#             reg[dog_name] = feats
#             if verbose: print(f"[INFO] {dog_name}: {len(feats)} images loaded")
#         else:
#             if verbose: print(f"[WARN] {dog_name}: 사용 가능한 이미지가 없습니다.")
#     return reg

# def _good_matches(des1, des2, ratio=0.75, max_per=200):
#     """Lowe ratio test 기반 good match 개수."""
#     if des1 is None or des2 is None or len(des1)==0 or len(des2)==0:
#         return 0
#     matches = bf.knnMatch(des1, des2, k=2)
#     good = 0
#     for m_n in matches:
#         if len(m_n) < 2: continue
#         m, n = m_n
#         if m.distance < ratio * n.distance:
#             good += 1
#             if good >= max_per: break
#     return good

# def match_dog(roi_bgr, registered, min_roi=72):
#     """
#     ROI와 등록 DB 비교 → (best_name, raw_score, conf_pct 0~100)
#     conf<20은 None 처리로 흔들림 억제.
#     """
#     if roi_bgr is None or roi_bgr.size == 0:
#         return None, 0, 0
#     h, w = roi_bgr.shape[:2]
#     if min(h, w) < min_roi:
#         return None, 0, 0

#     gray = cv2.cvtColor(roi_bgr, cv2.COLOR_BGR2GRAY)
#     gray = cv2.GaussianBlur(gray, (3,3), 0)
#     clahe = cv2.createCLAHE(clipLimit=2.0, tileGridSize=(8,8))
#     gray = clahe.apply(gray)

#     kp_roi, des_roi = orb.detectAndCompute(gray, None)
#     if des_roi is None or len(des_roi) < 20:
#         return None, 0, 0

#     best_name, best_score = None, 0
#     for name, feat_list in registered.items():
#         local_best = 0
#         for des_reg in feat_list:
#             g = _good_matches(des_roi, des_reg, ratio=0.75)
#             if g > local_best: local_best = g
#         if local_best > best_score:
#             best_score = local_best
#             best_name = name

#     denom = max(40, len(des_roi))  # 과대평가 방지
#     conf = int(np.clip(100.0 * (best_score / denom), 0, 100))
#     if conf < 20:
#         return None, best_score, conf
#     return best_name, int(best_score), int(conf)


# # test_embedding_utils.py
# import cv2, os, numpy as np

# # ORB 파라미터 보강
# orb = cv2.ORB_create(
#     nfeatures=800, scaleFactor=1.2, nlevels=8,
#     edgeThreshold=15, patchSize=31, fastThreshold=7
# )
# # knnMatch 쓸 거라 crossCheck=False
# bf = cv2.BFMatcher(cv2.NORM_HAMMING, crossCheck=False)

# VALID_EXTS = (".jpg", ".jpeg", ".png", ".bmp", ".webp")

# def _read_gray(path):
#     return cv2.imread(path, cv2.IMREAD_GRAYSCALE)

# def _read_bgr(path):
#     return cv2.imread(path, cv2.IMREAD_COLOR)

# def load_registered_images(folder="pet_images", verbose=True):
#     """
#     이름/폴더별로 ORB 디스크립터 리스트를 적재.
#     return: Dict[str name] -> List[np.ndarray descriptors]
#     """
#     reg = {}
#     if not os.path.isdir(folder):
#         if verbose: print(f"[WARN] 등록 폴더가 없습니다: {folder}")
#         return reg

#     for dog_name in os.listdir(folder):
#         dpath = os.path.join(folder, dog_name)
#         if not os.path.isdir(dpath): continue
#         feats = []
#         for fn in os.listdir(dpath):
#             if not fn.lower().endswith(VALID_EXTS): continue
#             img = _read_gray(os.path.join(dpath, fn))
#             if img is None: continue
#             # 조명·노이즈 완화
#             img = cv2.GaussianBlur(img, (3,3), 0)
#             clahe = cv2.createCLAHE(clipLimit=2.0, tileGridSize=(8,8))
#             img = clahe.apply(img)
#             kp, des = orb.detectAndCompute(img, None)
#             if des is not None and len(des) > 0:
#                 feats.append(des)
#         if feats:
#             reg[dog_name] = feats
#             if verbose: print(f"[INFO] {dog_name}: {len(feats)} images loaded")
#         else:
#             if verbose: print(f"[WARN] {dog_name}: 사용 가능한 이미지가 없습니다.")
#     return reg

# def load_registered_color_images(folder="pet_images", verbose=True, max_per_name=20, min_side=224):
#     """
#     품종 임베딩 계산용으로 이름별 BGR 이미지를 적재.
#     return: Dict[str name] -> List[np.ndarray BGR]
#     """
#     reg = {}
#     if not os.path.isdir(folder):
#         if verbose: print(f"[WARN] 등록 폴더가 없습니다: {folder}")
#         return reg

#     for dog_name in os.listdir(folder):
#         dpath = os.path.join(folder, dog_name)
#         if not os.path.isdir(dpath): continue
#         imgs = []
#         for fn in sorted(os.listdir(dpath)):
#             if not fn.lower().endswith(VALID_EXTS): continue
#             img = _read_bgr(os.path.join(dpath, fn))
#             if img is None: continue
#             h, w = img.shape[:2]
#             # 너무 작은 이미지는 스킵(임베딩 품질)
#             if min(h, w) < min_side:
#                 scale = float(min_side) / max(1, min(h, w))
#                 img = cv2.resize(img, (int(w*scale), int(h*scale)))
#             imgs.append(img)
#             if len(imgs) >= max_per_name: break
#         if imgs:
#             reg[dog_name] = imgs
#             if verbose: print(f"[INFO] {dog_name}: color {len(imgs)} images loaded")
#         else:
#             if verbose: print(f"[WARN] {dog_name}: 컬러 이미지 없음/부적합.")
#     return reg

# def _good_matches(des1, des2, ratio=0.75, max_per=200):
#     """Lowe ratio test 기반 good match 개수."""
#     if des1 is None or des2 is None or len(des1)==0 or len(des2)==0:
#         return 0
#     matches = bf.knnMatch(des1, des2, k=2)
#     good = 0
#     for m_n in matches:
#         if len(m_n) < 2: continue
#         m, n = m_n
#         if m.distance < ratio * n.distance:
#             good += 1
#             if good >= max_per: break
#     return good

# def match_dog(roi_bgr, registered, min_roi=72):
#     """
#     ROI와 등록 DB 비교 → (best_name, raw_score, conf_pct 0~100)
#     conf<20은 None 처리로 흔들림 억제.
#     """
#     if roi_bgr is None or roi_bgr.size == 0:
#         return None, 0, 0
#     h, w = roi_bgr.shape[:2]
#     if min(h, w) < min_roi:
#         return None, 0, 0

#     gray = cv2.cvtColor(roi_bgr, cv2.COLOR_BGR2GRAY)
#     gray = cv2.GaussianBlur(gray, (3,3), 0)
#     clahe = cv2.createCLAHE(clipLimit=2.0, tileGridSize=(8,8))
#     gray = clahe.apply(gray)

#     kp_roi, des_roi = orb.detectAndCompute(gray, None)
#     if des_roi is None or len(des_roi) < 20:
#         return None, 0, 0

#     best_name, best_score = None, 0
#     for name, feat_list in registered.items():
#         local_best = 0
#         for des_reg in feat_list:
#             g = _good_matches(des_roi, des_reg, ratio=0.75)
#             if g > local_best: local_best = g
#         if local_best > best_score:
#             best_score = local_best
#             best_name = name

#     denom = max(40, len(des_roi))  # 과대평가 방지
#     conf = int(np.clip(100.0 * (best_score / denom), 0, 100))
#     if conf < 20:
#         return None, best_score, conf
#     return best_name, int(best_score), int(conf)

# test_embedding_utils.py
import os
import cv2
import numpy as np

# ORB 파라미터 (비교적 관대한 설정)
orb = cv2.ORB_create(
    nfeatures=800, scaleFactor=1.2, nlevels=8,
    edgeThreshold=15, patchSize=31, fastThreshold=7
)
# knnMatch 사용 → crossCheck=False
bf = cv2.BFMatcher(cv2.NORM_HAMMING, crossCheck=False)

VALID_EXTS = (".jpg", ".jpeg", ".png", ".bmp", ".webp")

def _read_gray(path):
    return cv2.imread(path, cv2.IMREAD_GRAYSCALE)

def _read_bgr(path):
    return cv2.imread(path, cv2.IMREAD_COLOR)

def load_registered_images(folder="pet_images", verbose=True):
    """
    이름/폴더별로 ORB 디스크립터 리스트를 적재.
    return: Dict[str name] -> List[np.ndarray descriptors]
    """
    reg = {}
    if not os.path.isdir(folder):
        if verbose: print(f"[WARN] 등록 폴더가 없습니다: {folder}")
        return reg

    for dog_name in os.listdir(folder):
        dpath = os.path.join(folder, dog_name)
        if not os.path.isdir(dpath):
            continue
        feats = []
        for fn in os.listdir(dpath):
            if not fn.lower().endswith(VALID_EXTS):
                continue
            img = _read_gray(os.path.join(dpath, fn))
            if img is None:
                continue
            # 조명·노이즈 완화
            img = cv2.GaussianBlur(img, (3,3), 0)
            clahe = cv2.createCLAHE(clipLimit=2.0, tileGridSize=(8,8))
            img = clahe.apply(img)
            kp, des = orb.detectAndCompute(img, None)
            if des is not None and len(des) > 0:
                feats.append(des)
        if feats:
            reg[dog_name] = feats
            if verbose: print(f"[INFO] {dog_name}: {len(feats)} images loaded")
        else:
            if verbose: print(f"[WARN] {dog_name}: 사용 가능한 이미지가 없습니다.")
    return reg

def load_registered_color_images(folder="pet_images", verbose=True, max_per_name=20, min_side=224):
    """
    품종 임베딩 계산용으로 이름별 BGR 이미지를 적재.
    return: Dict[str name] -> List[np.ndarray BGR]
    """
    reg = {}
    if not os.path.isdir(folder):
        if verbose: print(f"[WARN] 등록 폴더가 없습니다: {folder}")
        return reg

    for dog_name in os.listdir(folder):
        dpath = os.path.join(folder, dog_name)
        if not os.path.isdir(dpath):
            continue
        imgs = []
        for fn in sorted(os.listdir(dpath)):
            if not fn.lower().endswith(VALID_EXTS):
                continue
            img = _read_bgr(os.path.join(dpath, fn))
            if img is None:
                continue
            h, w = img.shape[:2]
            # 너무 작은 이미지는 리사이즈
            if min(h, w) < min_side:
                scale = float(min_side) / max(1, min(h, w))
                img = cv2.resize(img, (int(w*scale), int(h*scale)))
            imgs.append(img)
            if len(imgs) >= max_per_name:
                break
        if imgs:
            reg[dog_name] = imgs
            if verbose: print(f"[INFO] {dog_name}: color {len(imgs)} images loaded")
        else:
            if verbose: print(f"[WARN] {dog_name}: 컬러 이미지 없음/부적합.")
    return reg

def _good_matches(des1, des2, ratio=0.75, max_per=200):
    """Lowe ratio test 기반 good match 개수."""
    if des1 is None or des2 is None or len(des1)==0 or len(des2)==0:
        return 0
    matches = bf.knnMatch(des1, des2, k=2)
    good = 0
    for m_n in matches:
        if len(m_n) < 2:
            continue
        m, n = m_n
        if m.distance < ratio * n.distance:
            good += 1
            if good >= max_per:
                break
    return good

def match_dog(roi_bgr, registered, min_roi=48):
    """
    ROI와 등록 DB 비교 → (best_name, raw_score, conf_pct 0~100)
    conf<20은 None 처리로 흔들림 억제.
    """
    if roi_bgr is None or roi_bgr.size == 0:
        return None, 0, 0
    h, w = roi_bgr.shape[:2]
    if min(h, w) < min_roi:
        return None, 0, 0

    gray = cv2.cvtColor(roi_bgr, cv2.COLOR_BGR2GRAY)
    gray = cv2.GaussianBlur(gray, (3,3), 0)
    clahe = cv2.createCLAHE(clipLimit=2.0, tileGridSize=(8,8))
    gray = clahe.apply(gray)

    kp_roi, des_roi = orb.detectAndCompute(gray, None)
    if des_roi is None or len(des_roi) < 12:
        return None, 0, 0

    best_name, best_score = None, 0
    for name, feat_list in registered.items():
        local_best = 0
        for des_reg in feat_list:
            g = _good_matches(des_roi, des_reg, ratio=0.75)
            if g > local_best:
                local_best = g
        if local_best > best_score:
            best_score = local_best
            best_name = name

    denom = max(40, len(des_roi))  # 과대평가 방지
    conf = int(np.clip(100.0 * (best_score / denom), 0, 100))
    if conf < 20:
        return None, best_score, conf
    return best_name, int(best_score), int(conf)
