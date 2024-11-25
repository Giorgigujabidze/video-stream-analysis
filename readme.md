# Stream Analysis Build Guide

building and installing the stream analysis tool with its dependencies on
RHEL 9.

## Table of Contents

- [Prerequisites](#prerequisites)
- [Dependencies Installation](#dependencies-installation)
- [Using gstreamer backand with opencv (easier installation)](#using-gstreamer-backand-with-opencv-easier-installation)
- [Building FFmpeg](#building-ffmpeg)
- [Building OpenCV](#building-opencv)
- [JSON Library Installation](#json-library-installation)
- [Building the Project](#building-the-project)
- [Usage](#usage)
- [ Understanding Results](#understanding-results)
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

## Using gstreamer backand with opencv (easier installation)

### Gstreamer setup

this plugins should already be on rhel 9 if not
install:

```bash
sudo dnf install -y gstreamer1-plugins-base
sudo dnf install -y gstreamer1-plugins-good
sudo dnf install -y gstreamer1-plugins-bad-free

```

install necessary dependency

```bash
sudo dnf install -y gstreamer1-libav
```

### opencv setup

```bash
sudo dnf install opencv opencv-devel
```

### note

if you use gstreamer backand with opencv there is
no need to build ffmpeg and opencv from source, in config
file, change api backand. see [configuration options.](#configuration-options)

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
    --enable-vaapi \
    --enable-pthreads

make -j$(nproc)
sudo make install
sudo ldconfig

# Configure library path
echo "/usr/local/lib" | sudo tee /etc/ld.so.conf.d/local-lib.conf
sudo ldconfig
cd ..
```

## Building OpenCV

Download and build OpenCV with contrib modules:

```bash
# Set PKG streams path
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

you can build this project from source
or [download a release tar](https://github.com/Giorgigujabidze/video-stream-analysis/releases)

Build the stream analysis tool:

```bash
mkdir build
cd build
cmake ..
make
```

## Usage

Run the stream analysis tool in config generation mode:

```bash
./stream_analysis -c data.csv
```

Run the stream analysis tool:

```bash
./stream_analysis
```

Run the stream analysis too on n files

```bash
./stream_nalaysis -n <count>
```

### Understanding Results

The tool outputs its analysis results to a Json file.

```json
{
  "black_frame_count": 0,
  "blank_frame_count": 0,
  "coloured_stripes_detected": false,
  "no_input_stream": false,
  "static_frame_count": 0
}
```

#### Json Description

| Name                      | Type            | Description                                               |
|---------------------------|-----------------|-----------------------------------------------------------|
| no_input_stream           | boolean   (0/1) | Whether no input stream is detected  (1 = yes, 0  = no  ) |
| blank_frame_count         | integer         | Number of blank frames detected in this period            |
| static_frame_count        | integer         | Number of static (frozen) frames detected                 |
| black_frame_count         | integer         | Number of black frames detected                           |
| coloured_stripes_detected | boolean (0/1)   | Whether colored stripes were detected (1 = yes, 0 = no)   |

## Configuration Options

The stream analysis tool is configured using a JSON configuration file. Here's a complete reference of all available
options:

### Basic Settings

| Parameter         | Description                                      | Type    |
|-------------------|--------------------------------------------------|---------|
| url               | Input stream URL (e.g., UDP endpoint)            | string  |
| interval          | Analysis interval in seconds                     | integer |
| output_to_console | option to log results to console                 | boolean |
| save_last_frame   | option to save last frame of processing interval | boolean |

### Api Backend Settings

| Value | Description           |
|-------|-----------------------|
| 1800  | use gstreamer backend |
| 1900  | use ffmpeg backend    |

### Hardware Acceleration Settings

The `hardware_acceleration` parameter accepts the following values:

| Value | Description                                                     |
|-------|-----------------------------------------------------------------|
| 0     | Software processing (no hardware acceleration)                  |
| 1     | Automatic (prefer hardware acceleration with software fallback) |
| 2     | DirectX 11                                                      |
| 3     | VAAPI                                                           |
| 4     | Intel MediaSDK/oneVPL (MFX)                                     |

use 1 for better compatibility

### Frame Processing Settings

| Value | Description                                                                                     |
|-------|-------------------------------------------------------------------------------------------------|
| 1-5   | program will process every value-th frame. could be higher, but may introduce some inaccuracies |

### Analysis Thresholds

The `thresholds` object contains various threshold settings for analysis:

| Parameter                      | Description                                    | Type  |
|--------------------------------|------------------------------------------------|-------|
| static_frame_threshold         | Threshold for detecting static frames          | float |
| coloured_stripes_threshold     | Threshold for colored stripe detection         | float |
| coloured_stripes_max_deviation | Maximum allowed deviation for stripe detection | float |
| black_frame_threshold          | Threshold for black frame detection            | float |

### Size Parameters

The `size_parameters` object contains buffer size configurations:

| Parameter            | Description                           | Type    |
|----------------------|---------------------------------------|---------|
| max_mean_buffer_size | Maximum size of the mean value buffer | integer |

### Example Configuration

```json
{
  "api_backend": 1900,
  "hardware_acceleration": 0,
  "interval": 30,
  "output_to_console": false,
  "save_last_frame": true,
  "process_every_nth_frame": 4,
  "size_parameters": {
    "max_mean_buffer_size": 15
  },
  "thresholds": {
    "black_frame_threshold": 15.0,
    "coloured_stripes_max_deviation": 10.0,
    "coloured_stripes_threshold": 85.0,
    "static_frame_threshold": 0.05
  }
}

```