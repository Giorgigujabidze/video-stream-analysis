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
cd ..
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

you can build this project from source or [download a release tar](https://github.com/Giorgigujabidze/video-stream-analysis/releases)

Build the stream analysis tool:

```bash
mkdir build
cd build
cmake ..
make
```

## Usage

Before running the tool, check that settings are correct in config.json
such as color_ranges_path and hardware_acceleration settings.

Run the stream analysis tool:

```bash
cd build
./stream_analysis config.json results.csv
```

### Understanding Results

The tool outputs its analysis results to a CSV file (results.csv). Each line in the output file represents a
single analysis period with the following format:

```
timestamp, blank_frame_count, static_frame_count, black_frame_count, coloured_stripes_detected
```

#### Column Descriptions

| Column                    | Type            | Description                                               |
|---------------------------|-----------------|-----------------------------------------------------------|
| timestamp                 | string          | ISO formatted timestamp of the analysis period            |
| no_input_stream           | boolean   (0/1) | Whether no input stream is detected  (1 = yes, 0  = no  ) |
| blank_frame_count         | integer         | Number of blank frames detected in this period            |
| static_frame_count        | integer         | Number of static (frozen) frames detected                 |
| black_frame_count         | integer         | Number of black frames detected                           |
| coloured_stripes_detected | boolean (0/1)   | Whether colored stripes were detected (1 = yes, 0 = no)   |

#### Example Output

```csv
2024-03-15T14:30:00,  1, 0,  11,  0,  0
2024-03-15T14:30:30,  0, 5,  0,  2,  1
```

In this example:

- First row: At 14:30:00,stream has no input and is odd, there were no blank frames, 11 static frames, no black frames,
  and no colored stripes
- Second row: At 14:30:30, stream has input and functions normally, there were 5 blank frames, no static frames, 2 black
  frames, and colored stripes were
  detected

## Configuration Options

The stream analysis tool is configured using a JSON configuration file. Here's a complete reference of all available
options:

### Basic Settings

| Parameter         | Description                               | Type    |
|-------------------|-------------------------------------------|---------|
| url               | Input stream URL (e.g., UDP endpoint)     | string  |
| color_ranges_path | Path to the color ranges definition file  | string  |
| max_log_number    | Maximum number of log entries to maintain | integer |
| interval          | Analysis interval in seconds              | integer |
| output_to_console | option to log results to console          | boolean |

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
  "url": "udp://0.0.0.0:1232",
  "color_ranges_path": "/home/gio/CLionProjects/untitled8/color_ranges/color_ranges.json",
  "hardware_acceleration": 3,
  "max_log_number": 1000,
  "interval": 30,
  "output_to_console": false,
  "thresholds": {
    "static_frame_threshold": 0.06,
    "coloured_stripes_threshold": 85,
    "coloured_stripes_max_deviation": 10,
    "black_frame_threshold": 15
  },
  "size_parameters": {
    "max_mean_buffer_size": 10
  }
}

```