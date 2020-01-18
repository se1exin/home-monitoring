FROM arm32v7/python:2.7-alpine

COPY requirements.txt /
RUN pip install -r /requirements.txt

COPY . /app
WORKDIR /app

CMD ["python", "-u", "main.py"]
