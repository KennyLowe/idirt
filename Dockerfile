FROM ubuntu:22.04

RUN apt-get update && apt-get install -y \
    gcc \
    make \
    cpp \
    procps \
    && rm -rf /var/lib/apt/lists/*

RUN mkdir -p /mud/bin /mud/data /mud/include /mud/src /mud/utils

COPY include/ /mud/include/
COPY src/ /mud/src/
COPY data/ /mud/data/
COPY utils/ /mud/utils/
COPY bin/test.sh /mud/bin/test.sh

WORKDIR /mud/src

# Fix CRLF line endings and ensure scripts are executable
RUN apt-get update && apt-get install -y dos2unix && rm -rf /var/lib/apt/lists/*
RUN find /mud -type f -exec dos2unix {} \; 2>/dev/null; true
RUN chmod +x /mud/utils/makedep /mud/utils/backup /mud/utils/fixlog /mud/utils/stats 2>/dev/null; true

# Build the generator and MUD daemon
RUN make depend && make all 2>&1

# Build setlevel utility
RUN gcc -pipe -m64 -O3 -fcommon -I../include -o /mud/bin/setlevel utils/setlevel.c

RUN chmod +x /mud/bin/test.sh /mud/bin/aberd /mud/bin/setlevel

WORKDIR /mud/bin

EXPOSE 6715

CMD ["/mud/bin/aberd", "-f"]
