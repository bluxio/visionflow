FROM python:3.12-slim

WORKDIR /app

COPY requirements.txt /app/demo_app/requirements.txt
RUN pip install --no-cache-dir -r /app/demo_app/requirements.txt

COPY . /app/demo_app

EXPOSE 8080

CMD ["uvicorn", "demo_app.main:app", "--host", "0.0.0.0", "--port", "8080"]
