EasyFramework v0.4.3
====================
Little framework created during the study of language c.<br/>
easyalloc......: Manager Memory and Customize thread safe malloc<br/>
easybenchmark..: Measures the time code<br/>
easyconsole....: Large number of functions for the management of the terminal<br/>
easycrypto.....: Hash, MD5, AES<br/>
easyfile.......: Management file<br/>
easyhttp.......: Http get/post, Ftp, Imap<br/>
easylist.......: List, Code with priority, Tree<br/>
easymarklang...: special xml<br/>
easymath.......: Date, Matrix, FFT, etc<br/>
easyopt........: Easy way opt<br/>
easyprocess....: Info of process,processor,inet etc, easy way pipe,system, cpu speed, internet speed,etc<br/>
easyserial.....: Serial port<br/>
easysocket.....: Server and client and working for ssl<br/>
easystring.....: Working with string<br/>
easythread.....: Working with thread, and named mutex<br/>
easytype.......: Var type<br/>

State:
======
* Create man documentation (easyframework easytype easyalloc easybenchmark easyconsole easycrypto)(0.4)
* resolve man (0.4.1)
* add fix gcc bug (0.4.2)

Bug:
====
GCC version 4.8 could not export some symbols to solve use --fix-dso option with path where
 ubicate libgcc.a for example<br/>
$ ./generate --fix-dso /usr/lib/gcc/arm-linux-gnueabihf/4.8/libgcc.a<br/>
<br/>
In some machines linking with ld not working properly causing the error:"hidden symbol '__stack_chk_fail_local' isn't defined" to solve use --gcc-linker for example<br/>
$ ./generate --gcc-linker<br/>
<br/>
<br/>
(easyalloc     )No Bug Reported<br/>
(easybenchmark )No Bug Reported<br/>
(easyconsole   )con_input() for now not support resize screen<br/>
(easycrypto    )No Bug Reported<br/>
(easyfile      )No Bug Reported<br/>
(easyhttp      )No Bug Reported<br/>
(easylist      )No Bug Reported<br/>
(easymarklang  )No Bug Reported<br/>
(easymath      )No Bug Reported<br/>
(easyopt       )No Bug Reported<br/>
(easyprocess   )No Bug Reported<br/>
(easyserial    )No Bug Reported<br/>
(easysocket    )No Bug Reported<br/>
(easystring    )No Bug Reported<br/>
(easythread    )No Bug Reported<br/>
(easytype      )No Bug Reported<br/>

Require:
========
pthread<br/>
curl

To install it:
==============
$ ./generate<br/>
$ sudo ./install

To uninstall it:
==============
$ sudo ./uninstall
