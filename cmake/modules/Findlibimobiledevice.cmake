 
# - Try to find libimobiledevice
# Once done, this will define
#
#  libimobiledevice_FOUND - system has libimobiledevice
#  libimobiledevice_INCLUDE_DIRS - the libimobiledevice include directories
#  libimobiledevice_LIBRARIES - link these to use libimobiledevice
#
# Copyright (c) 2010, Jonathan Beck, <jonabeck@gmail.com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.


IF (libimobiledevice_INCLUDE_DIR AND libimobiledevice_LIBRARY)
   # in cache already
   SET(gnutls_FIND_QUIETLY TRUE)
ENDIF (libimobiledevice_INCLUDE_DIR AND libimobiledevice_LIBRARY)

IF (NOT WIN32)
   # try using pkg-config to get the directories and then use these values
   # in the FIND_PATH() and FIND_LIBRARY() calls
   # also fills in libimobiledevice_DEFINITIONS, although that isn't normally useful
   FIND_PACKAGE(PkgConfig)
   PKG_CHECK_MODULES(PC_libimobiledevice libimobiledevice-1.0)
   SET(libimobiledevice_DEFINITIONS ${PC_libimobiledevice_CFLAGS_OTHER})
ENDIF (NOT WIN32)

FIND_PATH(libimobiledevice_INCLUDE_DIR libimobiledevice/libimobiledevice.h
   HINTS
   ${PC_libimobiledevice_INCLUDEDIR}
   ${PC_libimobiledevice_INCLUDE_DIRS}
   )

FIND_LIBRARY(libimobiledevice_LIBRARY NAMES imobiledevice libimobiledevice
   HINTS
   ${PC_libimobiledevice_LIBDIR}
   ${PC_libimobiledevice_LIBRARY_DIRS}
   )

MARK_AS_ADVANCED(libimobiledevice_INCLUDE_DIR libimobiledevice_LIBRARY)

# handle the QUIETLY and REQUIRED arguments and set libimobiledevice_FOUND to TRUE if 
# all listed variables are TRUE
INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(libimobiledevice DEFAULT_MSG libimobiledevice_LIBRARY libimobiledevice_INCLUDE_DIR)

SET(libimobiledevice_LIBRARIES    ${libimobiledevice_LIBRARY})
SET(libimobiledevice_INCLUDE_DIRS ${libimobiledevice_INCLUDE_DIR})
