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
	${autojson_SOURCE_DIR}/src/JSON_checker.c
)

# gcc specific
if(CMAKE_COMPILER_IS_GNUCXX)
        list(APPEND CMAKE_CXX_FLAGS "-std=c++0x")
endif(CMAKE_COMPILER_IS_GNUCXX)

if (GTEST_FOUND)
	include_directories(${GTEST_INCLUDE_DIRS})
	add_executable(unittest
		${autojson_SOURCE_DIR}/test/JsonTest.cc
	)
	target_link_libraries(unittest autojson ${GTEST_BOTH_LIBRARIES})
endif (GTEST_FOUND)

# build documentation by doxygen
if (DOXYGEN_FOUND)
#	configure_file(
#		${webwrite2_SOURCE_DIR}/doc/Doxyfile.in
#		${webwrite2_BINARY_DIR}/Doxyfile @ONLY)

#	add_custom_target(doc
#		${DOXYGEN_EXECUTABLE} ${webwrite2_BINARY_DIR}/Doxyfile
#		WORKING_DIRECTORY ${webwrite2_BINARY_DIR}
#		COMMENT "Generating API documentation with Doxygen" VERBATIM)
		
endif (DOXYGEN_FOUND)
