# Lua Task Engine

## Dependencies

* Lua
* Poco
* JsonCpp

## Build Lua

```bash
wget http://www.lua.org/ftp/lua-5.1.5.tar.gz
tar zxvf lua-5.1.5.tar.gz
cd lua-5.1.5
make linux
make install INSTALL_TOP=/usr/local/lua-5.1.5
```

## Build Poco

```bash
weget https://github.com/pocoproject/poco/archive/poco-1.10.1-release.tar.gz
tar zxvf poco-poco-1.10.1-release.tar.gz
cd poco-poco-1.10.1-release
mkdir cmake-build
cd cmake-build
cmake .. -DBUILD_SHARED_LIBS=OFF -DCMAKE_INSTALL_PREFIX=/usr/local/poco-1.10.1
cmake --build . --target install
```

## Build LTA

```bash
cd src
make
```
