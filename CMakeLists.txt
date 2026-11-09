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
        include/ValidatoreGeometrico.hpp
        src/ValidatoreGeometrico.cpp
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
option(APEX_BUILD_GUI "Compila GUI Qt6 (OFF per test C++ senza Qt)" ON)
file(GLOB APEX_CORE_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")
list(FILTER APEX_CORE_SOURCES EXCLUDE REGEX "/main\\.cpp$")
if(APEX_BUILD_GUI)
find_package(Qt6 6.2 REQUIRED COMPONENTS Widgets)

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
    gui/GraficoConvergenza.hpp
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
endif()  # APEX_BUILD_GUI
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

# Diagnostica quantitativa della convergenza numerica (senza Qt, senza genetico).
# I fallimenti della soglia di convergenza sono segnalati in output, ma NON
# determinano l'esito del test CTest: il modulo misura, non corregge.
add_executable(TestConvergenzaNumerica
    test/TestConvergenzaNumerica.cpp
    src/Punto2D.cpp
    src/InterpolatorePista.cpp
    src/SezionePista.cpp
    src/Traiettoria.cpp
    src/Gomma.cpp
    src/Veicolo.cpp
    src/ValutatoreTraiettoria.cpp
)
target_include_directories(TestConvergenzaNumerica PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
)
add_test(NAME DiagnosticaConvergenzaNumerica
    COMMAND TestConvergenzaNumerica
        --spa "${CMAKE_CURRENT_SOURCE_DIR}/gui/data/spa.csv"
        --monza "${CMAKE_CURRENT_SOURCE_DIR}/gui/data/monza.csv"
)


# APEX LAB - diagnostica delle traiettorie realmente ottimizzate.
# Gli eseguibili sono separati dalla GUI e NON alterano l'algoritmo genetico.
add_executable(TestConvergenzaOttimizzata
    test/TestConvergenzaOttimizzata.cpp
    ${APEX_CORE_SOURCES}
)
target_include_directories(TestConvergenzaOttimizzata PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
)

add_executable(TestValidatoreGeometrico
    test/TestValidatoreGeometrico.cpp
    src/Punto2D.cpp
    src/SezionePista.cpp
    src/Traiettoria.cpp
    src/ValidatoreGeometrico.cpp
)
target_include_directories(TestValidatoreGeometrico PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
)
add_test(NAME ValidatoreGeometrico
    COMMAND TestValidatoreGeometrico
)
# Il test lungo e' volontariamente esterno a CTest per non rallentare
# ogni normale compilazione/commit. Si lancia quando serve dal terminale.

# ====================== V5 · VALIDAZIONE E BENCHMARK ======================
# Nessun utilizzo di OpenMP. La GUI e il motore fisico sono sequenziali.
add_executable(TestVincoliGenetico
    test/TestVincoliGenetico.cpp
    ${APEX_CORE_SOURCES}
)
target_include_directories(TestVincoliGenetico PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/include")
add_test(NAME VincoliGeometriciNelGenetico COMMAND TestVincoliGenetico)

add_executable(TestEsperimentiGenetici
    test/TestEsperimentiGenetici.cpp
    ${APEX_CORE_SOURCES}
)
target_include_directories(TestEsperimentiGenetici PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/include")
# Benchmark comparativi potenzialmente lunghi: lanciati manualmente.

add_executable(TestVincoliPisteReali
    test/TestVincoliPisteReali.cpp
    ${APEX_CORE_SOURCES}
)
target_include_directories(TestVincoliPisteReali PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/include")
add_test(NAME VincoliSuSpaEMonza
    COMMAND TestVincoliPisteReali
    "${CMAKE_CURRENT_SOURCE_DIR}/gui/data/spa.csv"
    "${CMAKE_CURRENT_SOURCE_DIR}/gui/data/monza.csv"
)
