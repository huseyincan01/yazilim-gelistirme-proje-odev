cmake_minimum_required(VERSION 3.20)

project(arkis
        VERSION 1.0.0
        DESCRIPTION "ARKIS - Arac Kiralama Sistemi"
        LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Faz 1: Sadece STL (Qt yok)
include_directories(include)

# --- Faz 2 icin: Asagidaki satirlarin basindaki # isaretlerini kaldirin ---
set(CMAKE_AUTOMOC ON)

find_package(Qt6 6.5 REQUIRED COMPONENTS Widgets Charts)
qt_standard_project_setup()


qt_add_executable(arkis src/main.cpp
        src/main_window.cpp
        src/main_window.h
        include/depo.h
        include/varliklar.h
        src/arac_dialog.h
        src/arac_dialog.cpp
        src/musteri_ekle_dialog.h
        src/musteri_ekle_dialog.cpp
        src/kiralama_sozlesme_dialog.h
        src/kiralama_sozlesme_dialog.cpp
        src/arac_model.h
        src/arac_model.cpp
        src/musteri_model.h
        src/musteri_model.cpp
        src/kiralama_model.h
        src/kiralama_model.cpp
        src/arkaplan_isci.h
        src/arkaplan_isci.cpp
    )

target_link_libraries(arkis PRIVATE Qt6::Widgets Qt6::Charts)


if(MSVC)
    target_compile_options(arkis PRIVATE /W4 /permissive-)
else()
    target_compile_options(arkis PRIVATE -Wall -Wextra -Wpedantic)
endif()