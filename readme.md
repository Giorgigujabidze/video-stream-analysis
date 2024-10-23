
## Build ffmpeg from source:

```bash

git clone https://git.ffmpeg.org/ffmpeg.git
cd ffmpeg

./configure \
  --enable-shared \
  --enable-gpl \
  --enable-nonfree \
  --enable-libvpl \
  --enable-vaapi \
  --enable-libx264 \
  --enable-libx265 \
  --enable-libvpx \
  --enable-libass \
  --enable-libfreetype \
  --enable-libvorbis \
  --enable-libmp3lame \
  --enable-pic \
  --enable-openssl \
  --extra-cflags=-I/usr/local/include \
  --extra-ldflags=-L/usr/local/lib

make -j$(nproc)

sudo make install

sudo ldconfig

```

## Build  opencv from source:

```bash

wget -O opencv.zip https://github.com/opencv/opencv/archive/4.10.0.zip
wget -O opencv_contrib.zip https://github.com/opencv/opencv_contrib/archive/4.10.0.zip
unzip opencv.zip && unzip opencv_contrib.zip

cd opencv-4.10.0
mkdir build && cd build

cmake -D CMAKE_BUILD_TYPE=RELEASE \
-D CMAKE_INSTALL_PREFIX=/usr/local \
-D WITH_VA=ON \
-D WITH_VA_INTEL=ON \
-D WITH_FFMPEG=ON \
-D WITH_GSTREAMER=ON \
-D WITH_MFX=ON \
-D OPENCV_GENERATE_PKGCONFIG=YES \
-D ENABLE_PRECOMPILED_HEADERS=OFF \
-D OPENCV_EXTRA_MODULES_PATH=../../opencv_contrib-4.10.0/modules \
..

make -j$(nproc)
sudo make install
sudo ldconfig
```

## To build project:

```bash
mkdir build
cd build
cmake ..
make
```

## Usage:

```bash
cd cmake-build-debug
./untitled8 config.json results.json
```