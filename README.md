# NImage
New Image Library

## 1.  Install

###   1.1 Prepare

#### 1.1.1 apt install libjpeg, libpng
`sudo apt-get install libjpeg-dev libpng-dev libnanomsg-dev`
```
Desired=Unknown/Install/Remove/Purge/Hold
| Status=Not/Inst/Conf-files/Unpacked/halF-conf/Half-inst/trig-aWait/Trig-pend
|/ Err?=(none)/Reinst-required (Status,Err: uppercase=bad)
||/ Name                                Version                Architecture           Description
+++-===================================-======================-======================-============================================================================
ii  libjpeg-dev:amd64                   8c-2ubuntu8            amd64                  Independent JPEG Group's JPEG runtime library (dependency package)
ii  libpng-dev:amd64                    1.6.34-1ubuntu0.18.04. amd64                  PNG library - development (version 1.6)
```

###   1.2 Make libnimage
`make && make install`

## 2. Demo
```
cd videoplay
make 
```
## 3. Source format
```
clang-format -i --style=WebKit source/*.c include/*.h
```
