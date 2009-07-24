//
// This file is part of the Marble Desktop Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2008 Torsten Rahn <tackat@kde.org>"
//

#ifndef MAPSCALEFLOATITEM_H
#define MAPSCALEFLOATITEM_H

#include <QtCore/QObject>

#include "AbstractFloatItem.h"

namespace Marble
{

/**
 * @short The class that creates a map scale.
 *
 */

class MapScaleFloatItem : public AbstractFloatItem
{
    Q_OBJECT
    Q_INTERFACES( Marble::RenderPluginInterface )
    MARBLE_PLUGIN( MapScaleFloatItem )
 public:
    explicit MapScaleFloatItem( const QPointF &point = QPointF( 10.5, -10.5 ),
                                const QSizeF &size = QSizeF( 0.0, 40.0 ) );
    ~MapScaleFloatItem();

    QStringList backendTypes() const;

    QString name() const;

    QString guiString() const;

    QString nameId() const;

    QString description() const;

    QIcon icon () const;


    void initialize ();

    bool isInitialized () const;

    void changeViewport( ViewportParams *viewport );

    void paintContent( GeoPainter *painter, ViewportParams *viewport,
                       const QString& renderPos, GeoSceneLayer * layer = 0 );

 private:
    int   invScale() const            { return m_invScale; }
    void  setInvScale( int invScale ) { m_invScale = invScale; }

    int      m_radius;
    int      m_invScale;

    QString  m_target;

    int      m_leftBarMargin;
    int      m_rightBarMargin;
    int      m_scaleBarWidth;
    int      m_viewportWidth;
    int      m_scaleBarHeight;
    qreal    m_scaleBarDistance;

    int      m_bestDivisor;
    int      m_pixelInterval;
    int      m_valueInterval;

    QString  m_unit;

    bool     m_scaleInitDone;

    void calcScaleBar();
};

}

#endif // MAPSCALEFLOATITEM_H
