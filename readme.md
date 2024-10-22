## opencv installation and build

```bash

wget -O opencv.zip https://github.com/opencv/opencv/archive/4.10.0.zip                                                               14:33:07
wget -O opencv_contrib.zip https://github.com/opencv/opencv_contrib/archive/4.10.0.zip
unzip opencv.zip && unzip opencv_contrib.zip

cd opencv-4.10.0
mkdir build && cd build

cmake -D CMAKE_BUILD_TYPE=RELEASE \                                                                                              INT 14:30:15
-D CMAKE_INSTALL_PREFIX=/usr/local \
-D WITH_VA=ON \
-D WITH_VA_INTEL=ON \
-D WITH_FFMPEG=ON \
-D WITH_MFX=ON \
-D OPENCV_GENERATE_PKGCONFIG=YES \
-D ENABLE_PRECOMPILED_HEADERS=OFF \
-D OPENCV_EXTRA_MODULES_PATH=../../opencv_contrib-4.10.0/modules \
..

make -j$(nproc)
sudo make install
sudo ldconfig
```

## usage:

```bash
cd cmake-build-debug
./untitled8 config.json results.json
```