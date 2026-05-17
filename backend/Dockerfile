FROM python:3.11-slim

WORKDIR /app

# System libs for opencv-python-headless + mediapipe (incl. libGLESv2.so.2)
RUN apt-get update && apt-get install -y --no-install-recommends \
    libglib2.0-0 \
    libgomp1 \
    libgl1 \
    libegl1 \
    libgles2 \
    libgbm1 \
    libxcb1 \
    libx11-6 \
    libxext6 \
    libxrender1 \
    libsm6 \
    && rm -rf /var/lib/apt/lists/*

COPY requirements.txt .

# Install app deps, then ensure headless OpenCV wins over mediapipe's GUI wheels.
# Do NOT pip uninstall opencv packages — that can remove cv2 entirely.
RUN pip install --no-cache-dir -r requirements.txt \
    && pip install --no-cache-dir --force-reinstall \
         opencv-python-headless opencv-contrib-python-headless \
    && python -c "\
import cv2;\
import mediapipe as mp;\
from mediapipe.tasks.python import vision;\
print('cv2', cv2.__version__, 'mediapipe', mp.__version__, 'vision', vision.__name__)"

COPY app ./app

EXPOSE 8000

CMD ["sh", "-c", "uvicorn app.main:app --host 0.0.0.0 --port ${PORT:-8000}"]
