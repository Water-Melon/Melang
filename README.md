<p align="center"><img width="100" src="https://github.com/Water-Melon/Melang/blob/master/docs/logo.png?raw=true" alt="Melang logo"></p>
<p align="center"><img src="https://img.shields.io/github/license/Water-Melon/Melang" /></p>
<h1 align="center">Melang</h1>




Melang is a script Language of preemptive scheduling coroutine in single thread. It is only support on UNIX/Linux.

> Melang has already supported MySQL 8.0, but the newest MySQL C client library is unstable. So if trying to connect an unreachable address, a buffer-overflow will happened even though program may not crash. I have submitted bug report to MySQL community, no more news now.
>



### Installation

At first, you should confirm that your device can visit GitHub. Then execute these shell commands below:

```
git clone https://github.com/Water-Melon/Melang.git
cd Melang
sudo ./install [--prefix=INSTALL_PATH] [--libprefix=Lib_INSTALL_PATH]
```

*prefix* is the path where melang will be placed after shell *install* executed. Default is */usr/bin/*.

*libprefix* is the install-path of library that melang relied on. The library will be installed at */usr/local/melon_for_melang* by default.

After installed, you can execute these commands below:

```
melang -v //show version
melang -h //help information
melang a.mln b.mln ... //execute melang files
```



### License

[BSD-3-Clause License](https://github.com/Water-Melon/Melang/blob/master/LICENSE)

Copyright (c) 2018-present, Niklaus F. Schen



### Document

Please visit [Pages](https://water-melon.github.io/Melang/) to get more information.
