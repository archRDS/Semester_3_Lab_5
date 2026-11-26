cmake_minimum_required(VERSION 3.20)
project(MarketplaceLab5 LANGUAGES CXX)

add_executable(Marketplace
    src/main.cpp
    src/ConsoleMenu.cpp
    src/Marketplace.cpp
    src/PaymentStrategy.cpp
    src/User.cpp
)
target_compile_features(Marketplace PRIVATE cxx_std_17)

if(MSVC)
    target_compile_options(Marketplace PRIVATE /W4 /utf-8)
elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(Marketplace PRIVATE -Wall -Wextra -Wpedantic)
endif()
if(MINGW)
    target_link_options(Marketplace PRIVATE -static)
endif()
