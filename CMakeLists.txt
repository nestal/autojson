#
#	autojson: A JSON parser base on the automaton provided by json.org
#	Copyright (C) 2014  Wan Wai Ho

#	This program is free software; you can redistribute it and/or
#	modify it under the terms of the GNU General Public License
#	as published by the Free Software Foundation version 2
#	of the License.
#
#	This program is distributed in the hope that it will be useful,
#	but WITHOUT ANY WARRANTY; without even the implied warranty of
#	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#	GNU General Public License for more details.
#
#	You should have received a copy of the GNU General Public License
#	along with this program; if not, write to the Free Software
#	Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
#

project(autojson)
cmake_minimum_required(VERSION 2.8)

find_package(Doxygen)
find_package(GTest)

include_directories(${autojson_SOURCE_DIR}/src)

add_library(autojson
	src/JSON_checker.h
	src/JSON_checker.c
	src/JsonParser.hh
	src/JsonParser.cc
	src/LexicalCast.hh
	src/LexicalCast.cc
	src/Exception.hh
	src/Exception.cc
	src/Key.hh
	src/Key.cc
	src/Cursor.hh
	src/Cursor.cc
	src/JVar.hh
	src/JVar.cc
	src/Type.hh
	src/JsonProcessor.hh
	src/TypeBuilder.hh
	src/VectorBuilder.hh
)

include(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)
CHECK_CXX_COMPILER_FLAG("-std=c++0x" COMPILER_SUPPORTS_CXX0X)
if(COMPILER_SUPPORTS_CXX11)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
elseif(COMPILER_SUPPORTS_CXX0X)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
endif()

if (GTEST_FOUND)
	add_definitions(
		-DTEST_DATA="${autojson_SOURCE_DIR}/test/data/"
	)

	include_directories(${GTEST_INCLUDE_DIRS})
	add_executable(unittest
		test/JsonTest.cc
		test/ParserTest.cc
		test/ArrayTest.cc
		test/CursorTest.cc
		test/JVarTest.cc
		test/LexicalCastTest.cc
	)
	target_link_libraries(unittest autojson ${GTEST_BOTH_LIBRARIES})
endif (GTEST_FOUND)

# build documentation by doxygen
if (DOXYGEN_FOUND)
	configure_file(
		${autojson_SOURCE_DIR}/doc/Doxyfile.in
		${autojson_BINARY_DIR}/Doxyfile @ONLY)

	add_custom_target(doc
		${DOXYGEN_EXECUTABLE} ${autojson_BINARY_DIR}/Doxyfile
		WORKING_DIRECTORY ${autojson_BINARY_DIR}
		COMMENT "Generating API documentation with Doxygen" VERBATIM)
		
endif (DOXYGEN_FOUND)
