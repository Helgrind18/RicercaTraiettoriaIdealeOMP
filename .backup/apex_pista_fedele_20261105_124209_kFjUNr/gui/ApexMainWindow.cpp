#include "ApexMainWindow.hpp"
#include "VistaPista.hpp"
#include "GraficoGeni.hpp"
#include "GraficoProfilo.hpp"

#include <QApplication>
#include <QCheckBox>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QDialog>
#include <QFile>
#include <QFileDialog>
#include <QFont>
#include <QFrame>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLabel>
#include <QMessageBox>
#include <QProgressBar>
#include <QPushButton>
#include <QScrollArea>
#include <QSpacerItem>
#include <QSpinBox>
#include <QTabWidget>
#include <QTextStream>
#include <QThread>
#include <QTimer>
#include <QDateTime>
#include <QVBoxLayout>
#include <QWidget>

#include <atomic>
#include <algorithm>
#include <cstdint>
#include <cmath>
#include <limits>
#include <memory>
#include <optional>
#include <stdexcept>

#ifndef APEX_PISTE_DIR
#define APEX_PISTE_DIR "gui/data"
#endif

namespace {
QString durataStimata(double secondi) {
    if (!std::isfinite(secondi) || secondi < 0.0) return "—";
    const auto totale = static_cast<qint64>(std::clamp(secondi, 0.0, 9e15));
    const qint64 giorni = totale / 86400;
    const qint64 ore = (totale / 3600) % 24;
    const qint64 minuti = (totale / 60) % 60;
    const qint64 sec = totale % 60;
    if (giorni > 0) return QString("%1 g %2 h %3 min").arg(giorni).arg(ore).arg(minuti);
    if (ore > 0) return QString("%1 h %2 min").arg(ore).arg(minuti);
    return QString("%1 min %2 s").arg(minuti).arg(sec, 2, 10, QChar('0'));
}

QLabel *testo(const QString &contenuto, const QString &classe = {}) {
    auto *l = new QLabel(contenuto);
    if (!classe.isEmpty()) l->setObjectName(classe);
    l->setWordWrap(true);
    return l;
}
QWidget *contenitore(const QString &nome = "card") {
    auto *w = new QWidget;
    w->setObjectName(nome);
    return w;
}
QVBoxLayout *verticale(QWidget *w, int margin = 16, int spacing = 12) {
    auto *v = new QVBoxLayout(w);
    v->setContentsMargins(margin, margin, margin, margin);
    v->setSpacing(spacing);
    return v;
}
QDoubleSpinBox *decimale(double min, double max, double value, int decimals = 2, double step = .05,
                         const QString &unit = {}) {
    auto *s = new QDoubleSpinBox;
    s->setRange(min, max);
    s->setDecimals(decimals);
    s->setSingleStep(step);
    s->setValue(value);
    if (!unit.isEmpty()) s->setSuffix(" " + unit);
    s->setButtonSymbols(QAbstractSpinBox::PlusMinus);
    return s;
}
QSpinBox *intero(int min, int max, int value) {
    auto *s = new QSpinBox;
    s->setRange(min, max);
    s->setValue(value);
    s->setButtonSymbols(QAbstractSpinBox::PlusMinus);
    return s;
}
void riga(QGridLayout *layout, int r, const QString &nome, QWidget *campo) {
    QLabel *titolo = testo(nome, "labelParam");
    layout->addWidget(titolo, r, 0);
    layout->addWidget(campo, r, 1);
}
void metrica(QVBoxLayout *layout, const QString &titolo, QLabel *&valore, const QString &iniziale,
             const QString &objectName = "metricaValore") {
    QWidget *box = contenitore("metrica");
    auto *v = verticale(box, 13, 4);
    v->addWidget(testo(titolo.toUpper(), "metricaTitolo"));
    valore = testo(iniziale, objectName);
    v->addWidget(valore);
    layout->addWidget(box);
}
QJsonArray puntiJson(const QVector<QPointF> &punti) {
    QJsonArray array;
    for (const auto &p : punti) {
        QJsonArray pair;
        pair.append(p.x()); pair.append(p.y());
        array.append(pair);
    }
    return array;
}
QJsonArray geniJson(const QVector<double> &geni) {
    QJsonArray array;
    for (const double g : geni) array.append(g);
    return array;
}
}  // namespace

ApexMainWindow::ApexMainWindow(QWidget *parent) : QMainWindow(parent) {
    setWindowTitle("APEX LAB  |  Ricerca traiettoria ideale · C++ / Qt6");
    resize(1680, 960);
    setMinimumSize(1120, 720);
    costruisciFinestra();
    aggiornaPista();
}

ApexMainWindow::~ApexMainWindow() {
    if (lavoro_ && lavoro_->isRunning()) lavoro_->wait();
}

QString ApexMainWindow::percorsoCircuito() const {
    return QString::fromUtf8(APEX_PISTE_DIR) + "/" + circuito_->currentData().toString() + ".csv";
}

