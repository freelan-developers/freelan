FROM debian

# Update package list
RUN apt-get update

# Install dependencies
ENV DEPENDENCIES libssl1.0.0 libboost-system1.55.0 libboost-thread1.55.0 libboost-filesystem1.55.0 libboost-date-time1.55.0 libboost-program-options1.55.0 libboost-iostreams1.55.0 libcurl4-openssl-dev
ENV BUILD_DEPENDENCIES g++ git scons libssl-dev libboost-system-dev  libboost-thread-dev libboost-filesystem-dev libboost-date-time-dev libboost-program-options-dev libboost-iostreams-dev

RUN apt-get install -y $DEPENDENCIES $BUILD_DEPENDENCIES
RUN apt-get clean

# Get FreeLAN sources
ENV FREELAN_BRANCH master

RUN git clone https://github.com/freelan-developers/freelan-all.git
RUN cd freelan-all && git checkout $FREELAN_BRANCH

# Compile FreeLAN
ENV CXX=g++
RUN cd freelan-all && scons install prefix=/

# Remove sources and dependencies
RUN rm -r freelan-all
RUN apt-get autoremove -y --purge $BUILD_DEPENDENCIES
RUN apt-get clean

# Profit !
EXPOSE 12000/udp

ENTRYPOINT ["/bin/freelan", "-f", "--tap_adapter.enabled=off", "--switch.relay_mode_enabled=yes"]
