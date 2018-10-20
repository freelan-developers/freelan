# Common Ambient Variables:
#   CURRENT_BUILDTREES_DIR    = ${VCPKG_ROOT_DIR}\buildtrees\${PORT}
#   CURRENT_PACKAGES_DIR      = ${VCPKG_ROOT_DIR}\packages\${PORT}_${TARGET_TRIPLET}
#   CURRENT_PORT_DIR          = ${VCPKG_ROOT_DIR}\ports\${PORT}
#   PORT                      = current port name (zlib, etc)
#   TARGET_TRIPLET            = current triplet (x86-windows, x64-windows-static, etc)
#   VCPKG_CRT_LINKAGE         = C runtime linkage type (static, dynamic)
#   VCPKG_LIBRARY_LINKAGE     = target library linkage type (static, dynamic)
#   VCPKG_ROOT_DIR            = <C:\path\to\current\vcpkg>
#   VCPKG_TARGET_ARCHITECTURE = target architecture (x64, x86, arm)
#

include(vcpkg_common_functions)
set(SOURCE_PATH ${CURRENT_BUILDTREES_DIR}/src/miniupnp-miniupnpd_2_0/miniupnpc)
vcpkg_download_distfile(ARCHIVE
    URLS "https://github.com/miniupnp/miniupnp/archive/miniupnpd_2_0.zip"
    FILENAME "miniupnpc-2.0.zip"
    SHA512 cbab65f2aee299c710342fdb52c9f3c13d204ff51e73cd7dcb18a445e3255656c12d36a022d2510b15cc9280f6fb41c533a43dc2946fa44c78a6a75f04a506e1
)
vcpkg_extract_source_archive(${ARCHIVE})

if (VCPKG_LIBRARY_LINKAGE STREQUAL dynamic)
vcpkg_configure_cmake(
    SOURCE_PATH ${SOURCE_PATH}
    OPTIONS -DCMAKE_WINDOWS_EXPORT_ALL_SYMBOLS=TRUE
    PREFER_NINJA # Disable this option if project cannot be built with Ninja
)
elseif (VCPKG_LIBRARY_LINKAGE STREQUAL static)
vcpkg_configure_cmake(
    SOURCE_PATH ${SOURCE_PATH}
    OPTIONS -DUPNPC_BUILD_SHARED=OFF
    PREFER_NINJA # Disable this option if project cannot be built with Ninja
)
endif()

vcpkg_install_cmake()

# Remove duplicate directories
file(REMOVE_RECURSE ${CURRENT_PACKAGES_DIR}/debug/include)

# Copy PDB file
vcpkg_copy_pdbs()

# Handle copyright
file(INSTALL ${SOURCE_PATH}/LICENSE DESTINATION ${CURRENT_PACKAGES_DIR}/share/miniupnpc RENAME copyright)
