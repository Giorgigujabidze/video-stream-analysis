# Stream Analysis Build Guide

building and installing the stream analysis tool with its dependencies on
RHEL 9.

## Table of Contents

- [Prerequisites](#prerequisites)
- [Dependencies Installation](#dependencies-installation)
- [Building FFmpeg](#building-ffmpeg)
- [Building OpenCV](#building-opencv)
- [JSON Library Installation](#json-library-installation)
- [Building the Project](#building-the-project)
- [Usage](#usage)
- [Configuration Options](#configuration-options)

## Prerequisites

Enable external libraries for RHEL 9:

```bash
# Enable CodeReady Builder repository
sudo subscription-manager repos --enable codeready-builder-for-rhel-9-$(arch)-rpms

# Install EPEL repository
sudo dnf install https://dl.fedoraproject.org/pub/epel/epel-release-latest-9.noarch.rpm

# Install RPM Fusion repositories
sudo dnf install https://download1.rpmfusion.org/free/el/rpmfusion-free-release-$(rpm -E %rhel).noarch.rpm
sudo dnf install https://download1.rpmfusion.org/nonfree/el/rpmfusion-nonfree-release-$(rpm -E %rhel).noarch.rpm
```

## Dependencies Installation

Install required development tools and libraries:

```bash
# Install development tools
sudo dnf groupinstall "Development Tools" -y
sudo dnf install autoconf automake cmake git libtool pkgconfig nasm yasm -y

# Install media libraries and dependencies
sudo dnf install \
    freetype-devel \
    lame-devel \
    opus-devel \
    libvorbis-devel \
    libvpx-devel \
    x264-devel \
    x265-devel \
    libva-devel \
    libass-devel \
    zlib-devel \
    openssl-devel \
    intel-media-driver \
    libva-utils \
    libvdpau-devel -y
```

## Building FFmpeg

Clone and build FFmpeg with custom configuration:

```bash
git clone https://git.ffmpeg.org/ffmpeg.git
cd ffmpeg

./configure --prefix=/usr/local \
    --enable-gpl \
    --enable-nonfree \
    --enable-libx264 \
    --enable-libx265 \
    --enable-libvpx \
    --enable-libass \
    --enable-libfreetype \
    --enable-libvorbis \
    --enable-libmp3lame \
    --enable-openssl \
    --enable-shared \
    --enable-pic \
    --enable-libopus \
    --enable-vaapi

make -j$(nproc)
sudo make install
sudo ldconfig

# Configure library path
echo "/usr/local/lib" | sudo tee /etc/ld.so.conf.d/local-lib.conf
sudo ldconfig
```

## Building OpenCV

Download and build OpenCV with contrib modules:

```bash
# Set PKG config path
export PKG_CONFIG_PATH=/usr/local/lib/pkgconfig:$PKG_CONFIG_PATH

# Download OpenCV and contrib modules
wget -O opencv.zip https://github.com/opencv/opencv/archive/4.10.0.zip
wget -O opencv_contrib.zip https://github.com/opencv/opencv_contrib/archive/4.10.0.zip
unzip opencv.zip
unzip opencv_contrib.zip

# Build OpenCV
cd opencv-4.10.0
mkdir build && cd build

cmake -D CMAKE_BUILD_TYPE=RELEASE \
    -D CMAKE_INSTALL_PREFIX=/usr/local \
    -D WITH_VA=ON \
    -D WITH_VA_INTEL=ON \
    -D WITH_FFMPEG=ON \
    -D WITH_GSTREAMER=ON \
    -D OPENCV_GENERATE_PKGCONFIG=YES \
    -D OPENCV_EXTRA_MODULES_PATH=../../opencv_contrib-4.10.0/modules \
    ..

make -j$(nproc)
sudo make install
sudo ldconfig
```

## JSON Library Installation

Install the JSON development library:

```bash
sudo dnf install nlohmann-json-devel
```

## Building the Project

Build the stream analysis tool:

```bash
mkdir build
cd build
cmake ..
make
```

## Usage

Run the stream analysis tool:

```bash
cd build
./stream_analysis config.json results.csv
```

## Configuration Options

### Hardware Acceleration Settings

The `hardware_acceleration` parameter in the configuration file accepts the following values:

| Value | Description                                                     |
|-------|-----------------------------------------------------------------|
| 0     | Software processing (no hardware acceleration)                  |
| 1     | Automatic (prefer hardware acceleration with software fallback) |
| 2     | DirectX 11                                                      |
| 3     | VAAPI                                                           |
| 4     | Intel MediaSDK/oneVPL (MFX)                                     |

Example configuration:

```json
{
  "hardware_acceleration": 1
}
```