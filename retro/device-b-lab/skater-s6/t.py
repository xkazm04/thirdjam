try:
    import scipy; print("scipy",scipy.__version__)
except Exception as e: print("noscipy")
try:
    import cv2; print("cv2")
except Exception as e: print("nocv2")
