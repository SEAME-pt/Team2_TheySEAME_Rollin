import cv2

class ArucoDetector:
    
    def __init__(self):
        self.arDict = cv2.aruco.getPredefinedDictionary(cv2.aruco.DICT_4X4_50)
        self.currId = 0
        self.elapsedTime = 0
        self.timeLimit = 5

    def detect(self, frame):
        corners, marker_ids, rejected = cv2.aruco.detectMarkers(frame, self.arDict)
        if marker_ids is None:
            return
        arrayId = []
        for row in marker_ids:
            for value in row:
                arrayId.append(value)
        arrayId.sort()
        if arrayId[0] != self.currId:
            self.elapsedTime += 1
            if self.elapsedTime < self.timeLimit:
                return
        self.currId = arrayId[0]
        self.elapsedTime = 0

    def getDetection(self): 
        return (self.currId)
