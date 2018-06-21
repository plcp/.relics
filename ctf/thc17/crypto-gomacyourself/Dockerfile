FROM debian:jessie

ARG WD="/srv"

RUN apt-get update && \
    apt-get upgrade -y && \
    apt-get install -y python3-dev python3-pip && \
    pip3 install twisted && \
    useradd -s /bin/nologin chall

WORKDIR $WD
ADD src/server.py src/crypto.py src/flag-1.txt src/flag-2.txt "$WD/"
RUN find . -type f -exec chmod 444 {} +

EXPOSE 1058
USER chall

CMD python3 server.py
