FROM mcr.microsoft.com/devcontainers/cpp:1-debian-12

RUN apt-get update \
    && apt-get install -y --no-install-recommends \
        cmake \
        git \
        pkg-config \
        libgl1-mesa-dev \
        libglu1-mesa-dev \
        xorg-dev \
    && rm -rf /var/lib/apt/lists/*
