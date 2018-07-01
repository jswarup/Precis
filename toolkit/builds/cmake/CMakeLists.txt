#---------------------------------------- CMakeLists.txt --------------------------------------------------------------------

cmake_minimum_required(VERSION 3.5)


#options [ be sure to remove CMakeCache.txt after editing these lines ]
OPTION(PR_DBG "Build Library with Malloc Debug option" ON)
OPTION(PR_MALLOCDBG "Build Library with Malloc Debug option" OFF)  
OPTION(PR_TRACEDBG "enable tracing" OFF) 
#------------------------------------------------------------------------------------------------------------------------------

# Workspace name
project(precis)

set(CMAKE_VERBOSE_MAKEFILE ON)


#source path
set(ROOT_PATH "${CMAKE_SOURCE_DIR}/../../../")  
set(SOURCE_PATH "${ROOT_PATH}src/")
set(BUILD_PATH "${ROOT_PATH}delivery/")


# Set default locations
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${BUILD_PATH}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${BUILD_PATH}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${BUILD_PATH}/lib)
 
#------------------------------------------------------------------------------------------------------------------------------
  
include_directories( 
    ${SOURCE_PATH} 
)

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

include_directories(${CMAKE_LIBRARY_OUTPUT_DIRECTORY})
list(APPEND SOURCES "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/PR_version.h")

#------------------------------------------------------------------------------------------------------------------------------

# Compiler option 

if(PR_DBG)
    set(CONFIGURATION_NAME "Debug")
    add_definitions(-DPR_DBG)
    add_definitions(-g) 
#    add_definitions(-Wall)
    SET( CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} -g")
else()
    set(CONFIGURATION_NAME "Release")
#	add_definitions(-O)  
    SET( CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} -O3")
endif()

#add_definitions(-fpermissive)
add_definitions(-std=c++1z)
add_definitions(-Wno-literal-suffix)
add_definitions(-Wno-unused-but-set-variable)
add_definitions(-Wno-unused-variable)
add_definitions(-Wno-unused-value)
 
#------------------------------------------------------------------------------------------------------------------------------
# Define the CXX sources
set ( CXX_SRCS  
)

set ( CXX_SRCS ${CXX_SRCS}   
	  
    
    ${SOURCE_PATH}cove/barn/cv_include.cpp
    ${SOURCE_PATH}cove/barn/cv_aid.cpp 
    ${SOURCE_PATH}cove/snip/cv_typerep.cpp 
    ${SOURCE_PATH}cove/snip/cv_cmdexec.cpp 

    ${SOURCE_PATH}segue/tenor/sg_include.cpp
    ${SOURCE_PATH}segue/tenor/sg_cmdtimbre.cpp
    ${SOURCE_PATH}segue/tenor/sg_cmdxml.cpp 
)

if(ES_TRACEDBG)
set ( CXX_SRCS ${CXX_SRCS}  
)
endif()
 

set(PrecisSourceFiles ${SOURCE_PATH}/console/pr_console.cpp) 

# Library path
set(CMAKE_LDFLAGS "${CMAKE_LDFLAGS} -L. ")
set(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} -pthread")

add_executable(precis ${PrecisSourceFiles} ${CXX_SRCS} )

#------------------------------------------------------------------------------------------------------------------------------ 


#------------------------------------------------------------------------------------------------------------------------------


 
