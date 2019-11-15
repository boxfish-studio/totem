FROM ubuntu:18.04

# Update OS and settings
RUN apt update && apt -y install locales
RUN locale-gen en_US.UTF-8
ENV LANG en_US.UTF-8
ENV LANGUAGE en_US:en
ENV LC_ALL en_US.UTF-8

# Install dependencies for the build
RUN apt-get update && apt-get install -y \
	build-essential \
	git \
	wget \
	sudo

# Set Java environment
ENV JAVA_VER 8
ENV JAVA_HOME /usr/lib/jvm/jdk1.8.0_211

# Install Java and check version
RUN cd / && git clone https://github.com/boxfish-studio/ubuntu_quick_setup.git && \
    sh ubuntu_quick_setup/gdrive_download.sh 12bzQbyEEegk9LxJOV9nvEJ-5bDH4M2Mt jdk-8u211-linux-x64.tar.gz && \
    mkdir /usr/lib/jvm && mv jdk-8u211-linux-x64.tar.gz /usr/lib/jvm/jdk-8u211-linux-x64.tar.gz && cd /usr/lib/jvm && \
    tar -xvzf jdk-8u211-linux-x64.tar.gz && rm jdk-8u211-linux-x64.tar.gz &&  \
    update-alternatives --install "/usr/bin/java" "java" "/usr/lib/jvm/jdk1.8.0_211/bin/java" 0 && \
    update-alternatives --list java && java -version

# Install CMake and check version
RUN	cd / && wget -nv https://github.com/Kitware/CMake/releases/download/v3.15.4/cmake-3.15.4-Linux-x86_64.tar.gz && \
		tar -zxvf cmake-3.15.4-Linux-x86_64.tar.gz && cp cmake-3.15.4-Linux-x86_64/bin/* /usr/bin/ && cp -r cmake-3.15.4-Linux-x86_64/share/* /usr/share/ && \
		cmake --version

# Install ARM-GNU Cross Compiler and check version
RUN cd / && wget -nv https://developer.arm.com/-/media/Files/downloads/gnu-rm/8-2019q3/RC1.1/gcc-arm-none-eabi-8-2019-q3-update-linux.tar.bz2 && \
		tar xf gcc-arm-none-eabi-8-2019-q3-update-linux.tar.bz2 && ./gcc-arm-none-eabi-8-2019-q3-update/bin/arm-none-eabi-gcc --version

ENV PATH="/gcc-arm-none-eabi-8-2019-q3-update/bin:${PATH}"

# Clear installation
RUN	apt-get clean && rm -rf /var/lib/apt/lists/* /tmp/* /var/tmp/* /ubuntu_quick_setup /cmake-3.15.4-Linux-x86_64.tar.gz /cmake-3.15.4-Linux-x86_64 /gcc-arm-none-eabi-8-2019-q3-update-linux.tar.bz2
