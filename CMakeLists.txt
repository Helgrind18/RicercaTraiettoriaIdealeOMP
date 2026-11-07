cmake_minimum_required(VERSION 3.20)

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
        include/InterpolatorePista.hpp
        src/InterpolatorePista.cpp
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
        include/AlgoritmoGenetico.hpp
        src/AlgoritmoGenetico.cpp
        test/TestAlgoritmoGenetico.hpp
        test/TestAlgoritmoGenetico.cpp
)

target_include_directories(RicercaTraiettoriaIdealeOMP PRIVATE
        include
        test
)

# ===================== APEX LAB · GUI QT 6 =====================
# Aggiungi questo blocco UNA SOLA VOLTA alla fine del CMakeLists.txt esistente.
# Non tocca il target originale dei tuoi test.
find_package(Qt6 6.2 REQUIRED COMPONENTS Widgets)

# Usa tutti i sorgenti del motore C++ gia' presenti nella cartella src,
# escludendo un eventuale main.cpp (la GUI ha il suo main separato).
file(GLOB APEX_CORE_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)
list(FILTER APEX_CORE_SOURCES EXCLUDE REGEX "/main\\.cpp$")

add_executable(RicercaTraiettoriaGUI
    gui/main_gui.cpp
    gui/ApexMainWindow.hpp
    gui/ApexMainWindow.cpp
    gui/MotoreApex.hpp
    gui/MotoreApex.cpp
    gui/VistaPista.hpp
    gui/VistaPista.cpp
    gui/GraficoGeni.hpp
    gui/GraficoGeni.cpp
    ${APEX_CORE_SOURCES}
)

target_include_directories(RicercaTraiettoriaGUI PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/gui"
)
target_link_libraries(RicercaTraiettoriaGUI PRIVATE Qt6::Widgets)
target_compile_features(RicercaTraiettoriaGUI PRIVATE cxx_std_20)
target_compile_definitions(RicercaTraiettoriaGUI PRIVATE
    APEX_PISTE_DIR="${CMAKE_CURRENT_SOURCE_DIR}/gui/data"
)
# =================== FINE APEX LAB GUI ========================

# Test indipendente dalla GUI: controlla la spline su Spa e Monza.
add_executable(TestInterpolatorePista
    test/TestInterpolatorePista.cpp
    include/InterpolatorePista.hpp
    src/InterpolatorePista.cpp
    include/Punto2D.hpp
    src/Punto2D.cpp
)
target_include_directories(TestInterpolatorePista PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
)
enable_testing()
add_test(NAME GeometriaPistaSpaMonza
    COMMAND TestInterpolatorePista
        "${CMAKE_CURRENT_SOURCE_DIR}/gui/data/spa.csv"
        "${CMAKE_CURRENT_SOURCE_DIR}/gui/data/monza.csv"
)

# Test standalone: indipendenza tra geni e campioni di valutazione.
add_executable(TestRisoluzioneIndipendente
    test/TestRisoluzioneIndipendente.cpp
    ${APEX_CORE_SOURCES}
)
target_include_directories(TestRisoluzioneIndipendente PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/include")
add_test(NAME RisoluzioneIndipendente
    COMMAND TestRisoluzioneIndipendente
        "${CMAKE_CURRENT_SOURCE_DIR}/gui/data/spa.csv"
        "${CMAKE_CURRENT_SOURCE_DIR}/gui/data/monza.csv"
)
