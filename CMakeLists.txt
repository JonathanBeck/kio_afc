
if (NOT KDE4_FOUND)
	find_package(KDE4 REQUIRED)
	include(KDE4Defaults)
	add_definitions (${QT_DEFINITIONS} ${KDE4_DEFINITIONS})
	include_directories(${KDE4_INCLUDES})
endif (NOT KDE4_FOUND)

SET( CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake ${CMAKE_SOURCE_DIR}/cmake/modules )
FIND_PACKAGE( libimobiledevice REQUIRED )

set(kio_man_PART_SRCS
        kio_afc.cpp
        afcdevice.cpp
        afcpath.cpp)

kde4_add_plugin(kio_afc ${kio_man_PART_SRCS})

target_link_libraries(kio_afc ${KDE4_KIO_LIBS} ${libimobiledevice_LIBRARIES})

install(TARGETS kio_afc  DESTINATION ${PLUGIN_INSTALL_DIR})

install(FILES afc.protocol DESTINATION  ${SERVICES_INSTALL_DIR})
