#ifndef APEX_LAB_MOTORE_APEX_HPP
#define APEX_LAB_MOTORE_APEX_HPP

#include <QPointF>
#include <QVector>
#include <QString>
#include <functional>

struct ParametriApex {
    QString circuito = "spa";
    int sezioni = 80; // Punti di controllo del genetico
    int campioniValutazione = 4800; // Risoluzione fisica indipendente
    int popolazione = 80;
    int generazioni = 25;
    double crossover = 0.80;
    double mutazione = 0.05;
    double intensita = 0.10;
    int torneo = 3;
    int elite = 4;
    unsigned int seed = 2026;

    double massaKg = 798.0;
    double potenzaKW = 650.0;
    double accelerazione = 10.0;
    double frenata = 15.0;
    double cd = 0.9;
    double areaM2 = 1.5;
    double velocitaKmh = 315.0;
    double accelerazioneLaterale = 18.0;
    double grip = 0.95;
    // Solo vincolo geometrico: non cambia dinamica, grip o forze fisiche.
    double larghezzaVeicoloMetri = 2.0;
    double margineExtraMetri = 0.0;
};

struct GeometriaApex {
    QVector<QPointF> centro;
    QVector<QPointF> sinistro;
    QVector<QPointF> destro;
};

struct RisultatoApex {
    ParametriApex parametri;
    GeometriaApex geometria;
    QVector<QPointF> iniziale;
    QVector<QPointF> ottimizzata;
    QVector<double> geniIniziali;
    QVector<double> geniOttimizzati;
    // Posizioni laterali effettivamente campionate: una per punto di valutazione.
    QVector<double> posizioniCampionateIniziali;
    QVector<double> posizioniCampionateOttimizzate;
    double tempoIniziale = 0.0;
    double tempoFinale = 0.0;
    double durataMs = 0.0;
    double margineMinimoIniziale = 0.0;
    double margineMinimoFinale = 0.0;
    bool validaIniziale = false;
    bool validaFinale = false;
    QVector<int> generazioniStorico, individuiValidi, individuiNonValidi;
    QVector<double> miglioriGenerazione, medieGenerazione, miglioriGlobali;
    // Profili del valutatore usati anche dal calcolo del giro (non inventati dalla GUI).
    QVector<double> distanzaInizialeKm, distanzaOttimizzataKm;
    QVector<double> velocitaInizialeKmh, velocitaOttimizzataKmh;
    // Accelerazione/frenata medie tra sezioni: (v_f^2-v_i^2)/(2*ds).
    QVector<double> accelerazioneIniziale, accelerazioneOttimizzata;
    QVector<double> frenataIniziale, frenataOttimizzata;
    QVector<double> lateraleIniziale, lateraleOttimizzata;
    // Curvatura non firmata, esposta in 1/km per rendere leggibile il grafico.
    QVector<double> curvaturaInizialePerKm, curvaturaOttimizzataPerKm;
};

// Punto di accesso al motore C++ e alla ricostruzione periodica della traiettoria.
QVector<QPointF> caricaCentroPista(const QString &percorso);
GeometriaApex creaGeometriaPista(const QVector<QPointF> &centro, int sezioni, double larghezzaMetri = 12.0);
// Il callback e' facoltativo e viene chiamato dal thread del calcolo.
// Chi lo utilizza non deve aggiornare i widget Qt direttamente.
RisultatoApex eseguiOttimizzazioneApex(
    const ParametriApex &p, const QString &fileCsv,
    const std::function<void(int, int, double)> &progresso = {});

#endif
