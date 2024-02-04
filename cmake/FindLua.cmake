mmo_set_extra_dirs_lib(LUA lua)
find_file(LUA_LIBRARY lua51.lib
        HINTS ${HINTS_LUA_LIBDIR}
        PATHS ${PATHS_LUA_LIBDIR}
        ${CROSSCOMPILING_NO_CMAKE_SYSTEM_PATH}
        )

mmo_set_extra_dirs_lib(LUAJIT lua)
find_file(LUAJIT_LIBRARY luajit.lib
        HINTS ${HINTS_LUAJIT_LIBDIR}
        PATHS ${PATHS_LUAJIT_LIBDIR}
        ${CROSSCOMPILING_NO_CMAKE_SYSTEM_PATH}
)

mmo_set_extra_dirs_include(LUA lua "${LUA_LIBRARY}")
find_path(LUA_INCLUDEDIR lua/lua.h
        HINTS ${HINTS_LUA_INCLUDEDIR}
        PATHS ${PATHS_LUA_INCLUDEDIR}
        ${CROSSCOMPILING_NO_CMAKE_SYSTEM_PATH}
        )

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Lua DEFAULT_MSG LUA_INCLUDEDIR LUA_LIBRARY)
mark_as_advanced(LUA_INCLUDEDIR LUA_LIBRARY)

if(LUA_FOUND)
    is_bundled(LUA_BUNDLED "${LUA_LIBRARY}")
    set(LUA_LIBRARIES ${LUA_LIBRARY} ${LUAJIT_LIBRARY})
    set(LUA_INCLUDE_DIRS ${LUA_INCLUDEDIR})

    set(LUA_COPY_FILES)
    if(LUA_BUNDLED AND TARGET_OS STREQUAL "windows")
        set(LUA_COPY_FILES "${EXTRA_LUA_LIBDIR}/lua51.dll")
    endif()
endif()
