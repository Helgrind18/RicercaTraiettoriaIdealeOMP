cmake_minimum_required(VERSION 4.3)

project(RicercaTraiettoriaIdealeOMP)

set(CMAKE_CXX_STANDARD 20)

add_executable(RicercaTraiettoriaIdealeOMP
        main.cpp

        include/Punto2D.hpp
        src/Punto2D.cpp

        include/Pista.hpp
        src/Pista.cpp

        test/TestPunto2D.hpp
        test/TestPunto2D.cpp

        test/TestPista.hpp
        test/TestPista.cpp
        include/Veicolo.hpp
        include/Gomma.hpp
        src/Gomma.cpp
        src/Veicolo.cpp
        test/TestVeicolo.hpp
        test/TestVeicolo.cpp
        include/SezionePista.hpp
        src/SezionePista.cpp
)

target_include_directories(RicercaTraiettoriaIdealeOMP PRIVATE
        include
        test
)