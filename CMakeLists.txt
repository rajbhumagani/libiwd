cmake_minimum_required(VERSION 3.20)
project(libiwd VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(LIBIWD_BUILD_TESTS "Build libiwd unit tests" ON)

add_library(iwdwrapper SHARED
    src/logger.cpp
    src/priority_store.cpp
    src/selection_engine.cpp
    src/managers.cpp
    src/iwd_client.cpp
)

target_include_directories(iwdwrapper
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

target_compile_options(iwdwrapper PRIVATE -Wall -Wextra -Wpedantic)

find_library(SYSTEMD_LIB systemd)
if(SYSTEMD_LIB)
    target_link_libraries(iwdwrapper PRIVATE ${SYSTEMD_LIB})
    target_compile_definitions(iwdwrapper PRIVATE LIBIWD_HAS_SYSTEMD=1)
endif()

include(GNUInstallDirs)
install(TARGETS iwdwrapper
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

if(LIBIWD_BUILD_TESTS)
    enable_testing()
    add_executable(libiwd_tests
        tests/test_main.cpp
    )
    target_link_libraries(libiwd_tests PRIVATE iwdwrapper)
    target_compile_options(libiwd_tests PRIVATE -Wall -Wextra -Wpedantic)
    add_test(NAME libiwd_tests COMMAND libiwd_tests)
endif()
