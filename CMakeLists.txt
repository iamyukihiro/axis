cmake_minimum_required(VERSION 3.22)

file(READ "${CMAKE_CURRENT_SOURCE_DIR}/VERSION" AXIS_CENTER_VERSION)
string(STRIP "${AXIS_CENTER_VERSION}" AXIS_CENTER_VERSION)

project(Axis VERSION ${AXIS_CENTER_VERSION})

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(FetchContent)
include(CTest)

set(AXIS_SOURCE_FILES
    src/domain/AxisParameters.cpp
    src/domain/AxisParameters.h
    src/dsp/AxisProcessorCore.cpp
    src/dsp/AxisProcessorCore.h
    src/ui/components/LevelMeter.cpp
    src/ui/components/LevelMeter.h
    src/PluginEditor.cpp
    src/PluginEditor.h
    src/PluginProcessor.cpp
    src/PluginProcessor.h
)

set(AXIS_TEST_SOURCE_FILES
    tests/TestMain.cpp
    tests/AxisParametersTests.cpp
    tests/AxisProcessorCoreTests.cpp
)

if (DEFINED ENV{JUCE_DIR} AND EXISTS "$ENV{JUCE_DIR}/CMakeLists.txt")
    add_subdirectory("$ENV{JUCE_DIR}" JUCE)
elseif (DEFINED JUCE_DIR AND EXISTS "${JUCE_DIR}/CMakeLists.txt")
    add_subdirectory("${JUCE_DIR}" JUCE)
else()
    FetchContent_Declare(
        juce
        GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
        GIT_TAG 8.0.8
    )
    FetchContent_MakeAvailable(juce)
endif()

juce_add_plugin(AxisCenter
    COMPANY_NAME "T0hka"
    VERSION ${AXIS_CENTER_VERSION}
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD FALSE
    PLUGIN_MANUFACTURER_CODE Axix
    PLUGIN_CODE Axis
    FORMATS VST3
    PRODUCT_NAME "Axis"
)

juce_generate_juce_header(AxisCenter)

juce_add_binary_data(AxisCenterAssets
    SOURCES
        src/logo.svg
)

if(APPLE)
    add_custom_command(TARGET AxisCenter_VST3 POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory "$ENV{HOME}/Library/Audio/Plug-Ins/VST3"
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "$<TARGET_BUNDLE_DIR:AxisCenter_VST3>"
            "$ENV{HOME}/Library/Audio/Plug-Ins/VST3/$<TARGET_FILE_BASE_NAME:AxisCenter_VST3>.vst3"
        COMMENT "Installing Axis.vst3 to ~/Library/Audio/Plug-Ins/VST3"
    )
endif()

target_sources(AxisCenter
    PRIVATE
        ${AXIS_SOURCE_FILES}
)

target_compile_definitions(AxisCenter
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
)

target_link_libraries(AxisCenter
    PRIVATE
        AxisCenterAssets
        juce::juce_audio_utils
        juce::juce_dsp
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)

find_program(CLANG_FORMAT_BIN
    NAMES clang-format
    PATHS /Library/Developer/CommandLineTools/usr/bin
)

if(CLANG_FORMAT_BIN)
    add_custom_target(format
        COMMAND ${CLANG_FORMAT_BIN} -i ${AXIS_SOURCE_FILES}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMENT "Formatting Axis sources with clang-format"
    )
else()
    message(STATUS "clang-format not found: 'format' target will not be available")
endif()

find_program(CLANG_TIDY_BIN NAMES clang-tidy)

if(CLANG_TIDY_BIN)
    add_custom_target(tidy
        COMMAND ${CLANG_TIDY_BIN} ${AXIS_SOURCE_FILES} -p ${CMAKE_BINARY_DIR}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMENT "Running clang-tidy on Axis sources"
    )
else()
    message(STATUS "clang-tidy not found: 'tidy' target will not be available")
endif()

if(BUILD_TESTING)
    add_executable(axis-tests
        ${AXIS_TEST_SOURCE_FILES}
        src/domain/AxisParameters.cpp
        src/domain/AxisParameters.h
        src/dsp/AxisProcessorCore.cpp
        src/dsp/AxisProcessorCore.h
    )

    add_dependencies(axis-tests AxisCenter)

    target_include_directories(axis-tests
        PRIVATE
            src
            "${CMAKE_CURRENT_BINARY_DIR}/AxisCenter_artefacts/JuceLibraryCode"
    )

    target_compile_definitions(axis-tests
        PUBLIC
            JUCE_WEB_BROWSER=0
            JUCE_USE_CURL=0
            JUCE_VST3_CAN_REPLACE_VST2=0
    )

    target_link_libraries(axis-tests
        PRIVATE
            juce::juce_audio_utils
            juce::juce_dsp
        PUBLIC
            juce::juce_recommended_config_flags
            juce::juce_recommended_lto_flags
            juce::juce_recommended_warning_flags
    )

    add_test(NAME axis-tests COMMAND axis-tests)
endif()
