EasyFramework v0.4
==================
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
* Create man documentation (easyframework easytype easyalloc easybenchmark easyconsole easycrypto)
* other libraries:
 1. easyuser? clean && upload
 2. easyguiconsole? to finish
 3. easygraphic? to finish

Bug:
====
Strange errors ld.<br/>
The bug is not Easy Framework but is caused by a bad compilation of general gcc.<br/>
In some systems, such as in xubuntu 13.04 armhf various symbols like stat they are not 
exported correctly, and this causes errors during the link libraries.<br/>
How to fix :<br/>
Open file "generate"<br/>"
go to "declare -A libaex=(" at line 91<br/>
go to '[easyconsole]=""' at line 92<br/>
replace with '[easyconsole]="/usr/lib/gcc/arm-linux-gnueabihf/4.8/libgcc.a"'<br/>
or your directory where is ubicate libgcc.a<br/>
go to '[easymath]=""' at line 94<br/>
replace with '[easymath]="/usr/lib/gcc/arm-linux-gnueabihf/4.8/libgcc.a"'<br/>
or your directory where is ubicate libgcc.a<br/>
go to "declare -A libsoex=(" at line 78<br/>
go to '[easyfile]=""' at line 79<br/>
replace with '[easyfile]="-lc"'<br/>
$ ./generate<br/>
$ sudo ./install<br/>

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
