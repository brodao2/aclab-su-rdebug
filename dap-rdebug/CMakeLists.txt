# CMakeList.txt: projeto de CMake para dap-rdebug
#
cmake_minimum_required (VERSION 3.8)

# Habilite a Recarga Dinâmica para compiladores MSVC, se houver suporte.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project (dap-rdebug 
	VERSION 0.0.1 
	DESCRIPTION "AC Lab DAP-RDebug protocol tunnel"
    HOMEPAGE_URL https://github.com/brodao2/aclab-su-rdebug
)

# Adicione a origem ao executável deste projeto.
add_executable (dap-rdebug 	
	"third_party/argparse/argparse.h" 
	"src/event.cpp"
	"src/dap-rdebug.cpp"
	src/logger.cpp
	src/config.cpp
	"src/msg-initialize-request.cpp"
	"src/msg-disconnect-request.cpp")

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
