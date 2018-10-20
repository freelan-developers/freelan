FROM debian:stretch

ENV DEPENDENCIES libssl1.1 libboost-system1.62.0 libboost-thread1.62.0 libboost-filesystem1.62.0 libboost-date-time1.62.0 libboost-program-options1.62.0 libboost-iostreams1.62.0 libcurl4-openssl-dev libminiupnpc10
ENV BUILD_DEPENDENCIES g++ git scons libssl-dev libboost-system-dev  libboost-thread-dev libboost-filesystem-dev libboost-date-time-dev libboost-program-options-dev libboost-iostreams-dev libminiupnpc-dev
ENV FREELAN_BRANCH master
ENV CXX=g++

# Install dependencies
RUN apt-get update && \
  apt-get install -y $DEPENDENCIES $BUILD_DEPENDENCIES && \
  rm -rf /var/lib/apt/lists/*

# Get FreeLAN sources and compile it
RUN git clone -b $FREELAN_BRANCH --depth=100 https://github.com/freelan-developers/freelan.git && \
  cd freelan && \
  scons --mode=release install prefix=/usr/ -j2 && \
  cd .. && rm -rf freelan

# Profit !
EXPOSE 12000/udp

ENTRYPOINT ["/usr/bin/freelan", "-f", "--tap_adapter.enabled=off", "--switch.relay_mode_enabled=yes"]
