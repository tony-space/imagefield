# Image field

An experimental image processing framework.

## Prerequisites

### Boost libraries
The recommended way of obtaining the Boost Libraries is to download an archive from GitHub:

* Go to https://github.com/boostorg/boost/releases
* Download the latest version with `-cmake` preffix in the name, e.g. `boost-1.88.0-cmake.7z`
* Unpack the archive to a directory, e.g. `C:\boost\boost-1.88.0` / `~/boost/boost-1.88.0`

`git clone` way is also possible, but it takes more time. For example, you can clone a shallow copy of the repo with the following command:

```
git clone https://github.com/boostorg/boost.git boost/boost-1.88.0 --branch boost-1.88.0 --single-branch --recurse-submodules --filter=tree:0 --also-filter-submodules --jobs 8
```

After cloning / extracting the archive, add environment variable `BOOST_ROOT` with the path to the directory where the Boost Libraries are located.

### Android NDK
TODO: mention the following environment variables:

* NDK_LOCATION
