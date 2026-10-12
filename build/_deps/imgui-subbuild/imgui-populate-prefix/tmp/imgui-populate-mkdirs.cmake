# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file LICENSE.rst or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION ${CMAKE_VERSION}) # this file comes with cmake

# If CMAKE_DISABLE_SOURCE_CHANGES is set to true and the source directory is an
# existing directory in our source tree, calling file(MAKE_DIRECTORY) on it
# would cause a fatal error, even though it would be a no-op.
if(NOT EXISTS "C:/Users/Anne Patricio/Desktop/Files/DLSU/Year 3/Term 1/CSOPESY/CSOPESY_MO4/build/_deps/imgui-src")
  file(MAKE_DIRECTORY "C:/Users/Anne Patricio/Desktop/Files/DLSU/Year 3/Term 1/CSOPESY/CSOPESY_MO4/build/_deps/imgui-src")
endif()
file(MAKE_DIRECTORY
  "C:/Users/Anne Patricio/Desktop/Files/DLSU/Year 3/Term 1/CSOPESY/CSOPESY_MO4/build/_deps/imgui-build"
  "C:/Users/Anne Patricio/Desktop/Files/DLSU/Year 3/Term 1/CSOPESY/CSOPESY_MO4/build/_deps/imgui-subbuild/imgui-populate-prefix"
  "C:/Users/Anne Patricio/Desktop/Files/DLSU/Year 3/Term 1/CSOPESY/CSOPESY_MO4/build/_deps/imgui-subbuild/imgui-populate-prefix/tmp"
  "C:/Users/Anne Patricio/Desktop/Files/DLSU/Year 3/Term 1/CSOPESY/CSOPESY_MO4/build/_deps/imgui-subbuild/imgui-populate-prefix/src/imgui-populate-stamp"
  "C:/Users/Anne Patricio/Desktop/Files/DLSU/Year 3/Term 1/CSOPESY/CSOPESY_MO4/build/_deps/imgui-subbuild/imgui-populate-prefix/src"
  "C:/Users/Anne Patricio/Desktop/Files/DLSU/Year 3/Term 1/CSOPESY/CSOPESY_MO4/build/_deps/imgui-subbuild/imgui-populate-prefix/src/imgui-populate-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "C:/Users/Anne Patricio/Desktop/Files/DLSU/Year 3/Term 1/CSOPESY/CSOPESY_MO4/build/_deps/imgui-subbuild/imgui-populate-prefix/src/imgui-populate-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "C:/Users/Anne Patricio/Desktop/Files/DLSU/Year 3/Term 1/CSOPESY/CSOPESY_MO4/build/_deps/imgui-subbuild/imgui-populate-prefix/src/imgui-populate-stamp${cfgdir}") # cfgdir has leading slash
endif()
