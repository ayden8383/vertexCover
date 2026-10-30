# CMake generated Testfile for 
# Source directory: C:/Users/Ayden/Documents/vertexCover/VertexCoverProject
# Build directory: C:/Users/Ayden/Documents/vertexCover/VertexCoverProject/build-ablation
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
if(CTEST_CONFIGURATION_TYPE MATCHES "^([Dd][Ee][Bb][Uu][Gg])$")
  add_test([=[VertexCoverSmokeTests]=] "C:/Users/Ayden/Documents/vertexCover/VertexCoverProject/build-ablation/Debug/vertex_cover_tests.exe")
  set_tests_properties([=[VertexCoverSmokeTests]=] PROPERTIES  _BACKTRACE_TRIPLES "C:/Users/Ayden/Documents/vertexCover/VertexCoverProject/CMakeLists.txt;83;add_test;C:/Users/Ayden/Documents/vertexCover/VertexCoverProject/CMakeLists.txt;0;")
elseif(CTEST_CONFIGURATION_TYPE MATCHES "^([Rr][Ee][Ll][Ee][Aa][Ss][Ee])$")
  add_test([=[VertexCoverSmokeTests]=] "C:/Users/Ayden/Documents/vertexCover/VertexCoverProject/build-ablation/Release/vertex_cover_tests.exe")
  set_tests_properties([=[VertexCoverSmokeTests]=] PROPERTIES  _BACKTRACE_TRIPLES "C:/Users/Ayden/Documents/vertexCover/VertexCoverProject/CMakeLists.txt;83;add_test;C:/Users/Ayden/Documents/vertexCover/VertexCoverProject/CMakeLists.txt;0;")
elseif(CTEST_CONFIGURATION_TYPE MATCHES "^([Mm][Ii][Nn][Ss][Ii][Zz][Ee][Rr][Ee][Ll])$")
  add_test([=[VertexCoverSmokeTests]=] "C:/Users/Ayden/Documents/vertexCover/VertexCoverProject/build-ablation/MinSizeRel/vertex_cover_tests.exe")
  set_tests_properties([=[VertexCoverSmokeTests]=] PROPERTIES  _BACKTRACE_TRIPLES "C:/Users/Ayden/Documents/vertexCover/VertexCoverProject/CMakeLists.txt;83;add_test;C:/Users/Ayden/Documents/vertexCover/VertexCoverProject/CMakeLists.txt;0;")
elseif(CTEST_CONFIGURATION_TYPE MATCHES "^([Rr][Ee][Ll][Ww][Ii][Tt][Hh][Dd][Ee][Bb][Ii][Nn][Ff][Oo])$")
  add_test([=[VertexCoverSmokeTests]=] "C:/Users/Ayden/Documents/vertexCover/VertexCoverProject/build-ablation/RelWithDebInfo/vertex_cover_tests.exe")
  set_tests_properties([=[VertexCoverSmokeTests]=] PROPERTIES  _BACKTRACE_TRIPLES "C:/Users/Ayden/Documents/vertexCover/VertexCoverProject/CMakeLists.txt;83;add_test;C:/Users/Ayden/Documents/vertexCover/VertexCoverProject/CMakeLists.txt;0;")
else()
  add_test([=[VertexCoverSmokeTests]=] NOT_AVAILABLE)
endif()
