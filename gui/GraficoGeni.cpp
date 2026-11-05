#include "GraficoGeni.hpp"
#include <QPainter>
#include <QPainterPath>
#include <QFont>
#include <algorithm>

GraficoGeni::GraficoGeni(QWidget *parent) : QWidget(parent) { setMinimumHeight(145); }
void GraficoGeni::impostaGeni(const QVector<double> &ini, const QVector<double> &fin) {
    iniziali_ = ini;
    finali_ = fin;
    update();
}
void GraficoGeni::paintEvent(QPaintEvent*) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    p.fillRect(rect(), QColor("#0D1726"));
    const QRectF area(42, 18, width()-64, height()-40);
    if (area.width() <= 0 || area.height() <= 0) return;
    p.setFont(QFont("Sans Serif", 9));
    for (int y = 0; y <= 4; ++y) {
        double valore = y * .25;
        double yy = area.bottom() - valore * area.height();
        p.setPen(QPen(QColor("#25364C"), 1));
        p.drawLine(QPointF(area.left(), yy), QPointF(area.right(), yy));
        p.setPen(QColor("#7E91AA"));
        p.drawText(QRectF(0, yy-9, 35, 18), Qt::AlignRight, QString::number(valore, 'f', 2));
    }
    if (iniziali_.isEmpty() && finali_.isEmpty()) {
        p.setPen(QColor("#7E91AA"));
        p.drawText(area, Qt::AlignCenter, "Esegui l'ottimizzazione per visualizzare i geni");
        return;
    }
    const auto disegna = [&](const QVector<double> &geni, QColor colore, Qt::PenStyle stile) {
        if (geni.size() < 2) return;
        QPainterPath path;
        for (qsizetype i = 0; i < geni.size(); ++i) {
            const QPointF pt(area.left() + area.width() * double(i) / (geni.size()-1),
                             area.bottom() - area.height() * std::clamp(geni[i], 0.0, 1.0));
            if (i == 0) path.moveTo(pt); else path.lineTo(pt);
        }
        p.setPen(QPen(colore, 1.9, stile));
        p.drawPath(path);
    };
    disegna(iniziali_, QColor("#F4A261"), Qt::DashLine);
    disegna(finali_, QColor("#21D8D5"), Qt::SolidLine);
    p.setPen(QColor("#7E91AA"));
    p.drawText(QRectF(area.left(), area.bottom() + 6, area.width(), 18), Qt::AlignCenter,
               QString("SEZIONI DEL CIRCUITO · %1 GENI").arg(finali_.isEmpty() ? iniziali_.size() : finali_.size()));
}
