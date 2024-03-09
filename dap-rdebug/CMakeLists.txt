# Copyright 2024 Alan Cândido (brodao@gmail.com)
# 
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
# 
#   http: //www.apache.org/licenses/LICENSE-2.0
# 
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

# CMakeList.txt: projeto de CMake para dap-rdebug
#
cmake_minimum_required (VERSION 3.8)

# Habilite a Recarga Dinâmica para compiladores MSVC, se houver suporte.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

if (LINUX)
	set( CMAKE_CXX_COMPILER "/pathto/g++" )
	set( CMAKE_C_COMPILER "/pathto/gcc" )
endif()

project (dap-rdebug 
	VERSION 0.0.1 
	DESCRIPTION "AC Lab DAP-RDebug protocol tunnel"
    HOMEPAGE_URL https://github.com/brodao2/aclab-su-rdebug
)

# Adicione a origem ao executável deste projeto.

add_executable (dap-rdebug 	
	"third_party/argparse/argparse.h" 
	"include/event.h"
	"include/dap-rdebug.h"
	"include/logger.h"
	"include/config.h"
	"include/msg-abstract-request.h"
	"include/msg-breakpoints-locations-request.h"
	"include/msg-disconnect-request.h"
	"include/msg-initialize-request.h"   
	"include/msg-attach-request.h"   
	"include/msg-launch-request.h"   
	"include/rdebug-client.h"
	
	"src/event.cpp"
	"src/dap-rdebug.cpp"
	"src/logger.cpp"
	"src/config.cpp"
	"src/rdebug-client.cpp"
	"src/msg-attach-request.cpp"
	"src/msg-launch-request.cpp"
	"src/msg-initialize-request.cpp"
	"src/msg-disconnect-request.cpp"
	"src/msg-breakpoints-locations-request.cpp" 
)

target_include_directories(dap-rdebug 
	PRIVATE include
)

set(CPPDAP_DIR ${CMAKE_CURRENT_SOURCE_DIR}/third_party/cppdap) 
add_subdirectory(${CPPDAP_DIR})

set(SPDLOG_DIR ${CMAKE_CURRENT_SOURCE_DIR}/third_party/spdlog) 
add_subdirectory(${SPDLOG_DIR})

set(ARGPARSE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/third_party/argparse) 
#add_subdirectory(${ARGPARSE_DIR})
target_include_directories(dap-rdebug 
	PRIVATE ${ARGPARSE_DIR}
)

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET dap-rdebug PROPERTY CXX_STANDARD 20)
endif()

target_link_libraries(
	dap-rdebug 
	cppdap 
	spdlog::spdlog_header_only
) 
	
# TODO: Adicione testes e instale destinos, se necessário.
