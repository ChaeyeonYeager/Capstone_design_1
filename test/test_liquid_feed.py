# test_liquid_feed.py
"""
OpenCV를 이용한 유동식 잔여량 분석 독립 테스트
사용법: python test_liquid_feed.py <image_path>
"""

import sys
import cv2
import numpy as np

def analyze_liquid(path):
    img = cv2.imread(path)
    if img is None:
        print("이미지를 불러올 수 없습니다.")
        return
    h, w = img.shape[:2]

    # 1) 그릇 외곽 컨투어 검출
    gray = cv2.cvtColor(img, cv2.COLOR_BGR2GRAY)
    edges = cv2.Canny(gray, 50, 150)
    cnts, _ = cv2.findContours(edges, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)
    if not cnts:
        print("컨투어를 찾을 수 없습니다.")
        return
    bowl = max(cnts, key=cv2.contourArea)
    mask = np.zeros((h, w), np.uint8)
    cv2.drawContours(mask, [bowl], -1, 255, -1)
    bowl_area = cv2.countNonZero(mask)

    # 2) 유동식 색상 분할 (밝은 색 톤 예시)
    hsv = cv2.cvtColor(img, cv2.COLOR_BGR2HSV)
    lower = np.array([0, 0, 200])
    upper = np.array([180, 30, 255])
    liquid = cv2.inRange(hsv, lower, upper)
    liquid = cv2.bitwise_and(liquid, liquid, mask=mask)
    liquid_area = cv2.countNonZero(liquid)

    ratio = liquid_area / float(bowl_area)
    print(f"Bowl area: {bowl_area}, Liquid area: {liquid_area}, Remaining ratio: {ratio:.3f}")

    # 결과 시각화
    cv2.imshow('Bowl Mask', mask)
    cv2.imshow('Liquid Mask', liquid)
    cv2.waitKey(0)

if __name__ == '__main__':
    if len(sys.argv) < 2:
        print("Usage: python test_liquid_feed.py <image_path>")
    else:
        analyze_liquid(sys.argv[1])
