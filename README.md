<p align="center"><img width="210" src="https://github.com/Water-Melon/Melang/blob/master/docs/logo.png?raw=true" alt="Melang logo"></p>
<p align="center"><img src="https://img.shields.io/github/license/Water-Melon/Melang" /></p>
<h1 align="center">Melang</h1>




Melang is a script Language of time-sharing scheduling coroutine in single thread or multi-thread. It is only support on UNIX/Linux.



### Installation

If you want to install Melang on Windows, please download `mingw` and `git bash` at first.

Please install [Melon](https://github.com/Water-Melon/Melon) at first. It is the core library that Melang depends on.

Then execute these following shell commands:

```shell
$ git clone https://github.com/Water-Melon/Melang.git
$ cd Melang
$ ./configure [some options, --help will list them all]
#Then make
$ make #only install melang elf
#or
$ make lib #only install dynamic libraries
#or
$ make all #install melang and dynamic libraries
#make install
$ make install
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

