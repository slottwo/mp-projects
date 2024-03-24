# Use the latest Arch Linux base image
FROM archlinux/base:latest

# Update package list and install necessary packages
RUN pacman -Syu --noconfirm \
    && pacman -S --noconfirm \
       base-devel \
       git \
       vim

# Clean up package cache to reduce image size
RUN pacman -Scc --noconfirm

# Set locale (optional)
ENV LANG en_US.UTF-8

RUN pacman -S --noconfirm \
    openssh \
    github-cli \
    gcc \
    gdb \
    openmpi \
    tmux

RUN gh repo clone slottwo/mp-projects
RUN cd mp-projects
RUN chmod +x *.sh
