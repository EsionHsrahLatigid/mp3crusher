cmake_minimum_required(VERSION 3.22)

project(MP3Crusher VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(MP3CRUSHER_BUILD_PLUGIN "Build the JUCE plug-in targets" ON)
option(MP3CRUSHER_BUILD_TESTS "Build plug-in integration tests" ON)
set(MP3CRUSHER_ARTIFACT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/artifacts" CACHE PATH
    "Directory for final VST3, AU, and Standalone products")
set(EHL_JUCE_SOURCE_DIR "" CACHE PATH "Existing JUCE source checkout; avoids network when set")

if(MP3CRUSHER_BUILD_PLUGIN)
    include(FetchContent)
    find_package(Python3 COMPONENTS Interpreter REQUIRED)

    if(EHL_JUCE_SOURCE_DIR)
        if(NOT EXISTS "${EHL_JUCE_SOURCE_DIR}/CMakeLists.txt")
            message(FATAL_ERROR "EHL_JUCE_SOURCE_DIR does not contain JUCE: ${EHL_JUCE_SOURCE_DIR}")
        endif()
        add_subdirectory("${EHL_JUCE_SOURCE_DIR}" "${CMAKE_CURRENT_BINARY_DIR}/juce")
    elseif(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/JUCE/CMakeLists.txt")
        add_subdirectory(JUCE)
    else()
        FetchContent_Declare(
            JUCE
            GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
            GIT_TAG 8.0.15
            GIT_SHALLOW TRUE)
        FetchContent_MakeAvailable(JUCE)
    endif()

    add_subdirectory(modules/juce-ehl-design-module)

    set(MP3CRUSHER_FORMATS VST3 Standalone)
    if(APPLE)
        list(APPEND MP3CRUSHER_FORMATS AU)
    endif()

    juce_add_plugin(MP3Crusher
        VERSION ${PROJECT_VERSION}
        COMPANY_NAME "EsionHsrahLatigid"
        BUNDLE_ID "jp.ehl.mp3crusher"
        PRODUCT_NAME "MP3 Crusher"
        DESCRIPTION "MP3-style bit crush, bandwidth, noise, and stutter effect"
        PLUGIN_MANUFACTURER_CODE EHL_
        PLUGIN_CODE Mp3C
        FORMATS ${MP3CRUSHER_FORMATS}
        IS_SYNTH FALSE
        NEEDS_MIDI_INPUT FALSE
        NEEDS_MIDI_OUTPUT FALSE
        IS_MIDI_EFFECT FALSE
        EDITOR_WANTS_KEYBOARD_FOCUS TRUE
        COPY_PLUGIN_AFTER_BUILD FALSE)

    juce_generate_juce_header(MP3Crusher)

    target_sources(MP3Crusher PRIVATE
        Source/PluginEditor.cpp
        Source/PluginEditor.h
        Source/PluginProcessor.cpp
        Source/PluginProcessor.h)

    target_compile_definitions(MP3Crusher PUBLIC
        JUCE_REPORT_APP_USAGE=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
        JUCE_WEB_BROWSER=0)

    target_link_libraries(MP3Crusher
        PRIVATE
            EHL::JuceDesign
            juce::juce_audio_utils
            juce::juce_dsp
        PUBLIC
            juce::juce_recommended_config_flags
            juce::juce_recommended_lto_flags
            juce::juce_recommended_warning_flags)

    set(mp3crusher_stage_platform "linux-x64")
    if(APPLE)
        set(mp3crusher_stage_platform "macos-arm64")
    elseif(WIN32)
        set(mp3crusher_stage_platform "windows-x64")
    endif()
    set(mp3crusher_juce_products "${CMAKE_CURRENT_BINARY_DIR}/MP3Crusher_artefacts/$<CONFIG>")
    set(mp3crusher_staged_products "${MP3CRUSHER_ARTIFACT_DIR}/plugin-release/${mp3crusher_stage_platform}")
    set(mp3crusher_artifact_dependencies MP3Crusher_VST3 MP3Crusher_Standalone)
    set(mp3crusher_artifact_commands
        COMMAND ${CMAKE_COMMAND} -E make_directory "${mp3crusher_staged_products}/vst3"
        COMMAND ${CMAKE_COMMAND} -E make_directory "${mp3crusher_staged_products}/standalone"
        COMMAND ${CMAKE_COMMAND} -E make_directory "${mp3crusher_staged_products}/au")

    if(APPLE)
        list(APPEND mp3crusher_artifact_commands
            COMMAND ${CMAKE_COMMAND} -E rm -rf "${mp3crusher_staged_products}/vst3/mp3crusher_vst3_plugin.vst3"
            COMMAND ${CMAKE_COMMAND} -E rm -rf "${mp3crusher_staged_products}/standalone/mp3crusher_standalone_plugin.app"
            COMMAND ${CMAKE_COMMAND} -E rm -rf "${mp3crusher_staged_products}/au/mp3crusher_au_plugin.component"
            COMMAND ${CMAKE_COMMAND} -E copy_directory
                    "${mp3crusher_juce_products}/VST3/MP3 Crusher.vst3" "${mp3crusher_staged_products}/vst3/mp3crusher_vst3_plugin.vst3"
            COMMAND ${Python3_EXECUTABLE}
                    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/NormalizeVst3ModuleInfo.py"
                    "${mp3crusher_staged_products}/vst3/mp3crusher_vst3_plugin.vst3/Contents/Resources/moduleinfo.json"
            COMMAND ${CMAKE_COMMAND} -E copy_directory
                    "${mp3crusher_juce_products}/Standalone/MP3 Crusher.app" "${mp3crusher_staged_products}/standalone/mp3crusher_standalone_plugin.app")
    elseif(WIN32)
        list(APPEND mp3crusher_artifact_commands
            COMMAND ${CMAKE_COMMAND} -E rm -rf "${mp3crusher_staged_products}/vst3/mp3crusher_vst3_plugin.vst3"
            COMMAND ${CMAKE_COMMAND} -E copy_directory
                    "${mp3crusher_juce_products}/VST3/MP3 Crusher.vst3" "${mp3crusher_staged_products}/vst3/mp3crusher_vst3_plugin.vst3"
            COMMAND ${Python3_EXECUTABLE}
                    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/NormalizeVst3ModuleInfo.py"
                    "${mp3crusher_staged_products}/vst3/mp3crusher_vst3_plugin.vst3/Contents/Resources/moduleinfo.json"
            COMMAND ${CMAKE_COMMAND} -E copy
                    "${mp3crusher_juce_products}/Standalone/MP3 Crusher.exe" "${mp3crusher_staged_products}/standalone/mp3crusher_standalone_plugin.exe")
    else()
        list(APPEND mp3crusher_artifact_commands
            COMMAND ${CMAKE_COMMAND} -E rm -rf "${mp3crusher_staged_products}/vst3/mp3crusher_vst3_plugin.vst3"
            COMMAND ${CMAKE_COMMAND} -E copy_directory
                    "${mp3crusher_juce_products}/VST3/MP3 Crusher.vst3" "${mp3crusher_staged_products}/vst3/mp3crusher_vst3_plugin.vst3"
            COMMAND ${Python3_EXECUTABLE}
                    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/NormalizeVst3ModuleInfo.py"
                    "${mp3crusher_staged_products}/vst3/mp3crusher_vst3_plugin.vst3/Contents/Resources/moduleinfo.json"
            COMMAND ${CMAKE_COMMAND} -E copy
                    "${mp3crusher_juce_products}/Standalone/MP3 Crusher" "${mp3crusher_staged_products}/standalone/mp3crusher_standalone_plugin")
    endif()

    if(TARGET MP3Crusher_AU)
        list(APPEND mp3crusher_artifact_dependencies MP3Crusher_AU)
        list(APPEND mp3crusher_artifact_commands
            COMMAND ${CMAKE_COMMAND} -E copy_directory
                    "${mp3crusher_juce_products}/AU/MP3 Crusher.component" "${mp3crusher_staged_products}/au/mp3crusher_au_plugin.component")
    endif()

    if(APPLE)
        list(APPEND mp3crusher_artifact_commands
            COMMAND codesign --force --deep --sign -
                    "${mp3crusher_staged_products}/vst3/mp3crusher_vst3_plugin.vst3"
            COMMAND codesign --force --deep --sign -
                    "${mp3crusher_staged_products}/standalone/mp3crusher_standalone_plugin.app")
        if(TARGET MP3Crusher_AU)
            list(APPEND mp3crusher_artifact_commands
                COMMAND codesign --force --deep --sign -
                        "${mp3crusher_staged_products}/au/mp3crusher_au_plugin.component")
        endif()
    endif()

    list(APPEND mp3crusher_artifact_commands
        COMMAND ${CMAKE_COMMAND}
            "-DPRODUCT_NAME=MP3 Crusher"
            -DSLUG=mp3crusher
            -DBUNDLE_ID=jp.ehl.mp3crusher
            -DPLUGIN_CODE=Mp3C
            -DSTAGE_DIR=${mp3crusher_staged_products}
            -P "${CMAKE_CURRENT_SOURCE_DIR}/cmake/WriteArtifactsManifest.cmake")

    add_custom_target(ehl_stage_products
        ${mp3crusher_artifact_commands}
        DEPENDS ${mp3crusher_artifact_dependencies}
        COMMENT "Staging MP3 Crusher products under ${MP3CRUSHER_ARTIFACT_DIR}/plugin-release")

    add_custom_target(MP3Crusher_Artifacts DEPENDS ehl_stage_products)

    if(MP3CRUSHER_BUILD_TESTS)
        enable_testing()
        add_executable(MP3CrusherIntegrationTests Tests/PluginIntegrationTest.cpp)
        target_include_directories(MP3CrusherIntegrationTests PRIVATE
            Source
            "${CMAKE_CURRENT_BINARY_DIR}/MP3Crusher_artefacts/JuceLibraryCode")
        target_link_libraries(MP3CrusherIntegrationTests PRIVATE MP3Crusher juce::juce_audio_utils)
        add_dependencies(MP3CrusherIntegrationTests MP3Crusher_Artifacts)
        add_test(NAME MP3CrusherPluginIntegrationTests COMMAND MP3CrusherIntegrationTests)
        add_test(
            NAME MP3CrusherArtifactChecks
            COMMAND ${CMAKE_COMMAND}
                    -DBUILD_CONFIG=$<CONFIG>
                    -DSTAGE_DIR=${mp3crusher_staged_products}
                    -DSLUG=mp3crusher
                    -DEXPECT_AU=$<BOOL:${APPLE}>
                    -DPython3_EXECUTABLE=${Python3_EXECUTABLE}
                    -P ${CMAKE_CURRENT_SOURCE_DIR}/Tests/VerifyArtifacts.cmake)
    endif()
elseif(MP3CRUSHER_BUILD_TESTS)
    message(FATAL_ERROR "MP3CRUSHER_BUILD_TESTS requires MP3CRUSHER_BUILD_PLUGIN")
endif()
