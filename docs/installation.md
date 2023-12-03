## Installation



Melang relies on the core C library [Melon](https://github.com/Water-Melon/Melon), so it needs to be installed first. We can use the following commands to quickly install Melon:

```bash
git clone https://github.com/Water-Melon/Melon.git
cd Melon
./configure
make
sudo make install
```

If you are a Windows user, please refer to [Melon installation](http://doc.melonc.io/en/install.html) for more details.



Then, we can execute the following commands to install Melang:

```bash
git clone https://github.com/Water-Melon/Melang.git
cd Melang
./configure
make all
sudo make install
```

The `configure` command has the following parameters:

- `--help` Output the help information.
- `--prefix` The installation path for Melang.
- `--melon-prefix` The installation path for the core library Melon.
- `--lib-prefix` The installation path for the dynamic libraries used by the Melang.
- `--enable-mysql` Enable the MySQL feature, which means compiling the MySQL dynamic library used by Melang.
- `--mysql_header_path` The header files path for libmysqlclient (If they cannot be found in `/usr/include` or any other default header loading path).
- `--mysql_lib_path` The library path for libmysqlclient (If they cannot be found in `/usr/lib` or any other default library loading path.).
- `--enable-wasm` Compile into a wasm format file.
- `--debug` Enable debug mode, which is used for developing and debugging Melang.



Now you can use Melang for development.

### Example

Here is a Melang script file.

```
//example.m

sys = Import('sys');
sys.print("Hello World!");
```

Then execute

```bash
melang example.m
```

The output is

```
Hello World!
```

