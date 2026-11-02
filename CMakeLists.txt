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
        test/TestSezionePista.hpp
        test/TestSezionePista.cpp
        include/DiscretizzatorePista.hpp
        src/DiscretizzatorePista.cpp
        include/Traiettoria.hpp
        src/Traiettoria.cpp
        test/TestDiscretizzatorePista.hpp
        test/TestDiscretizzatorePista.cpp
        test/TestTraiettoria.hpp
        test/TestTraiettoria.cpp
        include/GeneraTraiettorie.hpp
        src/GeneraTraiettorie.cpp
        test/TestGeneratoreTraiettorie.hpp
        test/TestGeneratoreTraiettorie.cpp
        include/ValutatoreTraiettoria.hpp
        src/ValutatoreTraiettoria.cpp
        test/TestValutatoreTraiettoria.hpp
        test/TestValutatoreTraiettoria.cpp
)

target_include_directories(RicercaTraiettoriaIdealeOMP PRIVATE
        include
        test
)