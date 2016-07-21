//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2010      Dennis Nienhüser <nienhueser@kde.org>
// Copyright 2012      Bernhard Beschow <bbeschow@cs.tu-berlin.de>
//

#include "GosmoreRoutingPlugin.h"
#include "GosmoreRoutingRunner.h"
#include "MarbleDirs.h"

#include <QDir>
#include <QFileInfo>

namespace Marble
{

GosmorePlugin::GosmorePlugin( QObject *parent ) :
    RoutingRunnerPlugin( parent )
{
    setSupportedCelestialBodies( QStringList() << "earth" );
    setCanWorkOffline( true );
}

QString GosmorePlugin::name() const
{
    return tr( "Gosmore Routing" );
}

QString GosmorePlugin::guiString() const
{
    return tr( "Gosmore" );
}

QString GosmorePlugin::nameId() const
{
    return "gosmore-routing";
}

QString GosmorePlugin::version() const
{
    return "1.0";
}

QString GosmorePlugin::description() const
{
    return tr( "Offline route retrieval using Gosmore" );
}

QString GosmorePlugin::copyrightYears() const
{
    return "2010, 2012";
}

QVector<PluginAuthor> GosmorePlugin::pluginAuthors() const
{
    return QVector<PluginAuthor>()
            << PluginAuthor( QString::fromUtf8( "Dennis Nienhüser" ), "nienhueser@kde.org" )
            << PluginAuthor( "Bernhard Beschow", "bbeschow@cs.tu-berlin.de" );
}

RoutingRunner *GosmorePlugin::newRunner() const
{
    return new GosmoreRunner;
}

bool GosmorePlugin::canWork() const
{
    QDir mapDir( MarbleDirs::localPath() + "/maps/earth/gosmore/" );
    QFileInfo mapFile = QFileInfo ( mapDir, "gosmore.pak" );
    return mapFile.exists();
}

bool GosmorePlugin::supportsTemplate( RoutingProfilesModel::ProfileTemplate profileTemplate ) const
{
    return profileTemplate == RoutingProfilesModel::CarFastestTemplate;
}

}

#include "moc_GosmoreRoutingPlugin.cpp"
