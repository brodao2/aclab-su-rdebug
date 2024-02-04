# CMakeList.txt: projeto de CMake para dap-rdebug, incluir origem e definir
# lógica específica a um projeto aqui.
#
cmake_minimum_required (VERSION 3.8)

# Habilite a Recarga Dinâmica para compiladores MSVC, se houver suporte.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project ("dap-rdebug")

# Adicione a origem ao executável deste projeto.
add_executable (dap-rdebug 
	"dap-rdebug.cpp" 
	"logger.cpp"
	"config.cpp"
)

set(CPPDAP_DIR ${CMAKE_CURRENT_SOURCE_DIR}/third_party/cppdap) 
add_subdirectory(${CPPDAP_DIR})

set(SPDLOG_DIR ${CMAKE_CURRENT_SOURCE_DIR}/third_party/spdlog) 
add_subdirectory(${SPDLOG_DIR})

#add_subdirectory(extern/cxxopts) 
#target_include_directories(${TARGET_NAME} PUBLIC cxxopts) 
#target_link_libraries(${TARGET_NAME} PUBLIC cxxopts),
set(CXXOPTS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/third_party/cxxopts) 
add_subdirectory(${CXXOPTS_DIR})

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET dap-rdebug PROPERTY CXX_STANDARD 20)
endif()

target_link_libraries(dap-rdebug cppdap spdlog::spdlog_header_only) # replace <target> with the name of your project's target
# TODO: Adicione testes e instale destinos, se necessário.