void ApexMainWindow::costruisciFinestra() {
    setStyleSheet(R"CSS(
        QMainWindow, QWidget#fondo { background: #080F1A; color: #E9F2FF; }
        QWidget#card, QWidget#pannello { background: #111E2E; border: 1px solid #26374A; border-radius: 13px; }
        QWidget#metrica { background: #0D1726; border: 1px solid #273950; border-radius: 10px; }
        QLabel { color: #D8E4F4; background: transparent; }
        QLabel#logo { color: #F4FAFF; font-size: 23px; font-weight: 900; letter-spacing: 2px; }
        QLabel#micro { color: #3AE0D5; font-size: 10px; letter-spacing: 3px; font-weight: 700; }
        QLabel#muted, QLabel#description, QLabel#labelParam { color: #93A6BE; }
        QLabel#titoloCard { color: #F0F7FF; font-size: 13px; font-weight: 800; letter-spacing: 1px; }
        QLabel#metricaTitolo { color: #89A0BC; font-size: 10px; font-weight: 700; letter-spacing: 1px; }
        QLabel#metricaValore { color: #EAF8FF; font-size: 22px; font-weight: 800; }
        QLabel#metricaHighlight { color: #30E1CF; font-size: 24px; font-weight: 900; }
        QLabel#stato { color: #A8BED6; padding: 5px 0; }
        QSpinBox, QDoubleSpinBox, QComboBox {
            background: #0A1524; color: #E8F2FF; border: 1px solid #334B66;
            border-radius: 7px; padding: 7px 10px; min-height: 23px;
            selection-background-color: #166C79;
        }
        QSpinBox:hover, QDoubleSpinBox:hover, QComboBox:hover { border-color: #32CDBE; }
        QComboBox QAbstractItemView { background: #101F30; color: #E8F2FF; selection-background-color: #165E70; }
        QPushButton { background: #1B2D41; color: #E4F2FF; border: 1px solid #38506B;
                      border-radius: 8px; padding: 9px 15px; font-weight: 700; }
        QPushButton:hover { background: #27405A; border-color: #38C7C6; }
        QPushButton:disabled { color: #687C92; background: #172438; border-color: #29374B; }
        QPushButton#avvia { background: #18CBBF; color: #061A22; border: none; font-size: 13px;
                            font-weight: 900; padding: 13px; }
        QPushButton#avvia:hover { background: #52F1E5; }
        QCheckBox { color: #C6D8E8; spacing: 6px; }
        QCheckBox::indicator { width: 16px; height: 16px; }
        QCheckBox::indicator:unchecked { border: 1px solid #56718D; background: #0C1829; border-radius: 3px; }
        QCheckBox::indicator:checked { background: #28DCD3; border: 1px solid #28DCD3; border-radius: 3px; }
        QTabWidget::pane { border: 0; }
        QTabBar::tab { background: #132337; color: #9AAFC9; padding: 12px 18px; border-radius: 5px; }
        QTabBar::tab:selected { background: #1E5765; color: #F3FAFF; }
        QScrollArea, QScrollArea QWidget#qt_scrollarea_viewport { border: none; background: transparent; }
        QScrollBar:vertical { width: 8px; background: #0A1624; }
        QScrollBar::handle:vertical { background: #36546C; border-radius: 4px; }
        QProgressBar { background: #102339; border: none; border-radius: 4px; height: 7px; }
        QProgressBar::chunk { background: #20DBCD; border-radius: 4px; }
    )CSS");

    auto *fondo = contenitore("fondo");
    setCentralWidget(fondo);
    auto *main = new QVBoxLayout(fondo);
    main->setContentsMargins(18, 14, 18, 16);
    main->setSpacing(14);

    // HEADER
    auto *barra = new QHBoxLayout;
    auto *marca = new QVBoxLayout;
    marca->setSpacing(0);
    marca->addWidget(testo("APEX / LAB", "logo"));
    marca->addWidget(testo("RACING LINE OPTIMIZATION  //  C++ ENGINE", "micro"));
    barra->addLayout(marca);
    barra->addStretch();
    barra->addWidget(testo("SEQUENTIAL GA  ·  QT 6 DESKTOP", "micro"));
    main->addLayout(barra);

    auto *sezione = new QHBoxLayout;
    sezione->setSpacing(14);
    main->addLayout(sezione, 1);

    // SINISTRA: Parametri genetici e veicolo.
    QWidget *sinistra = contenitore("pannello");
    sinistra->setMinimumWidth(305);
    sinistra->setMaximumWidth(345);
    auto *l = verticale(sinistra, 15, 11);
    l->addWidget(testo("CONFIGURAZIONE", "titoloCard"));
    l->addWidget(testo("Imposta il circuito e i parametri del tuo algoritmo genetico.", "description"));
    l->addWidget(testo("CIRCUITO", "micro"));
    circuito_ = new QComboBox;
    circuito_->addItem("SPA-FRANCORCHAMPS", "spa");
    circuito_->addItem("AUTODROMO DI MONZA", "monza");
    l->addWidget(circuito_);
    infoPista_ = testo("", "muted");
    l->addWidget(infoPista_);
    auto *tabs = new QTabWidget;
    l->addWidget(tabs, 1);

    auto *genetico = contenitore("pannello");
    auto *gl = verticale(genetico, 10, 12);
    auto *g = new QGridLayout;
    g->setColumnStretch(0,1);
    g->setHorizontalSpacing(10);
    g->setVerticalSpacing(9);
    sezioni_ = intero(3, std::numeric_limits<int>::max(), 80);
    popolazione_ = intero(2, std::numeric_limits<int>::max(), 80);
    generazioni_ = intero(1, std::numeric_limits<int>::max(), 25);
    crossover_ = decimale(0.0, 1.0, .80);
    mutazione_ = decimale(0.0, 1.0, .05);
    intensita_ = decimale(.001, 1.0, .10, 3, .01);
    torneo_ = intero(2, 80, 3);
    elite_ = intero(0, 79, 4);
    seed_ = intero(0, 2147483647, 2026);
    int gr = 0;
    riga(g, gr++, "Sezioni", sezioni_);
    riga(g, gr++, "Popolazione", popolazione_);
    riga(g, gr++, "Generazioni", generazioni_);
    riga(g, gr++, "Crossover", crossover_);
    riga(g, gr++, "Mutazione", mutazione_);
    riga(g, gr++, "Intensità", intensita_);
    riga(g, gr++, "Dimensione torneo", torneo_);
    riga(g, gr++, "Élite", elite_);
    riga(g, gr++, "Seed", seed_);
    gl->addLayout(g);
    gl->addStretch();
    tabs->addTab(genetico, "GENETICO");

    auto *veicolo = contenitore("pannello");
    auto *vl = verticale(veicolo, 10, 12);
    auto *v = new QGridLayout;
    v->setColumnStretch(0, 1);
    v->setVerticalSpacing(9);
    massa_ = decimale(1, 10000, 798, 0, 1, "kg");
    potenza_ = decimale(1, 1500, 650, 0, 10, "kW");
    accelerazione_ = decimale(.1, 50, 10, 1, .5, "m/s²");
    frenata_ = decimale(.1, 60, 15, 1, .5, "m/s²");
    cd_ = decimale(.01, 5, .9, 2, .05);
    area_ = decimale(.1, 20, 1.5, 2, .1, "m²");
    velocita_ = decimale(1, 500, 315, 0, 5, "km/h");
    accelerazioneLaterale_ = decimale(.1, 50, 18, 1, .5, "m/s²");
    grip_ = decimale(.01, 1, .95, 2, .05);
    int vr = 0;
    riga(v, vr++, "Massa", massa_);
    riga(v, vr++, "Potenza max", potenza_);
    riga(v, vr++, "Accelerazione", accelerazione_);
    riga(v, vr++, "Frenata", frenata_);
    riga(v, vr++, "Coefficiente Cd", cd_);
    riga(v, vr++, "Area frontale", area_);
    riga(v, vr++, "Velocità max", velocita_);
    riga(v, vr++, "Acceleraz. laterale", accelerazioneLaterale_);
    riga(v, vr++, "Grip gomme", grip_);
    vl->addLayout(v);
    vl->addStretch();
    tabs->addTab(veicolo, "VEICOLO");

    connect(popolazione_, qOverload<int>(&QSpinBox::valueChanged), this, [this](int n) {
        torneo_->setMaximum(n);
        elite_->setMaximum(n - 1);
    });
    bottoneAvvio_ = new QPushButton("▶  AVVIA OTTIMIZZAZIONE");
    bottoneAvvio_->setObjectName("avvia");
    l->addWidget(bottoneAvvio_);
    attivita_ = new QProgressBar;
    attivita_->setRange(0, 100);
    attivita_->setFormat("%p% completato");
    attivita_->setTextVisible(true);
    attivita_->setVisible(false);
    l->addWidget(attivita_);
    stimaRimanente_ = testo("", "muted");
    stimaRimanente_->setVisible(false);
    l->addWidget(stimaRimanente_);
    l->addWidget(testo("I calcoli utilizzano le tue classi C++ originali.", "description"));
    sezione->addWidget(sinistra);

    // CENTRO: pista + grafico genetico.
    pagine_ = new QTabWidget;
    pagine_->setObjectName("paginePrincipali");
    sezione->addWidget(pagine_, 1);
    auto *paginaCircuito = contenitore("fondo");
    auto *centrale = verticale(paginaCircuito, 0, 0);
    centrale->setSpacing(14);
    auto *cartaPista = contenitore("card");
    auto *cp = verticale(cartaPista, 12, 10);
    auto *titoloPista = new QHBoxLayout;
    titoloPista->addWidget(testo("CIRCUIT VIEW  /  CONFRONTO TRAIETTORIE", "titoloCard"));
    titoloPista->addStretch();
    auto *barraConfronto = new QHBoxLayout;
    barraConfronto->addWidget(testo("MODALITÀ", "muted"));
    modalitaVista_ = new QComboBox;
    modalitaVista_->addItem("AFFIANCATE", 0);
    modalitaVista_->addItem("SOVRAPPOSTE", 1);
    modalitaVista_->setToolTip("Mostra le due traiettorie su pannelli distinti oppure sullo stesso circuito");
    barraConfronto->addWidget(modalitaVista_);
    barraConfronto->addWidget(testo("LARGHEZZA VISIVA", "muted"));
    amplificazioneVista_ = new QComboBox;
    amplificazioneVista_->addItem("NORMALE", 1);
    amplificazioneVista_->addItem("PIÙ LARGA", 10);
    amplificazioneVista_->addItem("MOLTO LARGA", 25);
    amplificazioneVista_->addItem("MASSIMA", 50);
    amplificazioneVista_->setCurrentIndex(0);  // Scala reale come impostazione predefinita.
    amplificazioneVista_->setToolTip("Modifica soltanto la larghezza disegnata di pista e traiettorie; non altera il modello fisico");
    barraConfronto->addWidget(amplificazioneVista_);
    barraConfronto->addStretch();
    auto *reset = new QPushButton("CENTRA VISTA");
    reset->setToolTip("Reimposta zoom e spostamento");
    auto *grande = new QPushButton("⤢  VISTA GRANDE");
    grande->setToolTip("Apri il confronto su una finestra massimizzata e ingrandisci le curve con la rotellina");
    titoloPista->addWidget(grande);
    titoloPista->addWidget(reset);
    cp->addLayout(titoloPista);
    cp->addLayout(barraConfronto);
    pista_ = new VistaPista;
    cp->addWidget(pista_, 1);
    auto *notaConfronto = testo(
        "CURVE CUBICHE DI SOLA VISUALIZZAZIONE: I PUNTI DEL MODELLO E I TEMPI NON CAMBIANO. "
        "×10/×25/×50 sono solo deformazioni grafiche opzionali.", "description");
    cp->addWidget(notaConfronto);
    auto *controlli = new QHBoxLayout;
    mostraIniziale_ = new QCheckBox("● INIZIALE · ARANCIONE");
    mostraIniziale_->setStyleSheet("color: #FFB062; font-weight: 700;");
    mostraIniziale_->setChecked(true);
    mostraOttimizzata_ = new QCheckBox("● OTTIMIZZATA · TURCHESE");
    mostraOttimizzata_->setStyleSheet("color: #25E7DE; font-weight: 700;");
    mostraOttimizzata_->setChecked(true);
    controlli->addWidget(mostraIniziale_);
    controlli->addWidget(mostraOttimizzata_);
    controlli->addStretch();
    bottonePlay_ = new QPushButton("❚❚  PAUSA");
    controlli->addWidget(bottonePlay_);
    cp->addLayout(controlli);
    centrale->addWidget(cartaPista, 1);
    pagine_->addTab(paginaCircuito, "01  CIRCUITO");

    // ANALISI: pagina separata con risultati e grafici del motore C++.
    auto *scrollAnalisi = new QScrollArea;
    scrollAnalisi->setWidgetResizable(true);
    auto *paginaAnalisi = contenitore("fondo");
    auto *a = verticale(paginaAnalisi, 12, 14);
    a->addWidget(testo("ANALISI DEL GIRO  /  CONFRONTO INIZIALE VS OTTIMIZZATA", "titoloCard"));
    a->addWidget(testo("I profili di velocità sono quelli del ValutatoreTraiettoria. Accelerazione e frenata sono stime cinematiche ricavate dalle velocità tra sezioni; non sono comandi del pilota.", "description"));
    auto *metriche = new QHBoxLayout;
    for (int k = 0; k < 4; ++k) {
        auto *col = new QVBoxLayout;
        if (k == 0) metrica(col, "Migliore iniziale", tempoIniziale_, "— s");
        if (k == 1) metrica(col, "Migliore ottimizzata", tempoFinale_, "— s", "metricaHighlight");
        if (k == 2) metrica(col, "Riduzione tempo sul giro", miglioramento_, "— %", "metricaHighlight");
        if (k == 3) metrica(col, "Durata del calcolo", durata_, "— ms");
        metriche->addLayout(col, 1);
    }
    a->addLayout(metriche);

    auto *griglia = new QGridLayout;
    griglia->setHorizontalSpacing(12);
    griglia->setVerticalSpacing(12);
    griglia->setColumnStretch(0, 1);
    griglia->setColumnStretch(1, 1);
    const auto schedaGrafico = [&](const QString& titolo, const QString& nota,
                                  GraficoProfilo*& grafico, const QString& unita,
                                  int r, int c, int colonne = 1) {
        auto *scheda = contenitore("card");
        auto *layout = verticale(scheda, 12, 7);
        layout->addWidget(testo(titolo, "titoloCard"));
        layout->addWidget(testo(nota, "description"));
        grafico = new GraficoProfilo(unita);
        layout->addWidget(grafico, 1);
        griglia->addWidget(scheda, r, c, 1, colonne);
    };
    schedaGrafico("VELOCITÀ  /  PROFILO SUL GIRO",
                  "Velocità prevista dal modello fisico, in km/h.",
                  graficoVelocita_, "km/h", 0, 0);
    schedaGrafico("ACCELERAZIONE  /  LONGITUDINALE",
                  "Accelerazione positiva stimata sui segmenti, in m/s².",
                  graficoAccelerazione_, "m/s²", 0, 1);
    schedaGrafico("FRENATA  /  DECELERAZIONE",
                  "Modulo della decelerazione stimata sui segmenti, in m/s².",
                  graficoFrenata_, "m/s²", 1, 0);
    schedaGrafico("ACCELERAZIONE LATERALE  /  CURVE",
                  "Stima v² · curvatura in ogni punto, in m/s².",
                  graficoLaterale_, "m/s²", 1, 1);
    schedaGrafico("CURVATURA  /  GEOMETRIA DELLA TRAIETTORIA",
                  "Curvatura non firmata calcolata dal ValutatoreTraiettoria con 3 punti consecutivi. "
                  "Picchi isolati possono imporre bruschi cali di velocità. Unità: 1/km.",
                  graficoCurvatura_, "1/km", 2, 0, 2);
    a->addLayout(griglia);

    auto *cartaGeni = contenitore("card");
    auto *cg = verticale(cartaGeni, 12, 8);
    cg->addWidget(testo("GENOMA  /  POSIZIONI LATERALI [0–1]", "titoloCard"));
    cg->addWidget(testo("Arancione tratteggiato: iniziale  ·  Turchese: ottimizzata", "description"));
    grafico_ = new GraficoGeni;
    grafico_->setMinimumHeight(250);
    cg->addWidget(grafico_);
    a->addWidget(cartaGeni);

    auto *azioniAnalisi = new QHBoxLayout;
    azioniAnalisi->addWidget(testo("ESPORTA RISULTATI", "micro"));
    esportaJson_ = new QPushButton("ESPORTA JSON");
    esportaCsv_ = new QPushButton("ESPORTA CSV");
    esportaJson_->setEnabled(false);
    esportaCsv_->setEnabled(false);
    azioniAnalisi->addWidget(esportaJson_);
    azioniAnalisi->addWidget(esportaCsv_);
    azioniAnalisi->addStretch();
    a->addLayout(azioniAnalisi);
    a->addWidget(testo("Nota: la pista è ricavata da coordinate GPS, con larghezza stimata di 12 m. I grafici sono simulazioni del modello del tuo progetto, non telemetria misurata.", "description"));
    scrollAnalisi->setWidget(paginaAnalisi);
    pagine_->addTab(scrollAnalisi, "02  ANALISI E GRAFICI");

    // FOOTER
    stato_ = testo("PRONTO  ·  SELEZIONA UN CIRCUITO E AVVIA IL CALCOLO", "stato");
    main->addWidget(stato_);

    connect(reset, &QPushButton::clicked, pista_, &VistaPista::resetVista);
    connect(grande, &QPushButton::clicked, this, [this] { apriVistaGrande(); });
    connect(modalitaVista_, qOverload<int>(&QComboBox::currentIndexChanged), this, [this] {
        const auto modo = modalitaVista_->currentData().toInt() == 0
            ? VistaPista::ModalitaConfronto::Affiancato
            : VistaPista::ModalitaConfronto::Sovrapposto;
        pista_->impostaModalitaConfronto(modo);
    });
    connect(amplificazioneVista_, qOverload<int>(&QComboBox::currentIndexChanged), this, [this] {
        pista_->impostaAmplificazione(amplificazioneVista_->currentData().toDouble());
    });
    pista_->impostaAmplificazione(amplificazioneVista_->currentData().toDouble());
    connect(circuito_, qOverload<int>(&QComboBox::currentIndexChanged), this, [this] { aggiornaPista(); });
    connect(bottoneAvvio_, &QPushButton::clicked, this, [this] { avviaOttimizzazione(); });
    auto cambiaVisibilita = [this] {
        pista_->impostaVisibilita(mostraIniziale_->isChecked(), mostraOttimizzata_->isChecked());
    };
    connect(mostraIniziale_, &QCheckBox::toggled, this, cambiaVisibilita);
    connect(mostraOttimizzata_, &QCheckBox::toggled, this, cambiaVisibilita);
    connect(bottonePlay_, &QPushButton::clicked, this, [this] {
        const bool nuovoStato = !pista_->animazioneAttiva();
        pista_->avviaAnimazione(nuovoStato);
        bottonePlay_->setText(nuovoStato ? "❚❚  PAUSA" : "▶  RIPRENDI");
    });
    connect(esportaJson_, &QPushButton::clicked, this, [this] { esportaRisultatoJson(); });
    connect(esportaCsv_, &QPushButton::clicked, this, [this] { esportaRisultatoCsv(); });
    pista_->avviaAnimazione(true);
}

// Visualizzazione estesa: usa le stesse traiettorie reali gia' calcolate
// dal motore C++. Nessun nuovo calcolo o modifica dell'algoritmo genetico.
void ApexMainWindow::apriVistaGrande() {
    auto *finestra = new QDialog(this);
    finestra->setAttribute(Qt::WA_DeleteOnClose);
    finestra->setWindowFlag(Qt::WindowMaximizeButtonHint, true);
    finestra->setWindowTitle("APEX LAB | Confronto traiettorie · vista grande");
    finestra->setStyleSheet(R"CSS(
        QDialog { background: #080F1A; color: #E9F2FF; }
        QLabel { color: #B6D1E6; }
        QPushButton, QComboBox { background: #17283D; color: #EAF8FF;
            border: 1px solid #38526A; border-radius: 7px; padding: 9px 12px; }
        QPushButton:hover, QComboBox:hover { background: #28506B; }
    )CSS");
    auto *layout = new QVBoxLayout(finestra);
    layout->setContentsMargins(14, 12, 14, 14);
    layout->setSpacing(9);
    auto *barra = new QHBoxLayout;
    auto *etichetta = new QLabel("APEX / LAB   ·   CONFRONTO IN SCALA REALE");
    QFont f = etichetta->font(); f.setBold(true); f.setPointSize(12);
    etichetta->setFont(f);
    barra->addWidget(etichetta);
    barra->addStretch();
    auto *modalita = new QComboBox;
    modalita->addItem("AFFIANCATE", 0);
    modalita->addItem("SOVRAPPOSTE", 1);
    modalita->setCurrentIndex(modalitaVista_->currentIndex());
    barra->addWidget(modalita);
    auto *meno = new QPushButton("− ZOOM");
    auto *piu = new QPushButton("+ ZOOM");
    auto *ripristina = new QPushButton("MOSTRA TUTTO");
    barra->addWidget(meno);
    barra->addWidget(piu);
    barra->addWidget(ripristina);
    layout->addLayout(barra);

    auto *vista = new VistaPista(finestra);
    vista->impostaAmplificazione(1.0);
    vista->impostaVisibilita(mostraIniziale_->isChecked(), mostraOttimizzata_->isChecked());
    vista->impostaModalitaConfronto(modalitaVista_->currentData().toInt() == 0 ?
        VistaPista::ModalitaConfronto::Affiancato : VistaPista::ModalitaConfronto::Sovrapposto);
    if (risultato_) {
        vista->mostraRisultato(*risultato_);
    } else {
        try {
            vista->mostraGeometria(creaGeometriaPista(caricaCentroPista(percorsoCircuito()), 150));
        } catch (const std::exception &e) {
            QMessageBox::warning(this, "Caricamento pista", QString::fromUtf8(e.what()));
        }
    }
    layout->addWidget(vista, 1);
    auto *istruzioni = new QLabel(
        "ROTELLINA: ingrandisci la curva sotto il cursore  ·  DOPPIO CLIC: zoom rapido  ·  "
        "TRASCINA: sposta entrambe le viste  ·  "
        "LARGHEZZA DELLA PISTA ALLARGATA SOLO GRAFICAMENTE");
    istruzioni->setWordWrap(true);
    layout->addWidget(istruzioni);
    connect(modalita, qOverload<int>(&QComboBox::currentIndexChanged), vista,
            [vista, modalita](int) {
        vista->impostaModalitaConfronto(modalita->currentData().toInt() == 0 ?
            VistaPista::ModalitaConfronto::Affiancato : VistaPista::ModalitaConfronto::Sovrapposto);
    });
    connect(meno, &QPushButton::clicked, vista, [vista] { vista->modificaZoom(1.0 / 1.6); });
    connect(piu, &QPushButton::clicked, vista, [vista] { vista->modificaZoom(1.6); });
    connect(ripristina, &QPushButton::clicked, vista, &VistaPista::resetVista);
    vista->avviaAnimazione(true);
    finestra->resize(1500, 920);
    finestra->showMaximized();
}

ParametriApex ApexMainWindow::leggiParametri() const {
    ParametriApex p;
    p.circuito = circuito_->currentData().toString();
    p.sezioni = sezioni_->value();
    p.popolazione = popolazione_->value();
    p.generazioni = generazioni_->value();
    p.crossover = crossover_->value();
    p.mutazione = mutazione_->value();
    p.intensita = intensita_->value();
    p.torneo = torneo_->value();
    p.elite = elite_->value();
    p.seed = static_cast<unsigned int>(seed_->value());
    p.massaKg = massa_->value();
    p.potenzaKW = potenza_->value();
    p.accelerazione = accelerazione_->value();
    p.frenata = frenata_->value();
    p.cd = cd_->value();
    p.areaM2 = area_->value();
    p.velocitaKmh = velocita_->value();
    p.accelerazioneLaterale = accelerazioneLaterale_->value();
    p.grip = grip_->value();
    return p;
}

void ApexMainWindow::aggiornaPista() {
    try {
        const auto punti = caricaCentroPista(percorsoCircuito());
        pista_->mostraGeometria(creaGeometriaPista(punti, 150));
        grafico_->impostaGeni({}, {});
        for (auto *g : {graficoVelocita_, graficoAccelerazione_, graficoFrenata_,
                        graficoLaterale_, graficoCurvatura_})
            g->impostaSerie({}, {}, {}, {});
        risultato_.reset();
        esportaJson_->setEnabled(false);
        esportaCsv_->setEnabled(false);
        tempoIniziale_->setText("— s"); tempoFinale_->setText("— s");
        miglioramento_->setText("— %"); durata_->setText("— ms");
        infoPista_->setText(circuito_->currentData().toString() == "spa" ?
                              "BELGIO · 7.004 km · 12 m stimati" :
                              "ITALIA · 5.793 km · 12 m stimati");
        stato_->setText("PRONTO  ·  TRACCIATO CARICATO");
    } catch (const std::exception &e) {
        stato_->setText("ERRORE CARICAMENTO PISTA");
        QMessageBox::critical(this, "Errore circuito", QString::fromUtf8(e.what()));
    }
}

void ApexMainWindow::avviaOttimizzazione() {
    if (lavoro_ && lavoro_->isRunning()) return;
    const auto p = leggiParametri();
    if (p.torneo > p.popolazione || p.elite >= p.popolazione) {
        QMessageBox::warning(this, "Parametri non validi", "Dimensione torneo o numero élite fuori dai limiti.");
        return;
    }
    const QString file = percorsoCircuito();
    bottoneAvvio_->setEnabled(false);
    circuito_->setEnabled(false);
    attivita_->setRange(0, p.generazioni);
    attivita_->setValue(0);
    attivita_->setVisible(true);
    stimaRimanente_->setVisible(true);
    stimaRimanente_->setText("Preparazione e valutazione della popolazione iniziale...  ·  stima in arrivo");
    stato_->setText("CALCOLO IN CORSO  ·  MOTORE C++ SEQUENZIALE");

    // Il worker non accede mai ai widget. Pubblica soltanto valori atomici.
    struct StatoAsync {
        std::optional<RisultatoApex> valore;
        QString errore;
        std::atomic<int> completate{0};
        std::atomic<qint64> microsecondi{0};
    };
    const auto esito = std::make_shared<StatoAsync>();
    QThread *thread = QThread::create([p, file, esito] {
        try {
            esito->valore = eseguiOttimizzazioneApex(p, file,
                [esito](int completate, int /*totale*/, double secondi) {
                    const auto micros = static_cast<qint64>(secondi * 1000000.0);
                    esito->microsecondi.store(micros, std::memory_order_relaxed);
                    esito->completate.store(completate, std::memory_order_release);
                });
        } catch (const std::exception &e) {
            esito->errore = QString::fromUtf8(e.what());
        } catch (...) {
            esito->errore = "Errore sconosciuto durante l'ottimizzazione";
        }
    });
    lavoro_ = thread;

    // Lettura ogni 250 ms sul thread principale, anche durante calcoli lunghi.
    auto *aggiornaProgresso = new QTimer(this);
    aggiornaProgresso->setInterval(250);
    connect(aggiornaProgresso, &QTimer::timeout, this, [this, esito, p] {
        const int completate = std::clamp(esito->completate.load(std::memory_order_acquire),
                                          0, p.generazioni);
        attivita_->setValue(completate);
        const qint64 us = esito->microsecondi.load(std::memory_order_relaxed);
        if (completate < 3 || us <= 0) {
            stimaRimanente_->setText("Calibrazione della stima...  ·  " +
                                      QString::number(completate) + "/" +
                                      QString::number(p.generazioni) + " generazioni");
            return;
        }
        // ETA media sulle generazioni completate: dipende dal carico del PC.
        const double rimanente = (double(p.generazioni - completate) / double(completate))
                                  * (double(us) / 1000000.0);
        QString testo = QString("%1 / %2 generazioni  ·  tempo rimanente ≈ %3")
            .arg(completate).arg(p.generazioni).arg(durataStimata(rimanente));
        if (rimanente < 86400.0 && std::isfinite(rimanente))
            testo += "  ·  fine stimata " +
                     QDateTime::currentDateTime().addSecs(qint64(rimanente)).toString("HH:mm:ss");
        stimaRimanente_->setText(testo);
        stato_->setText(QString("OTTIMIZZAZIONE IN CORSO  ·  %1 / %2 GENERAZIONI")
                            .arg(completate).arg(p.generazioni));
    });
    aggiornaProgresso->start();
    connect(thread, &QThread::finished, this, [this, esito, aggiornaProgresso] {
        aggiornaProgresso->stop();
        aggiornaProgresso->deleteLater();
        lavoro_ = nullptr;
        bottoneAvvio_->setEnabled(true);
        circuito_->setEnabled(true);
        attivita_->setVisible(false);
        stimaRimanente_->setVisible(false);
        if (esito->valore) {
            mostraRisultato(*esito->valore);
        } else {
            stato_->setText("ERRORE DURANTE L'OTTIMIZZAZIONE");
            QMessageBox::critical(this, "Ottimizzazione non riuscita", esito->errore);
        }
    });
    connect(thread, &QThread::finished, thread, &QObject::deleteLater);
    thread->start();
}

void ApexMainWindow::mostraRisultato(const RisultatoApex &r) {
    risultato_ = r;
    pista_->mostraRisultato(r);
    grafico_->impostaGeni(r.geniIniziali, r.geniOttimizzati);
    graficoVelocita_->impostaSerie(r.distanzaInizialeKm, r.velocitaInizialeKmh,
                                   r.distanzaOttimizzataKm, r.velocitaOttimizzataKmh);
    graficoAccelerazione_->impostaSerie(r.distanzaInizialeKm, r.accelerazioneIniziale,
                                        r.distanzaOttimizzataKm, r.accelerazioneOttimizzata);
    graficoFrenata_->impostaSerie(r.distanzaInizialeKm, r.frenataIniziale,
                                  r.distanzaOttimizzataKm, r.frenataOttimizzata);
    graficoLaterale_->impostaSerie(r.distanzaInizialeKm, r.lateraleIniziale,
                                   r.distanzaOttimizzataKm, r.lateraleOttimizzata);
    graficoCurvatura_->impostaSerie(r.distanzaInizialeKm, r.curvaturaInizialePerKm,
                                    r.distanzaOttimizzataKm, r.curvaturaOttimizzataPerKm);
    pagine_->setCurrentIndex(1);
    const double percentuale = r.tempoIniziale > 0 ?
        100.0 * (r.tempoIniziale - r.tempoFinale) / r.tempoIniziale : 0.0;
    tempoIniziale_->setText(QString::number(r.tempoIniziale, 'f', 3) + " s");
    tempoFinale_->setText(QString::number(r.tempoFinale, 'f', 3) + " s");
    miglioramento_->setText(QString::number(percentuale, 'f', 2) + " %");
    durata_->setText(QString::number(r.durataMs, 'f', 2) + " ms");
    esportaJson_->setEnabled(true);
    esportaCsv_->setEnabled(true);
    stato_->setText(QString("COMPLETATO  ·  %1 GENERAZIONI  /  %2 INDIVIDUI  /  SEED %3")
                        .arg(r.parametri.generazioni).arg(r.parametri.popolazione).arg(r.parametri.seed));
}

void ApexMainWindow::esportaRisultatoJson() {
    if (!risultato_) return;
    const QString file = QFileDialog::getSaveFileName(this, "Esporta risultato JSON",
                                                       "apex_lab_" + risultato_->parametri.circuito + ".json",
                                                       "JSON (*.json)");
    if (file.isEmpty()) return;
    const RisultatoApex &r = *risultato_;
    QJsonObject root;
    root["motore"] = "AlgoritmoGenetico C++";
    root["circuito"] = r.parametri.circuito;
    root["tempoInizialeSecondi"] = r.tempoIniziale;
    root["tempoOttimizzatoSecondi"] = r.tempoFinale;
    root["durataCalcoloMs"] = r.durataMs;
    QJsonObject params;
    params["sezioni"] = r.parametri.sezioni;
    params["popolazione"] = r.parametri.popolazione;
    params["generazioni"] = r.parametri.generazioni;
    params["crossover"] = r.parametri.crossover;
    params["mutazione"] = r.parametri.mutazione;
    params["intensita"] = r.parametri.intensita;
    params["torneo"] = r.parametri.torneo;
    params["elite"] = r.parametri.elite;
    params["seed"] = int(r.parametri.seed);
    root["parametri"] = params;
    root["bordoSinistro"] = puntiJson(r.geometria.sinistro);
    root["bordoDestro"] = puntiJson(r.geometria.destro);
    root["traiettoriaIniziale"] = puntiJson(r.iniziale);
    root["traiettoriaOttimizzata"] = puntiJson(r.ottimizzata);
    root["geniIniziali"] = geniJson(r.geniIniziali);
    root["geniOttimizzati"] = geniJson(r.geniOttimizzati);
    root["distanzaInizialeKm"] = geniJson(r.distanzaInizialeKm);
    root["distanzaOttimizzataKm"] = geniJson(r.distanzaOttimizzataKm);
    root["velocitaInizialeKmh"] = geniJson(r.velocitaInizialeKmh);
    root["velocitaOttimizzataKmh"] = geniJson(r.velocitaOttimizzataKmh);
    root["accelerazioneIniziale"] = geniJson(r.accelerazioneIniziale);
    root["accelerazioneOttimizzata"] = geniJson(r.accelerazioneOttimizzata);
    root["frenataIniziale"] = geniJson(r.frenataIniziale);
    root["frenataOttimizzata"] = geniJson(r.frenataOttimizzata);
    root["accelerazioneLateraleIniziale"] = geniJson(r.lateraleIniziale);
    root["accelerazioneLateraleOttimizzata"] = geniJson(r.lateraleOttimizzata);
    root["curvaturaInizialePerKm"] = geniJson(r.curvaturaInizialePerKm);
    root["curvaturaOttimizzataPerKm"] = geniJson(r.curvaturaOttimizzataPerKm);
    QFile out(file);
    if (!out.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        QMessageBox::critical(this, "Errore esportazione", "Impossibile scrivere il file JSON");
        return;
    }
    out.write(QJsonDocument(root).toJson(QJsonDocument::Indented));
    stato_->setText("RISULTATO ESPORTATO IN JSON");
}

void ApexMainWindow::esportaRisultatoCsv() {
    if (!risultato_) return;
    const QString file = QFileDialog::getSaveFileName(this, "Esporta traiettoria CSV",
                                                       "apex_lab_" + risultato_->parametri.circuito + ".csv",
                                                       "CSV (*.csv)");
    if (file.isEmpty()) return;
    QFile out(file);
    if (!out.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
        QMessageBox::critical(this, "Errore esportazione", "Impossibile scrivere il file CSV");
        return;
    }
    QTextStream stream(&out);
    stream << "indice,x_iniziale_m,y_iniziale_m,x_ottimizzata_m,y_ottimizzata_m,gene_iniziale,gene_ottimizzato,";
    stream << "distanza_iniziale_km,distanza_ottimizzata_km,velocita_iniziale_kmh,velocita_ottimizzata_kmh,";
    stream << "accelerazione_iniziale_m_s2,accelerazione_ottimizzata_m_s2,frenata_iniziale_m_s2,frenata_ottimizzata_m_s2,";
    stream << "acc_laterale_iniziale_m_s2,acc_laterale_ottimizzata_m_s2,";
    stream << "curvatura_iniziale_1_km,curvatura_ottimizzata_1_km\n";
    const auto &r = *risultato_;
    for (qsizetype i = 0; i < r.ottimizzata.size(); ++i) {
        stream << i << ',' << r.iniziale[i].x() << ',' << r.iniziale[i].y() << ','
               << r.ottimizzata[i].x() << ',' << r.ottimizzata[i].y() << ','
               << r.geniIniziali[i] << ',' << r.geniOttimizzati[i] << ','
               << r.distanzaInizialeKm[i] << ',' << r.distanzaOttimizzataKm[i] << ','
               << r.velocitaInizialeKmh[i] << ',' << r.velocitaOttimizzataKmh[i] << ','
               << r.accelerazioneIniziale[i] << ',' << r.accelerazioneOttimizzata[i] << ','
               << r.frenataIniziale[i] << ',' << r.frenataOttimizzata[i] << ','
               << r.lateraleIniziale[i] << ',' << r.lateraleOttimizzata[i] << ','
               << r.curvaturaInizialePerKm[i] << ',' << r.curvaturaOttimizzataPerKm[i] << '\n';
    }
    stato_->setText("TRAIETTORIE ESPORTATE IN CSV");
}
