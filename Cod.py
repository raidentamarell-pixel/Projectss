import cv2 
import numpy as np

cap = cv2.VideoCapture(0)

framec = 0
mode = 0

while True:
    ret, frame = cap.read()
    framec += 1
    
    cv2.imshow("camera", frame)
    
    if not ret:
        print("Gagal")
        break
    
    elif framec %10 == 0:
        print(framec)
        
    elif mode == 1:
        temp = frame.copy()
        temp[:, :, 1] = 0
        temp[:, :, 2] = 0
        display_frame = temp
    
    elif mode == 2:
        temp = frame.copy()
        temp[:, :, 0] = 0
        temp[:, :, 2] = 0
        display_frame = temp
        
    elif mode == 3:
        temp = frame.copy()
        temp[:, :, 1] = 0
        temp[:, :, 0] = 0
        display_frame = temp
        
    elif mode == 4:
        display_frame = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
        
    elif mode == 5:
        display_frame = cv2.bitwise_not(frame)
        
    else:
        display_frame = frame
    
    key = cv2.waitKey(1) & 0xFF
    
    if key == ord('q'):
        break
    elif key == ord('0'):
        mode = 0
    elif key == ord('1'):
        mode = 1
    elif key == ord('2'):
        mode = 2
    elif key == ord('3'):
        mode = 3
    elif key == ord('4'):
        mode = 4
    elif key == ord('5'):
        mode = 5
    

cap.release()
cv2.destroyAllWindows()