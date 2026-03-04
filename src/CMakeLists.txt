add_library(libbmpsharky SHARED
    chunker.cpp
    bitmap.cpp
    extract.cpp
    hide.cpp
)

target_include_directories(libbmpsharky
    PUBLIC ${CMAKE_SOURCE_DIR}/include
)

add_executable(sharky
    main.cpp
)

target_link_libraries(sharky
    PRIVATE
    libbmpsharky
)
