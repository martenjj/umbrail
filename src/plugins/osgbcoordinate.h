// -*-mode:c++ -*-

#ifndef OSGBCOORDINATE_H
#define OSGBCOORDINATE_H


#include <qlabel.h>

#include "abstractcoordinatehandler.h"


class QComboBox;
class QLineEdit;

struct OSGBRef;


class OSGBCoordinateHandler : public AbstractCoordinateHandler
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.kde.navtracks.AbstractCoordinateHandler")
    Q_INTERFACES(AbstractCoordinateHandler)

public:
    PLUGIN_EXPORT OSGBCoordinateHandler(QObject *pnt = nullptr);
    virtual PLUGIN_EXPORT ~OSGBCoordinateHandler() = default;

    PLUGIN_EXPORT QWidget *createWidget(QWidget *pnt = nullptr) override;
    PLUGIN_EXPORT bool hasAcceptableInput() const override;
    PLUGIN_EXPORT QString tabName() const override;

protected:
    void updateGUI(double lat, double lon) override;

private slots:
    void slotReferenceChanged();
    void slotCoordinateChanged();
    void slotSelectSquare();
    void slotReferencePicked(const QByteArray &ref);

private:
    void setOSGB(double lat, double lon) const;
    void setOSGBToEN(const OSGBRef &ref) const;
    void setOSGBToRef(const OSGBRef &ref) const;
    void checkError() override;

private:
    QComboBox *mLetterCombo;
    QLineEdit *mReferenceEdit;
    QLineEdit *mNorthEdit;
    QLineEdit *mEastEdit;
};


class OSGBCoordinatePicker : public QLabel
{
    Q_OBJECT

public:
    OSGBCoordinatePicker(QWidget *pnt = nullptr);
    virtual ~OSGBCoordinatePicker() = default;

protected:
    void keyPressEvent(QKeyEvent *ev) override;
    void mousePressEvent(QMouseEvent *ev) override;
    void mouseMoveEvent(QMouseEvent *ev) override;
    void paintEvent(QPaintEvent *ev) override;
    void hideEvent(QHideEvent *ev) override;

signals:
    void referenceSelected(const QByteArray &ref);

private:
    QByteArray posToGridRef(const QPoint &pos) const;
    QByteArray squareToGridRef(int sqx, int sqy) const;

    void moveHighlightBy(int dx, int dy);
    void moveHighlightStep(int d);

private:
    int mSquaresWidth;
    int mSquaresHeight;

    int mHighlightSquareX;
    int mHighlightSquareY;

};

#endif							// OSGBCOORDINATE_H
