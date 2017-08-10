FROM debian:stretch

# Update package list
RUN apt-get update

# Install dependencies
ENV DEPENDENCIES libssl1.1 libboost-system1.62.0 libboost-thread1.62.0 libboost-filesystem1.62.0 libboost-date-time1.62.0 libboost-program-options1.62.0 libboost-iostreams1.62.0 libcurl4-openssl-dev
ENV BUILD_DEPENDENCIES g++ git scons libssl-dev libboost-system-dev  libboost-thread-dev libboost-filesystem-dev libboost-date-time-dev libboost-program-options-dev libboost-iostreams-dev

RUN apt-get install -y $DEPENDENCIES $BUILD_DEPENDENCIES
RUN apt-get clean

# Get FreeLAN sources
ENV FREELAN_BRANCH master

RUN git clone https://github.com/freelan-developers/freelan.git
RUN cd freelan && git checkout $FREELAN_BRANCH

# Compile FreeLAN
ENV CXX=g++
RUN cd freelan && scons install prefix=/usr/

# Remove sources and dependencies
RUN rm -r freelan
RUN apt-get autoremove -y --purge $BUILD_DEPENDENCIES
RUN apt-get clean

# Profit !
EXPOSE 12000/udp

ENTRYPOINT ["/usr/bin/freelan", "-f", "--tap_adapter.enabled=off", "--switch.relay_mode_enabled=yes"]
