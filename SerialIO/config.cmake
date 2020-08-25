if("${CMAKE_SYSTEM_NAME}" STREQUAL "Windows")
	message("-- Inlcude PosixWin32")
else()
	message("-- Exlcude PosixWin32")
	list(APPEND EXCLUDES ".*/PosixWin32/.*")
	add_link_options(-lpthread)
endif()

