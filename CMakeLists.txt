cmake_minimum_required(VERSION 3.22)

project(MP3Crusher VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(MP3CRUSHER_BUILD_PLUGIN "Build the JUCE plug-in targets" ON)
option(MP3CRUSHER_BUILD_TESTS "Build plug-in integration tests" ON)
set(MP3CRUSHER_ARTIFACT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/artifacts" CACHE PATH
    "Directory for final VST3, AU, and Standalone products")

if(MP3CRUSHER_BUILD_PLUGIN)
    include(FetchContent)
    find_package(Python3 COMPONENTS Interpreter REQUIRED)

    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/JUCE/CMakeLists.txt")
        add_subdirectory(JUCE)
    else()
        FetchContent_Declare(
            JUCE
            GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
            GIT_TAG 8.0.15
            GIT_SHALLOW TRUE)
        FetchContent_MakeAvailable(JUCE)
    endif()

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
        EDITOR_WANTS_KEYBOARD_FOCUS FALSE
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
            juce::juce_audio_utils
            juce::juce_dsp
        PUBLIC
            juce::juce_recommended_config_flags
            juce::juce_recommended_lto_flags
            juce::juce_recommended_warning_flags)

    set(mp3crusher_juce_products "${CMAKE_CURRENT_BINARY_DIR}/MP3Crusher_artefacts/$<CONFIG>")
    set(mp3crusher_staged_products "${MP3CRUSHER_ARTIFACT_DIR}/$<CONFIG>")
    set(mp3crusher_artifact_dependencies MP3Crusher_VST3 MP3Crusher_Standalone)
    set(mp3crusher_artifact_commands
        COMMAND ${CMAKE_COMMAND} -E make_directory "${mp3crusher_staged_products}"
        COMMAND ${CMAKE_COMMAND} -E remove_directory "${mp3crusher_staged_products}/VST3"
        COMMAND ${CMAKE_COMMAND} -E remove_directory "${mp3crusher_staged_products}/Standalone"
        COMMAND ${CMAKE_COMMAND} -E copy_directory
                "${mp3crusher_juce_products}/VST3" "${mp3crusher_staged_products}/VST3"
        COMMAND ${Python3_EXECUTABLE}
                "${CMAKE_CURRENT_SOURCE_DIR}/cmake/NormalizeVst3ModuleInfo.py"
                "${mp3crusher_staged_products}/VST3/MP3 Crusher.vst3/Contents/Resources/moduleinfo.json"
        COMMAND ${CMAKE_COMMAND} -E copy_directory
                "${mp3crusher_juce_products}/Standalone" "${mp3crusher_staged_products}/Standalone")

    if(TARGET MP3Crusher_AU)
        list(APPEND mp3crusher_artifact_dependencies MP3Crusher_AU)
        list(APPEND mp3crusher_artifact_commands
            COMMAND ${CMAKE_COMMAND} -E remove_directory "${mp3crusher_staged_products}/AU"
            COMMAND ${CMAKE_COMMAND} -E copy_directory
                    "${mp3crusher_juce_products}/AU" "${mp3crusher_staged_products}/AU")
    endif()

    if(APPLE)
        list(APPEND mp3crusher_artifact_commands
            COMMAND codesign --force --deep --sign -
                    "${mp3crusher_staged_products}/VST3/MP3 Crusher.vst3"
            COMMAND codesign --force --deep --sign -
                    "${mp3crusher_staged_products}/Standalone/MP3 Crusher.app")
        if(TARGET MP3Crusher_AU)
            list(APPEND mp3crusher_artifact_commands
                COMMAND codesign --force --deep --sign -
                        "${mp3crusher_staged_products}/AU/MP3 Crusher.component")
        endif()
    endif()

    add_custom_target(MP3Crusher_Artifacts
        ${mp3crusher_artifact_commands}
        DEPENDS ${mp3crusher_artifact_dependencies}
        COMMENT "Staging MP3 Crusher products in ${MP3CRUSHER_ARTIFACT_DIR}/$<CONFIG>")

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
                    -DMP3CRUSHER_STAGED_DIR=${MP3CRUSHER_ARTIFACT_DIR}/$<CONFIG>
                    -DMP3CRUSHER_EXPECT_AU=$<BOOL:${APPLE}>
                    -DPython3_EXECUTABLE=${Python3_EXECUTABLE}
                    -P ${CMAKE_CURRENT_SOURCE_DIR}/Tests/VerifyArtifacts.cmake)
    endif()
elseif(MP3CRUSHER_BUILD_TESTS)
    message(FATAL_ERROR "MP3CRUSHER_BUILD_TESTS requires MP3CRUSHER_BUILD_PLUGIN")
endif()
