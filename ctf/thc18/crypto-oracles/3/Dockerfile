FROM python:3.6-stretch

ARG WD="/srv"

WORKDIR $WD

RUN useradd -s /bin/nologin chall && \
    pip install cryptography

ADD flag crypto.py server.py "$WD/"
RUN chmod 500 server.py && \
    chmod 400 flag crypto.py && \
    chown chall flag crypto.py server.py

# Preparation

EXPOSE 7123
USER chall

CMD python server.py 0.0.0.0 7123
