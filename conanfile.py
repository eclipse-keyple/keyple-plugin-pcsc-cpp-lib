# *****************************************************************************
# Copyright (c) 2025 Calypso Networks Association https://calypsonet.org/     *
#                                                                             *
# This program and the accompanying materials are made available under the    *
# terms of the Eclipse Public License 2.0 which is available at               *
# http://www.eclipse.org/legal/epl-2.0                                        *
#                                                                             *
# SPDX-License-Identifier: EPL-2.0                                            *
# *****************************************************************************/

import os

from conan import ConanFile
from conan.tools.cmake import CMake, CMakeDeps, CMakeToolchain, cmake_layout
from conan.tools.files import copy


class KeyplePluginPcscCppLibConan(ConanFile):
    name = "keyple-plugin-pcsc-cpp-lib"
    version = "2.6.3"
    license = "EPL-2.0"
    author = "Calypso Networks Association <https://calypsonet.org/>"
    url = "https://github.com/eclipse-keyple/keyple-plugin-pcsc-cpp-lib"
    homepage = "https://keyple.org/apis/plugin-layer/pcsc-plugin/"
    description = "C++ implementation of the Eclipse Keyple PC/SC plugin"
    topics = ("keyple", "calypso", "pcsc")

    package_type = "library"
    settings = "os", "arch", "compiler", "build_type"

    options = {
        "shared": [True, False],
        "fPIC": [True, False],
        "with_tests": [True, False]
    }

    default_options = {
        "shared": True,
        "fPIC": True,
        "with_tests": False
    }

    exports_sources = [
        "include/*",
        "src/*",
        "CMakeLists.txt",
        "LICENSE",
        "NOTICE.md"
    ]

    def config_options(self):
        if self.settings.os == "Windows":
            self.options.rm_safe("fPIC")

    def configure(self):
        if self.options.shared:
            self.options.rm_safe("fPIC")

    def layout(self):
        cmake_layout(self)

    def requirements(self):
        self.requires("keyple-common-cpp-api/[~2.0.2]", transitive_headers=True)
        self.requires("keyple-plugin-cpp-api/[~2.3.2]", transitive_headers=True)
        self.requires("keyple-util-cpp-lib/[~2.4.1]", transitive_headers=True)

    def generate(self):
        tc = CMakeToolchain(self)
        tc.variables["LIBRARY_TYPE"] = "SHARED" if self.options.shared else "STATIC"
        tc.generate()
        CMakeDeps(self).generate()

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

    def package(self):
        copy(self, "LICENSE", src=self.source_folder, dst=os.path.join(self.package_folder, "licenses"))
        cmake = CMake(self)
        cmake.install()

    def package_info(self):
        self.cpp_info.libs = ["keyplepluginpcsccpplib"]
        self.cpp_info.set_property("cmake_file_name", "keyple-plugin-pcsc-cpp-lib")
        self.cpp_info.set_property("cmake_target_name", "Keyple::Plugin::Pcsc")

        # PC/SC is a system library, not managed by Conan; matches the
        # FIND_LIBRARY() logic in src/main/CMakeLists.txt.
        if self.settings.os == "Linux":
            self.cpp_info.system_libs = ["pcsclite"]
        elif self.settings.os == "Macos":
            self.cpp_info.frameworks = ["PCSC"]
        elif self.settings.os == "Windows":
            self.cpp_info.system_libs = ["winscard"]

    def package_id(self):
        self.info.options.rm_safe("with_tests")
