import cv2
import os

# ORB 특징 추출기 + 매칭기
orb = cv2.ORB_create(nfeatures=500)
bf = cv2.BFMatcher(cv2.NORM_HAMMING, crossCheck=True)

def load_registered_images(folder="pet_images"):
    """
    강아지별 폴더에서 등록 이미지 불러오기
    구조: pet_images/강아지이름/*.jpg
    """
    registered = {}
    for dog_name in os.listdir(folder):
        dog_path = os.path.join(folder, dog_name)
        if os.path.isdir(dog_path):  # 폴더일 경우만
            dog_features = []
            for file in os.listdir(dog_path):
                if file.lower().endswith((".jpg", ".png", ".jpeg")):
                    img = cv2.imread(os.path.join(dog_path, file), cv2.IMREAD_GRAYSCALE)
                    if img is None:
                        continue
                    kp, des = orb.detectAndCompute(img, None)
                    if des is not None:
                        dog_features.append(des)
            if dog_features:
                registered[dog_name] = dog_features
                print(f"[INFO] {dog_name}: {len(dog_features)} images loaded")
    return registered


def match_dog(roi, registered):
    """
    ROI 이미지와 등록 DB 비교 후 가장 유사한 강아지 리턴
    - roi: 탐지된 강아지 영역
    - registered: load_registered_images() 결과
    """
    gray = cv2.cvtColor(roi, cv2.COLOR_BGR2GRAY)
    kp_roi, des_roi = orb.detectAndCompute(gray, None)

    best_match = None
    best_score = 0

    if des_roi is not None:
        for name, features_list in registered.items():
            # 각 강아지 등록 사진들 중에서 가장 좋은 매칭 점수 선택
            total_score = 0
            for des_reg in features_list:
                matches = bf.match(des_roi, des_reg)
                score = len(matches)
                total_score = max(total_score, score)
            if total_score > best_score:
                best_score = total_score
                best_match = name

    return best_match, best_score
