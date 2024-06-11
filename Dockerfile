FROM ubuntu:20.04 as build

RUN mkdir -p /opt/build && mkdir -p /opt/dist
RUN apt-get update && apt-get upgrade -y && \
  DEBIAN_FRONTEND=noninteractive apt-get install -y --no-install-recommends \
    ca-certificates curl && \
  rm -rf /var/lib/apt/lists/*

# install cmake
RUN cd /opt/build && \
    curl -LO https://github.com/Kitware/CMake/releases/download/v3.24.3/cmake-3.24.3-linux-x86_64.sh && \
    mkdir -p /opt/dist//usr/local && \
    /bin/bash cmake-3.24.3-linux-x86_64.sh --prefix=/opt/dist//usr/local --skip-license

# cleanup
RUN rm -rf /opt/dist/usr/local/include && \
    rm -rf /opt/dist/usr/local/lib/pkgconfig && \
    find /opt/dist -name "*.a" -exec rm -f {} \; || echo ""
RUN rm -rf /opt/dist/usr/local/share/doc
RUN rm -rf /opt/dist/usr/local/share/man

FROM ubuntu:20.04

RUN apt-get update -y && apt-get install -y \
  libboost-filesystem-dev\
  libboost-program-options-dev && rm -r /var/lib/apt/lists/*
  #libboost-thread-dev \
  #libboost-log-dev\
  #libboost-system-dev

RUN apt-get update && apt-get upgrade -y && \
  DEBIAN_FRONTEND=noninteractive apt-get install -y --no-install-recommends \
    ca-certificates build-essential pkg-config gnupg libarchive13 openssh-server openssh-client wget net-tools git nano sudo && \
    rm -r /var/lib/apt/lists/*

RUN apt-get update && apt-get upgrade -y && \
  DEBIAN_FRONTEND=noninteractive apt-get install -y --no-install-recommends \
    python3 gdb gdb-doc && \
    rm -r /var/lib/apt/lists/*

RUN apt-get update && apt-get upgrade -y && \
    DEBIAN_FRONTEND=noninteractive apt-get install -y --no-install-recommends \
    default-jdk && rm -r /var/lib/apt/lists/*

COPY --from=build /opt/dist /

RUN apt-get update && apt-get upgrade -y && \
    DEBIAN_FRONTEND=noninteractive apt-get install -y --no-install-recommends \
    libtbb-dev && rm -r /var/lib/apt/lists/*

RUN mkdir encoding-tables

RUN wget https://sourceforge.net/projects/bbmap/files/BBMap_39.01.tar.gz/download 2>/dev/null && tar -xf download && mv bbmap BBMap

RUN useradd -m -s /bin/bash  -G sudo oligoarchive-user

RUN mkdir /ldpc-matrices

COPY --chown=oligoarchive-user:oligoarchive-user ldpc-matrices/ldpc* /ldpc-matrices/

COPY --chown=oligoarchive-user:oligoarchive-user oa-dsm /home/oligoarchive-user/oa-dsm

RUN cd /home/oligoarchive-user/oa-dsm/src/LDPC-codes && ln -s /ldpc-matrices matrices

USER oligoarchive-user
RUN cd /home/oligoarchive-user/oa-dsm && bash setup.sh
RUN rm -f ~/.bash_logout

WORKDIR /home/oligoarchive-user
CMD ["/bin/bash", "-l"]