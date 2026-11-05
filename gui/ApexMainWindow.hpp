#ifndef APEX_LAB_MAIN_WINDOW_HPP
#define APEX_LAB_MAIN_WINDOW_HPP

#include "MotoreApex.hpp"
#include <QMainWindow>
#include <QPointer>
#include <optional>

class QComboBox;
class QSpinBox;
class QDoubleSpinBox;
class QCheckBox;
class QLabel;
class QPushButton;
class QProgressBar;
class QThread;
class VistaPista;
class GraficoGeni;
class GraficoProfilo;
class QTabWidget;

class ApexMainWindow : public QMainWindow {
public:
    explicit ApexMainWindow(QWidget *parent = nullptr);
    ~ApexMainWindow() override;

private:
    VistaPista *pista_ = nullptr;
    GraficoGeni *grafico_ = nullptr;
    GraficoProfilo *graficoVelocita_ = nullptr;
    GraficoProfilo *graficoAccelerazione_ = nullptr;
    GraficoProfilo *graficoFrenata_ = nullptr;
    GraficoProfilo *graficoLaterale_ = nullptr;
    GraficoProfilo *graficoCurvatura_ = nullptr;
    QTabWidget *pagine_ = nullptr;
    QComboBox *circuito_ = nullptr;
    QComboBox *modalitaVista_ = nullptr, *amplificazioneVista_ = nullptr;
    QSpinBox *sezioni_ = nullptr, *popolazione_ = nullptr, *generazioni_ = nullptr;
    QSpinBox *torneo_ = nullptr, *elite_ = nullptr, *seed_ = nullptr;
    QDoubleSpinBox *crossover_ = nullptr, *mutazione_ = nullptr, *intensita_ = nullptr;
    QDoubleSpinBox *massa_ = nullptr, *potenza_ = nullptr, *accelerazione_ = nullptr;
    QDoubleSpinBox *frenata_ = nullptr, *cd_ = nullptr, *area_ = nullptr;
    QDoubleSpinBox *velocita_ = nullptr, *accelerazioneLaterale_ = nullptr, *grip_ = nullptr;
    QCheckBox *mostraIniziale_ = nullptr, *mostraOttimizzata_ = nullptr;
    QPushButton *bottoneAvvio_ = nullptr, *bottonePlay_ = nullptr;
    QPushButton *esportaJson_ = nullptr, *esportaCsv_ = nullptr;
    QLabel *tempoIniziale_ = nullptr, *tempoFinale_ = nullptr;
    QLabel *miglioramento_ = nullptr, *durata_ = nullptr;
    QLabel *stato_ = nullptr, *infoPista_ = nullptr;
    QProgressBar *attivita_ = nullptr;
    QLabel *stimaRimanente_ = nullptr;
    QPointer<QThread> lavoro_;
    std::optional<RisultatoApex> risultato_;

    void costruisciFinestra();
    void aggiornaPista();
    void avviaOttimizzazione();
    void mostraRisultato(const RisultatoApex &r);
    void apriVistaGrande();
    void esportaRisultatoJson();
    void esportaRisultatoCsv();
    ParametriApex leggiParametri() const;
    QString percorsoCircuito() const;
};

#endif
