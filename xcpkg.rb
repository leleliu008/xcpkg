class Xcpkg < Formula
  desc     "A package builder/manager for Xcode to build projects written in C, C++, Rust, Zig, Go, Haskell, etc"
  homepage "https://github.com/leleliu008/xcpkg"
  head     "https://github.com/leleliu008/xcpkg.git", branch: "master"
  url      "https://github.com/leleliu008/xcpkg.git", revision: "1ada5fb8d944433eda3ca79572f7dcb72cff771f"
  version  "0.1000.1"
  license  "Apache-2.0"

  depends_on "cmake" => :build
  depends_on "ninja" => :build
  depends_on "pkg-config" => :build

  depends_on "zlib"
  depends_on "curl"
  depends_on "openssl@3"
  depends_on "jansson"
  depends_on "libyaml"
  depends_on "libgit2"
  depends_on "libarchive"

  def install
    system "cmake", "-S", "c", "-B", "build", *std_cmake_args
    system "cmake", "--build",   "build"
    system "cmake", "--install", "build"
  end

  test do
    system "#{bin}/xcpkg", "--help"
  end
end
