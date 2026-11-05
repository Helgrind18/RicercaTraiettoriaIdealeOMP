#ifndef APEX_LAB_GRAFICO_GENI_HPP
#define APEX_LAB_GRAFICO_GENI_HPP
#include <QWidget>
#include <QVector>
class GraficoGeni : public QWidget {
public:
    explicit GraficoGeni(QWidget *parent = nullptr);
    void impostaGeni(const QVector<double> &iniziali, const QVector<double> &finali);
protected:
    void paintEvent(QPaintEvent *event) override;
private:
    QVector<double> iniziali_;
    QVector<double> finali_;
};
#endif
