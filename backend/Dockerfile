FROM python:3.11-slim

WORKDIR /app

# OpenCV / MediaPipe runtime deps (headless wheels; minimal system libs)
RUN apt-get update && apt-get install -y --no-install-recommends \
    libglib2.0-0 \
    libgomp1 \
    && rm -rf /var/lib/apt/lists/*

COPY requirements.txt .
# Install headless OpenCV before mediapipe so GUI opencv wheels are not pulled
RUN pip install --no-cache-dir opencv-python-headless opencv-contrib-python-headless \
    && pip install --no-cache-dir -r requirements.txt

COPY app ./app

EXPOSE 8000

CMD ["sh", "-c", "uvicorn app.main:app --host 0.0.0.0 --port ${PORT:-8000}"]
