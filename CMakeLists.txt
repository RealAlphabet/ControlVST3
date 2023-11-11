cmake_minimum_required(VERSION 3.14.0)

set(CMAKE_OSX_DEPLOYMENT_TARGET 10.13 CACHE STRING "")
set(vst3sdk_SOURCE_DIR "vst3sdk" CACHE PATH "Path to VST3 SDK")

if(NOT vst3sdk_SOURCE_DIR)
    message(FATAL_ERROR "Path to VST3 SDK is empty!")
endif()

project(Control_Master
    # This is your plug-in version number. Change it here only.
    # Version number symbols usable in C++ can be found in
    # source/version.h and ${PROJECT_BINARY_DIR}/projectversion.h.
    VERSION 1.0.0.0
    DESCRIPTION "Control_Master VST 3 Plug-in"
)

set(SMTG_VSTGUI_ROOT "${vst3sdk_SOURCE_DIR}")
set(SMTG_CREATE_PLUGIN_LINK OFF)
set(SMTG_ENABLE_VST3_PLUGIN_EXAMPLES OFF)
set(SMTG_ENABLE_VST3_HOSTING_EXAMPLES OFF)
set(SMTG_CREATE_BUNDLE_FOR_WINDOWS ON)
set(SMTG_ENABLE_VSTGUI_SUPPORT ON)

add_subdirectory(${vst3sdk_SOURCE_DIR} ${PROJECT_BINARY_DIR}/vst3sdk)
smtg_enable_vst3_sdk()

smtg_add_vst3plugin(Control_Master
    source/version.h
    source/myplugincids.h
    source/mypluginprocessor.h
    source/mypluginprocessor.cpp
    source/myplugincontroller.h
    source/myplugincontroller.cpp
    source/mypluginentry.cpp
)

#- VSTGUI Wanted ----
if(SMTG_ENABLE_VSTGUI_SUPPORT)
    target_sources(Control_Master
        PRIVATE
            resource/myplugineditor.uidesc
    )
    target_link_libraries(Control_Master
        PRIVATE
            vstgui_support
    )
    smtg_target_add_plugin_resources(Control_Master
        RESOURCES
            "resource/myplugineditor.uidesc"
    )
endif(SMTG_ENABLE_VSTGUI_SUPPORT)
# -------------------

smtg_target_add_plugin_snapshots (Control_Master
    RESOURCES
        resource/3767C22233F951C2890F309ECC3A5006_snapshot.png
        resource/3767C22233F951C2890F309ECC3A5006_snapshot_2.0x.png
)

target_link_libraries(Control_Master
    PRIVATE
        sdk
)

if (SMTG_WIN)
    target_link_libraries(Control_Master PRIVATE wsock32)
endif()

smtg_target_configure_version_file(Control_Master)

if(SMTG_MAC)
    smtg_target_set_bundle(Control_Master
        BUNDLE_IDENTIFIER com.sommet.control
        COMPANY_NAME "Sommet"
    )
    smtg_target_set_debug_executable(Control_Master
        "/Applications/VST3PluginTestHost.app"
        "--pluginfolder;$(BUILT_PRODUCTS_DIR)"
    )
elseif(SMTG_WIN)
    target_sources(Control_Master PRIVATE
        resource/win32resource.rc
    )
    if(MSVC)
        set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT Control_Master)

        smtg_target_set_debug_executable(Control_Master
            "$(ProgramW6432)/Steinberg/VST3PluginTestHost/VST3PluginTestHost.exe"
            "--pluginfolder \"$(OutDir)/\""
        )
    endif()
endif(SMTG_MAC)
