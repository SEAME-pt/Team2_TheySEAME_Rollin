FROM python:3.12-slim
WORKDIR /app
RUN pip install --no-cache-dir paho-mqtt psycopg2-binary
COPY bridge.py .
CMD ["python", "-u", "bridge.py"]
