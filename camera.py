

'''
import io
import time
import threading
import picamera

# Create a pool of image processors
done = False
lock = threading.Lock()
pool = []

class ImageProcessor(threading.Thread):
    def __init__(self):
        super(ImageProcessor, self).__init__()
        self.stream = io.BytesIO()
        self.event = threading.Event()
        self.terminated = False
        self.start()

    def run(self):
        # This method runs in a separate thread
        global done
        while not self.terminated:
            # Wait for an image to be written to the stream
            if self.event.wait(1):
                try:
                    self.stream.seek(0)
                    # Read the image and do some processing on it
                    #Image.open(self.stream)
                    #...
                    #...
                    # Set done to True if you want the script to terminate
                    # at some point
                    #done=True
                finally:
                    # Reset the stream and event
                    self.stream.seek(0)
                    self.stream.truncate()
                    self.event.clear()
                    # Return ourselves to the pool
                    with lock:
                        pool.append(self)

def streams():
    while not done:
        with lock:
            if pool:
                processor = pool.pop()
            else:
                processor = None
        if processor:
            yield processor.stream
            processor.event.set()
        else:
            # When the pool is starved, wait a while for it to refill
            time.sleep(0.1)

with picamera.PiCamera() as camera:
    pool = [ImageProcessor() for i in range(4)]
    camera.resolution = (640, 480)
    camera.framerate = 30
    #camera.start_preview()
    time.sleep(2)
    camera.capture_sequence(streams(), use_video_port=True)

# Shut down the processors in an orderly fashion
while pool:
    with lock:
        processor = pool.pop()
    processor.terminated = True
    processor.join()

    
#This will work in around 0.25 ms
'''
import time
import picamera
import picamera.array
import io
from datetime import datetime, timedelta
#import numpy as np
#import pdb

camera = 0
def init_camera():
    global camera
    camera = picamera.PiCamera()
    camera.resolution = (640, 480)
    camera.rotation = 180
    camera.framerate = 30
    camera.start_preview()
    time.sleep(2)
    camera.stop_preview()

def shoot():
    #start = time.time()
    global camera
    #output = picamera.array.PiRGBArray(camera)
    #camera.capture(output, format = 'bgr', use_video_port = True)

    #for filename in camera.capture_continuous('img{timestamp:%Y-%m-%d %H:%M:%S.%f}.jpg'):
        #print('Captured %s' % filename)
        #time.sleep(0.005)
    #outputs = [io.BytesIO() for i in range(2700)]
    #start = time.time()
    camera.capture_sequence(["obstacle7.jpg",'jpeg'],use_video_port=True)
    #finish = time.time()
    #print('Captured 2700 images at %.2ffps' % (2700/(finish-start)))
    #print('Captured 2700 images in %d seconds' % (finish-start))
    #finish = time.time()
    #print(finish - start)
    #return output.array

def grayscaleConversion(bgrArray):
    temp = 0
    global smoothGray
    for i in range(480):
        for j in range(640):
            temp = bgrArray[i][j][2] * 0.2989 + bgrArray[i][j][1] * 0.5870 + bgrArray[i][j][0] * 0.1140
            if temp < 100:
                smoothGray[i][j] = 0
            else:
                smoothGray[i][j] = 200

init_camera()
shoot()



'''
import picamera
import picamera.array
import time

with picamera.PiCamera() as camera:
    with picamera.array.PiRGBArray(camera) as output:
        camera.resolution = (640, 480)
        #camera.color_effects = (128, 128)
        camera.rotation = 180
        ft = time.time()
        camera.capture(output, format = 'jpeg', use_video_port = True)
        ft = time.time() - ft
        print(ft)
        #print('Captured %dx%d image, time %f' % (
        #        output.array.shape[1], output.array.shape[0], ft))
'''
'''
from picamera import PiCamera
from time import sleep
camera = PiCamera()
camera.resolution = (640, 480)
camera.color_effects = (128, 128)
camera.framerate = 10
camera.rotation = 180
camera.start_preview()
sleep(10)
camera.stop_preview()
'''
