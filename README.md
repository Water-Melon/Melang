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

```bash
melang -v //show version
melang -h //help information
melang a.mln b.mln ... //execute melang files
```



### Docker Image

If you need to quickly start a Melang runtime environment, you can use this Docker image.

```bash
docker pull melonc/melon
```



### **Example**

```
//example.m

sys = Import('sys');
sys.print("Hello World!");
```
```
melang example.m
```
The output is:
```
Hello World!
```



### Applications

[Meproc](https://github.com/MelonCTech/Meproc): a cross-platform process management and supervision service.



### License

[BSD-3-Clause License](https://github.com/Water-Melon/Melang/blob/master/LICENSE)

Copyright (c) 2018-present, Niklaus F. Schen



### Documentation

Please refer to [Official Site](https://melang.org) for more details.
