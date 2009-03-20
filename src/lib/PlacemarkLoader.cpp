//
// This file is part of the Marble Desktop Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2008      Patrick Spendrin <ps_ml@gmx.de>
//

#include "PlacemarkLoader.h"

#include <QtCore/QBuffer>
#include <QtCore/QDataStream>
#include <QtCore/QDateTime>
#include <QtCore/QDebug>
#include <QtCore/QFile>
#include <QtCore/QThread>

#include "GeoDataParser.h"
#include "GeoSceneDocument.h"
#include "GeoDataDocument.h"
#include "GeoDataPlacemark.h"
#include "MarbleDirs.h"
#include "MarblePlacemarkModel.h"
#include "PlacemarkContainer.h"


namespace Marble {

PlacemarkLoader::PlacemarkLoader( QObject* parent, const QString& file )
 : QThread( parent ), 
   m_filepath( file ),
   m_contents( QString() )
{
}

PlacemarkLoader::PlacemarkLoader( QObject* parent, const QString& contents, const QString& file )
 : QThread( parent ), 
   m_filepath( file ), 
   m_contents( contents )
{
}

void PlacemarkLoader::run()
{
if( m_contents.isEmpty() ) {
    QString defaultcachename;
    QString defaultsrcname;
    QString defaulthomecache;

    PlacemarkContainer *container = new PlacemarkContainer( m_filepath );
    
    if( m_filepath.endsWith(".kml") ) {
        m_filepath.remove(QRegExp("\\.kml$"));
    }
    qDebug() << "starting parser for" << m_filepath;

    QFileInfo fileinfo(m_filepath);
    if ( fileinfo.isAbsolute() ) {
        // We got an _absolute_ path now: e.g. "/patrick.kml"
        defaultcachename = m_filepath + ".cache";
        defaultsrcname   = m_filepath + ".kml";
    }
    else {
        if ( m_filepath.contains( '/' ) ) {
            // _relative_ path: "maps/mars/viking/patrick.kml" 
            defaultcachename = MarbleDirs::path( m_filepath + ".cache" );
            defaultsrcname   = MarbleDirs::path( m_filepath + ".kml");
            defaulthomecache = MarbleDirs::localPath() + m_filepath + ".cache";
        }
        else {
            // _standard_ shared placemarks: "placemarks/patrick.kml"
            defaultcachename = MarbleDirs::path( "placemarks/" + m_filepath + ".cache" );
            defaultsrcname   = MarbleDirs::path( "placemarks/" + m_filepath + ".kml");
            defaulthomecache = MarbleDirs::localPath() + "/placemarks/" + m_filepath + ".cache";
        }
    }

    if ( QFile::exists( defaultcachename ) ) {
        qDebug() << "Loading Default Placemark Cache File:" + defaultcachename;

        bool      cacheoutdated = false;
        QDateTime sourceLastModified;
        QDateTime cacheLastModified;

        if ( QFile::exists( defaultsrcname ) ) {
            sourceLastModified = QFileInfo( defaultsrcname ).lastModified();
            cacheLastModified  = QFileInfo( defaultcachename ).lastModified();

            if ( cacheLastModified < sourceLastModified )
                cacheoutdated = true;
        }

        bool loadok = false;

        if ( cacheoutdated == false ) {
            loadok = loadFile( defaultcachename, container );
            if ( loadok )
                emit placemarksLoaded( this, container );
        }
        qDebug() << "Loading ended" << loadok;
        if ( loadok == true )
            return;
    }

    qDebug() << "No recent Default Placemark Cache File available for " << m_filepath;

    if ( QFile::exists( defaultsrcname ) ) {

        // Read the KML file.
        importKml( defaultsrcname, container );

        qDebug() << "ContainerSize for" << m_filepath << ":" << container->size();
        // Save the contents in the efficient cache format.
        saveFile( defaulthomecache, container );

        // ...and finally add it to the PlacemarkContainer
        emit placemarksLoaded( this, container );
    }
    else {
        qDebug() << "No Default Placemark Source File for " << m_filepath;
        emit placemarkLoaderFailed( this );
    }
} else {
    PlacemarkContainer *container = new PlacemarkContainer( m_filepath );

    // Read the KML Data
    importKmlFromData( container );

    emit placemarksLoaded( this, container );
}

}

const quint32 MarbleMagicNumber = 0x31415926;

void PlacemarkLoader::importKml( const QString& filename,
                                 PlacemarkContainer* placemarkContainer )
{
    GeoDataParser parser( GeoData_KML );

    QFile file( filename );
    if ( !file.exists() ) {
        qWarning( "File does not exist!" );
        return;
    }

    // Open file in right mode
    file.open( QIODevice::ReadOnly );

    if ( !parser.read( &file ) ) {
        qWarning( "Could not parse file!" );
        return;
    }
    GeoDocument* document = parser.releaseDocument();
    Q_ASSERT( document );

    GeoDataDocument *dataDocument = static_cast<GeoDataDocument*>( document );
    dataDocument->setFileName( m_filepath );
    *placemarkContainer = PlacemarkContainer( dataDocument->placemarks(), 
                                              m_filepath );

    file.close();

    emit newGeoDataDocumentAdded( dataDocument );
}

void PlacemarkLoader::importKmlFromData( PlacemarkContainer* placemarkContainer )
{
    GeoDataParser parser( GeoData_KML );

    QByteArray ba( m_contents.toUtf8() );
    QBuffer buffer( &ba );
    buffer.open( QIODevice::ReadOnly );

    if ( !parser.read( &buffer ) ) {
        qWarning( "Could not parse buffer!" );
        return;
    }
    GeoDocument* document = parser.releaseDocument();
    Q_ASSERT( document );

    GeoDataDocument *dataDocument = static_cast<GeoDataDocument*>( document );
    dataDocument->setFileName( m_filepath );
    *placemarkContainer = PlacemarkContainer( dataDocument->placemarks(), 
                                              m_filepath );

    buffer.close();

    emit newGeoDataDocumentAdded( dataDocument );
}

void PlacemarkLoader::saveFile( const QString& filename,
                                 PlacemarkContainer* placemarkContainer )
{
    if ( QDir( MarbleDirs::localPath() + "/placemarks/" ).exists() == false )
        ( QDir::root() ).mkpath( MarbleDirs::localPath() + "/placemarks/" );

    QFile file( filename );
    file.open( QIODevice::WriteOnly );
    QDataStream out( &file );

    // Write a header with a "magic number" and a version
    // out << (quint32)0xA0B0C0D0;
    out << (quint32)MarbleMagicNumber;
    out << (qint32)014;

    out.setVersion( QDataStream::Qt_4_2 );

    qreal lon;
    qreal lat;
    qreal alt;

    PlacemarkContainer::const_iterator it = placemarkContainer->constBegin();
    PlacemarkContainer::const_iterator const end = placemarkContainer->constEnd();
    for (; it != end; ++it )
    {
        out << (*it)->name();
        (*it)->coordinate( lon, lat, alt );

        out << lon << lat << alt;
        out << QString( (*it)->role() );
        out << QString( (*it)->description() );
        out << QString( (*it)->countryCode() );
        out << (qreal)(*it)->area();
        out << (qint64)(*it)->population();
}
    }

bool PlacemarkLoader::loadFile( const QString& filename,
                                 PlacemarkContainer* placemarkContainer )
{
    QFile file( filename );
    file.open( QIODevice::ReadOnly );
    QDataStream in( &file );

    // Read and check the header
    quint32 magic;
    in >> magic;
    if ( magic != MarbleMagicNumber ) {
        qDebug( "Bad file format!" );
        return false;
    }

    // Read the version
    qint32 version;
    in >> version;
    if ( version < 014 ) {
        qDebug( "Bad file - too old!" );
        return false;
    }
    /*
      if (version > 002) {
      qDebug( "Bad file - too new!" );
      return;
      }
    */

    in.setVersion( QDataStream::Qt_4_2 );

    // Read the data itself
    qreal   lon;
    qreal   lat;
    qreal   alt;
    qreal   area;

    QString  tmpstr;
    qint64   tmpint64;

    QString testo;

    GeoDataPlacemark  *mark;
    while ( !in.atEnd() ) {
        mark = new GeoDataPlacemark;

        in >> tmpstr;
        mark->setName( tmpstr );
        testo = tmpstr;
        in >> lon >> lat >> alt;
        mark->setCoordinate( lon, lat, alt );
        in >> tmpstr;
        mark->setRole( tmpstr.at(0) );
        in >> tmpstr;
        mark->setDescription( tmpstr );
        in >> tmpstr;
        mark->setCountryCode( tmpstr );
        in >> area;
        mark->setArea( area );
        in >> tmpint64;
        mark->setPopulation( tmpint64 );

        placemarkContainer->append( mark );
    }

    return true;
}

#include "PlacemarkLoader.moc"
} // namespace Marble
