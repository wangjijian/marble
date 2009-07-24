//
// This file is part of the Marble Desktop Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2009      Bastian Holst <bastianholst@gmx.de>
//

#ifndef MARBLEGRAPHICSITEMPRIVATE_H
#define MARBLEGRAPHICSITEMPRIVATE_H

// Marble
#include "AbstractProjection.h"
#include "GeoPainter.h"

// Qt
#include<QtCore/QList>
#include<QtCore/QSet>
#include<QtCore/QSize>
#include<QtCore/QSizeF>
#include<QtCore/QRect>

namespace Marble {

class MarbleGraphicsItemPrivate {
 public:
    MarbleGraphicsItemPrivate( MarbleGraphicsItem *marbleGraphicsItem,
                               MarbleGraphicsItem *parent = 0 )
        : m_removeCachedPixmap( false ),
          m_cacheMode( MarbleGraphicsItem::NoCache ),
          m_visibility( true ),
          m_parent( parent ),
          m_children( 0 ),
          m_marbleGraphicsItem( marbleGraphicsItem )
    {
        if ( m_parent ) {
            m_parent->p()->addChild( m_marbleGraphicsItem );
        }
    }
    
    virtual ~MarbleGraphicsItemPrivate() {
        // Delete all children
        if ( m_children ) {
            qDeleteAll( *m_children );
            m_children->clear();
        }

        // Remove from parent
        if ( m_parent ) {
            m_parent->p()->removeChild( m_marbleGraphicsItem );
        }
    }

    void addChild( MarbleGraphicsItem *child ) {
        if ( m_children == 0 ) {
            m_children = new QSet<MarbleGraphicsItem *>();
        }

        m_children->insert( child );
    }

    void removeChild( MarbleGraphicsItem *child ) {
        if ( m_children ) {
            m_children->remove( child );
        }
    }
     
    virtual QList<QPointF> positions() {
        return QList<QPointF>();
    }
    
    void ensureValidCacheKey() {
        if( m_cacheKey.isNull() ) {
            static unsigned int key = 0;
            m_cacheKey = QString( "MarbleGraphicsItem:" ) + QString::number( key );
            key++;
        }
    }
    
    QList<QRectF> boundingRects() {
        QList<QRectF> list;
        
        foreach( QPointF point, positions() ) {
            QRectF rect( point, m_size );
            if( rect.x() < 0 )
                rect.setLeft( 0 );
            if( rect.y() < 0 )
                rect.setTop( 0 );
            
            list.append( rect );
        }
        
        return list;
    }
    
    virtual void setProjection( AbstractProjection *projection, ViewportParams *viewport ) {
        Q_UNUSED( projection );
        Q_UNUSED( viewport );
    }
    
    QSizeF m_size;
    QSize m_logicalCacheSize;
    
    bool m_removeCachedPixmap;
    
    MarbleGraphicsItem::CacheMode m_cacheMode;
    
    // TODO: Substitute this by QPixmapCache::Key once it is available.
    QString m_cacheKey;

    bool m_visibility;

    // The parent of the item
    MarbleGraphicsItem *m_parent;
    // The set of children. WARNING: This is not initialized by default.
    QSet<MarbleGraphicsItem *> *m_children;

    MarbleGraphicsItem *m_marbleGraphicsItem;
};

}

#endif // MARBLEGRAPHICSITEMPRIVATE_H
