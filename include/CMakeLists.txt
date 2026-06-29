add_library(fixed_bigint INTERFACE)

add_library(fbi::fixed_bigint ALIAS fixed_bigint)

target_include_directories(fixed_bigint INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:include>
)

target_compile_features(fixed_bigint INTERFACE cxx_std_17)