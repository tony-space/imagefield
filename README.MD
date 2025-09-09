# Image field

An experimental, cross-platform, open-source image processing framework inspired by Apple's `Core Image`.

## Project Goal

[Apple's `Core Image` framework](https://developer.apple.com/library/archive/documentation/GraphicsImaging/Conceptual/CoreImaging/ci_tasks/ci_tasks.html#//apple_ref/doc/uid/TP30001185-CH3-TPXREF101)
is a powerful tool for hardware-accelerated image processing, but it is exclusive to Apple platforms.
This project aims to provide a similar framework for multiple platforms, including Windows, Linux, and Android.

The main goal is not to create a one-to-one reimplementation of `Core Image`, which would present significant legal and technical challenges.
Instead, the goal is to provide an alternative cross-platform solution with comparable visual fidelity and performance.

## Design Rationale

Similar to the original `Core Image` framework, the computational model is based on a directed acyclic graph (DAG) of filters.

Each graph node represents an operation (typically an image filter) that can have multiple inputs and outputs. A graph node is purely declarative and does not perform any computation by itself.

Unlike `Core Image`, this framework requires the graph to be explicitly compiled before use. Compilation translates the declarative graph into a sequence of executable operations for a target backend, such as a CPU or GPU.

## Why "Image field"?

In digital imaging, there are two primary approaches to representing images.

Frameworks like `TensorFlow` and `MNN` treat images as multidimensional **discrete grids** of pixels, known as tensors. In contrast, `Core Image` treats images as **continuous fields** of color defined over a 2D plane. Both approaches offer unique opportunities for optimization and have distinct trade-offs.

This project adopts and extends the concept of images as **fields**, which inspired the name `Image field`.

The field-based approach provides several advantages over tensor-based methods:
*   Fields can be transformed (e.g., translated, rotated, scaled) without requiring explicit resampling or interpolation.
*   Images can be defined over arbitrary regions, not just rectangles (see the `Region` concept).
*   Images can be defined over infinite areas with an arbitrary level of detail (e.g., procedural noise, fractals).
*   A field can be converted to a discrete tensor at any time, as fields are continuous by nature.
*   Traditional GPU rasterization techniques are highly efficient for rendering fields thanks to mip-mapping, anisotropic filtering, and MSAA.

## Current Status

The project is in the early stages of development and currently supports a limited set of filters and operations.

Available features:
*   `GraphNode` concept for building backend-agnostic graphs.
*   `Image` concept for representing backend-agnostic images.
*   `Region` concept to represent an image's Domain of Definition (DOD). `Core Image` uses a similar concept to define the area outside of which the image is considered black and transparent.
*   `NaiveGraphCompiler` – a backend-agnostic graph compiler that produces a sequence of operations for a backend.
*   `CpuRuntime` – a CPU backend that provides graph compiler and executable low-level operations.
*   `Rasterizer` – a CPU-based MSAA x4 rasterizer for `Region` shapes.

## Prerequisites to build

### Boost libraries
The recommended way to obtain the Boost libraries is to download a pre-packaged archive from GitHub:

*   Go to https://github.com/boostorg/boost/releases
*   Download the latest version with a `-cmake` suffix in the name (e.g., `boost-1.89.0-cmake.7z`).
*   Unpack the archive to a directory (e.g., `C:\boost\boost-1.89.0` or `~/boost/boost-1.89.0`).

Alternatively, you can clone the repository, though this takes more time. The following command creates a shallow clone of a specific version:

~~~
git clone https://github.com/boostorg/boost.git boost/boost-1.89.0 --branch boost-1.89.0 --single-branch --recurse-submodules --filter=tree:0 --also-filter-submodules --jobs 8
~~~

After cloning or extracting the archive, set the environment variable `BOOST_ROOT` to the path where the Boost Libraries are located.

### Android NDK
Android NDK support is planned for the future.
Currently, the framework can be built for Android, but there are no Android-specific features yet.

To build for Android, follow these steps:
* Install Android Studio, then install the NDK via the SDK Manager.
* Set the environment variable `NDK_LOCATION` to the path of the NDK directory, for example,
`C:\Users\<username>\AppData\Local\Android\Sdk\ndk\<version>`
* Run `cmake\compile-ndk-clang.bat`

