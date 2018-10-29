set(PYLON_ROOT_DIR $ENV{PYLON_DEV_DIR})
        if (NOT DEFINED ENV{PYLON_DEV_DIR})
            set(PYLON_ROOTDIR "C:\Program Files\Basler\pylon 5\Development")
        endif()

        set(_PYLON_CONFIG "${PYLON_ROOT}/bin/pylon-config")
        if (EXISTS "${_PYLON_CONFIG}")
            set(Pylon_FOUND TRUE)
            execute_process(COMMAND ${_PYLON_CONFIG} --cflags-only-I OUTPUT_VARIABLE HEADERS_OUT)
            execute_process(COMMAND ${_PYLON_CONFIG} --libs-only-l OUTPUT_VARIABLE LIBS_OUT)
            execute_process(COMMAND ${_PYLON_CONFIG} --libs-only-L OUTPUT_VARIABLE LIBDIRS_OUT)
            string(REPLACE " " ";" HEADERS_OUT "${HEADERS_OUT}")
            string(REPLACE "-I" "" HEADERS_OUT "${HEADERS_OUT}")
            string(REPLACE "\n" "" Pylon_INCLUDE_DIRS "${HEADERS_OUT}")

            string(REPLACE " " ";" LIBS_OUT "${LIBS_OUT}")
            string(REPLACE "-l" "" LIBS_OUT "${LIBS_OUT}")
            string(REPLACE "\n" "" Pylon_LIBRARIES "${LIBS_OUT}")

            string(REPLACE " " ";" LIBDIRS_OUT "${LIBDIRS_OUT}")
            string(REPLACE "-L" "" LIBDIRS_OUT "${LIBDIRS_OUT}")
            string(REPLACE "\n" "" LIBDIRS_OUT "${LIBDIRS_OUT}")

            set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
            foreach (LIBDIR ${LIBDIRS_OUT})
                link_directories(${LIBDIR})
            endforeach()
        else()
            set(Pylon_FOUND FALSE)
        endif()


SET(Pylon_INCDIR_SUFFIXES
  include
  )

SET(Pylon_INCLUDE_DIR, $(PYLON_ROOT_DIR)/include)  

# library linkdir suffixes appended to PYLON_ROOT_DIR 
SET(Pylon_LIBDIR_SUFFIXES
  lib
  lib/x64
  )

FIND_PATH(Pylon_INCLUDE_DIR
    NAMES PylonIncludes.h      
    PATHS ${PYLON_ROOT_DIR} 
    PATH_SUFFIXES ${Pylon_INCDIR_SUFFIXES})

FIND_LIBRARY(Pylon_BASE_LIBRARY  
  NAMES PylonBase_v5_1
  PATHS ${PYLON_ROOT_DIR}  PATH_SUFFIXES ${Pylon_LIBDIR_SUFFIXES})

FIND_LIBRARY(Pylon_UTILITY_LIBRARY  
  NAMES PylonUtility_v5_1
  PATHS ${PYLON_ROOT_DIR}  PATH_SUFFIXES ${Pylon_LIBDIR_SUFFIXES})

FIND_LIBRARY(Pylon_GCBASE_LIBRARY  
  NAMES GCBase_MD_VC141_v3_1_Basler_pylon_v5_1
  PATHS ${PYLON_ROOT_DIR}  PATH_SUFFIXES ${Pylon_LIBDIR_SUFFIXES})

  # Logic selecting required libs and headers
SET(Pylon_FOUND ON)
FOREACH(NAME ${Pylon_FIND_REQUIRED_COMPONENTS})
  # only good if header and library both found   
  IF(Pylon_${NAME}_LIBRARY)
    message("Found library: ${Pylon_${NAME}_LIBRARY}")
    LIST(APPEND Pylon_LIBRARIES    "${Pylon_${NAME}_LIBRARY}")
  ELSE(Pylon_${NAME}_LIBRARY)
    message("Could not find Pylon_${NAME}_INCLUDE_DIR")
    SET(Pylon_FOUND OFF)
  ENDIF(Pylon_${NAME}_LIBRARY)
ENDFOREACH(NAME)

message("Include dir: ${Pylon_FOUND}")

# get the link directory for rpath to be used with LINK_DIRECTORIES: 
IF(Pylon_LIBRARY)
  GET_FILENAME_COMPONENT(Pylon_LINK_DIRECTORIES ${Pylon_LIBRARY} PATH)
ENDIF(Pylon_LIBRARY)

MARK_AS_ADVANCED(
    Pylon_BASE_LIBRARY
    Pylon_BASE_LIBRARY
    Pylon_GCBASE_LIBRARY
)

# display help message
IF(NOT Pylon_FOUND)
  # make FIND_PACKAGE friendly
  IF(Pylon_FIND_REQUIRED)
    MESSAGE(FATAL_ERROR
            "Pylon required but some headers or libs not found. Please specify it's location with Pylon_ROOT_DIR env. variable.")
  ELSE(Pylon_FIND_REQUIRED)
    MESSAGE(STATUS 
            "ERROR: Pylon was not found.")
  ENDIF(Pylon_FIND_REQUIRED)
ELSE(NOT Pylon_FOUND)
  MESSAGE(STATUS
          "Pylon found at ${PYLON_ROOT_DIR}")
    message("library dirs: ${Pylon_LIBRARIES}")
ENDIF(NOT Pylon_FOUND)
