#---------------------------------------- CMakeLists.txt --------------------------------------------------------------------

cmake_minimum_required(VERSION 3.0)


set(CMAKE_VERBOSE_MAKEFILE ON)

#------------------------------------------------------------------------------------------------------------------------------
#options [ be sure to remove CMakeCache.txt after editing these lines ]
OPTION(PR_DBG "Build Library with Malloc Debug option" ON)
OPTION(PR_MALLOCDBG "Build Library with Malloc Debug option" OFF)  
OPTION(PR_TRACEDBG "enable tracing" OFF) 
OPTION(PR_VSCODE "Build Library with Malloc Debug option" ON)
 
#------------------------------------------------------------------------------------------------------------------------------

#source path
set(ROOT_PATH "${CMAKE_SOURCE_DIR}/../../../")  
set(SOURCE_PATH "${ROOT_PATH}src/")
set(BUILD_PATH "${ROOT_PATH}delivery/")
set(CMAKE_BUILD_DIRECTORY "${ROOT_PATH}delivery/")


# Set default locations
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${BUILD_PATH}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${BUILD_PATH}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${BUILD_PATH}/lib)
 
#------------------------------------------------------------------------------------------------------------------------------
  
include_directories( 
    ${SOURCE_PATH} 
)

#------------------------------------------------------------------------------------------------------------------------------ 

if(PR_VSCODE)

if(PR_DBG)
add_definitions(-DEBUG)
endif()

add_definitions(-std:c++latest)
endif()

#------------------------------------------------------------------------------------------------------------------------------ 
# the commit's SHA1, and whether the building workspace was dirty or not

EXECUTE_PROCESS( 
  COMMAND /bin/rm "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/cv_version.h" 
) 

execute_process(COMMAND
  "git" describe --match=NeVeRmAtCh --always --abbrev=40 --dirty
  WORKING_DIRECTORY "${ROOT_PATH}"
  OUTPUT_VARIABLE   GIT_SHA1
  ERROR_QUIET OUTPUT_STRIP_TRAILING_WHITESPACE) 
 
configure_file("${SOURCE_PATH}cove/barn/cv_version.h.in" "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/cv_version.h" @ONLY) 

#------------------------------------------------------------------------------------------------------------------------------ 

include_directories(${CMAKE_LIBRARY_OUTPUT_DIRECTORY})
list(APPEND SOURCES "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/cv_version.h")

#------------------------------------------------------------------------------------------------------------------------------
 

set ( TR_COVEFILES 
    ${SOURCE_PATH}/cove/barn/cv_include.cpp
    ${SOURCE_PATH}/cove/barn/cv_aid.cpp 
    ${SOURCE_PATH}/cove/snip/cv_cmdexec.cpp
    ${SOURCE_PATH}/cove/snip/cv_typerep.cpp
    ${SOURCE_PATH}/segue/tenor/sg_cmdtimbre.cpp
    ${SOURCE_PATH}/segue/tenor/sg_cmdxml.cpp
)

add_library( cove STATIC ${TR_COVEFILES} )

set(PrecisSourceFiles ${SOURCE_PATH}/console/pr_console.cpp) 
add_executable(precis  ${PrecisSourceFiles} ${CXX_SRCS} )

target_link_libraries( precis cove )  

project(precis VERSION 0.1.0)

#------------------------------------------------------------------------------------------------------------------------------


 