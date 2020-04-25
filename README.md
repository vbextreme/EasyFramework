EasyFramework v1.2.5
====================
Little framework created during the study of language c.<br/>

State:
======
* 1.2.5 workaround media seek, av_seek not working
* 1.2.4 fix aur, media.seek but not working
* 1.2.3 test and fix
* 1.2.2 fix
* 1.2.1 graphics docs
* 1.2.0 introduce graphics
* 1.1.1 retry aur
* 1.1.0 add terminal function
* 1.0.0 full documentated, complete revision code
* 0.4.3 add fix gcc bug
* 0.4.2 resolve man
* 0.4.1 Create man documentation (easyframework easytype easyalloc easybenchmark easyconsole easycrypto)

Bug:
====

Require Developer version of:
========
tic<br/>
libm<br/>
libdl<br/>
librt<br/>
libpcre2-8<br/>
libunistring<br/>
libtar<br/>
zlib<br/>
gnutls<br/>
pthread<br/>
curl<br/>
libgit2<br/>
libpng<br/>
libjpeg<br/>
libgif<br/>
librsvg-2.0<br/>
gnutls<br/>
libcurl<br/>
libavcodec<br/>
libavdevice<br/>
libavfilter<br/>
libavformat<br/>
libavutil<br/>
libswresample<br/>
libswscale<br/>
freetype2<br/>
fontconfig<br/>
xcb<br/>
xcb-xrm<br/>
xcb-composite<br/>
xcb-util<br/>
xcb-image<br/>
xcb-randr<br/>
xcb-shape<br/>
xkbcommon<br/>
xkbcommon-x11<br/>

To install it:
==============
```
$ meson build
$ cd build
$ ninja
$ sudo ninja install
```

Doc:
================
install graphviz for doc graph
```
$ ./gendoc
```

To uninstall it:
==============

