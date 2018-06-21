FROM python:3-stretch

ARG WD="/srv"

RUN apt-get update && \
    apt-get -y upgrade && \
    apt-get install -y libcap2 && \
    pip3 install cheroot

WORKDIR $WD

ADD build/ "$WD/"
RUN chmod 755 "$WD/server.py" "$WD/sandkox/sandkox-preload.so" && \
    mkdir "$WD/binaries"

# Preparation
EXPOSE 8080

CMD python3 /srv/server.py

