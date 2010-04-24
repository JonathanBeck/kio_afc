/*
   Copyright (C) 2010 Jonathan Beck <jonabeck@gmail.com>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License (LGPL) as published by the Free Software Foundation;
   either version 2 of the License, or (at your option) any later
   version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "kio_afc.h"
#include <QtCore/QCoreApplication>
#include <QtCore/QDir>
#include <kcomponentdata.h>
#include <kglobal.h>
#include <kdebug.h>
#include <kurl.h>

#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#include <grp.h>


#define KIO_AFC 7002

static void device_callback(const idevice_event_t *event, void *user_data)
{
    AfcProtocol* afcProto = static_cast<AfcProtocol*>(user_data);
    afcProto->ProcessEvent(event);
}


using namespace KIO;

extern "C" int KDE_EXPORT kdemain( int argc, char **argv )
{
    QCoreApplication app( argc, argv ); // needed for QSocketNotifier
    KComponentData componentData( "kio_afc" );
    ( void ) KGlobal::locale();

    if (argc != 4)
    {
        fprintf(stderr, "Usage: kio_afc protocol domain-socket1 domain-socket2\n");
        exit(-1);
    }

    AfcProtocol slave(argv[2], argv[3]);
    slave.dispatchLoop();

    kDebug(KIO_AFC) << "Done";
    return 0;
}

QString AfcProtocol::m_user = QString();
QString AfcProtocol::m_group = QString();

AfcProtocol::AfcProtocol( const QByteArray &pool, const QByteArray &app )
    : SlaveBase( "afc", pool, app ), _opened_device(NULL)
{
    //store current user and group since AFC does not handle permissions
    struct passwd *user = getpwuid( getuid() );
    if ( user )
        m_user = QString::fromLatin1(user->pw_name);

    struct group *grp = getgrgid( getgid() );
    if ( grp )
        m_group = QString::fromLatin1(grp->gr_name);

    char** devices = NULL;
    int nbDevices = 0;

    idevice_get_device_list (&devices, &nbDevices);

    for (int i = 0; i < nbDevices; i++)
    {
        AfcDevice* dev = new AfcDevice ( devices[i], this );
        if (dev->isValid())
        {
            _devices.insert( QString(devices[i]), dev);
        }
        else
            delete dev;
    }

    idevice_device_list_free (devices);

    idevice_event_subscribe (device_callback, this);
}

AfcProtocol::~AfcProtocol()
{
    QHash<QString, AfcDevice*>::const_iterator i = _devices.constBegin();
    while (i != _devices.constEnd()) {
        delete i.value();
        ++i;
    }
    idevice_event_unsubscribe ();
}

void AfcProtocol::ProcessEvent(const idevice_event_t *event)
{
    if ( IDEVICE_DEVICE_ADD == event->event )
    {
        kDebug(KIO_AFC) << "IDEVICE_DEVICE_ADD";

        AfcDevice* dev = new AfcDevice ( event->uuid, this );
        if (dev->isValid())
        {
            _devices.insert( QString(event->uuid), dev);
        }
        else
            delete dev;
    }
    else if  ( IDEVICE_DEVICE_REMOVE == event->event )
    {
        kDebug(KIO_AFC) << "IDEVICE_DEVICE_REMOVE";

        AfcDevice* dev = _devices[QString(event->uuid)];
        delete dev;
        _devices.remove(QString(event->uuid));
    }
}

AfcPath AfcProtocol::checkURL( const KUrl& url )
{
    kDebug(KIO_AFC) << "checkURL " << url;
    QString surl = url.url();
    if (surl.startsWith(QLatin1String("afc:")))
    {
        if (surl.length() == 4)
        {
            // just the above
            kDebug(KIO_AFC) << "checkURL return1 " << surl << " " << url;
            return AfcPath("","");
        }

        QString path = QDir::cleanPath( url.path() );
        AfcPath p ( path.mid(1,40), path.mid(41) );
        if ( p.m_path == "" ) p.m_path = "/"; //make sure we start at root
        kDebug(KIO_AFC) << "checkURL return2 " << surl << " " << p;
        return p;
    }

    kDebug(KIO_AFC) << "checkURL return 3 ";
    return AfcPath("","");
}


void AfcProtocol::get( const KUrl& url )
{
    kDebug(KIO_AFC) << url;

    // check (correct) URL
    const AfcPath& path= checkURL(url);

    kDebug(KIO_AFC) << "path :" << path;
    //root case
    if ( path.isRoot() )
    {
        error(KIO::ERR_IS_DIRECTORY, "/");
        return;
    }

    AfcDevice* dev = _devices[path.m_host];

    if ( NULL != dev )
    {
        KIO::Error err;
        if ( !dev->get(path.m_path, err) )
        {
            error (err, path.m_path);
            return;
        }
    }
    else
    {
        error (KIO::ERR_DOES_NOT_EXIST, path.m_path);
        return;
    }

    finished();
}



void AfcProtocol::put( const KUrl& url, int _mode,
                       KIO::JobFlags _flags )
{
    kDebug(KIO_AFC) << url;

    // check (correct) URL
    const AfcPath& path= checkURL(url);

    kDebug(KIO_AFC) << "path :" << path;
    //root case
    if ( path.isRoot() )
    {
        error(KIO::ERR_IS_DIRECTORY, "/");
        return;
    }

    AfcDevice* dev = _devices[path.m_host];

    if ( NULL != dev )
    {
        KIO::Error err;
        if ( !dev->put(path.m_path, _flags, err ) )
        {
            error (err, path.m_path);
            return;
        }
    }
    else
    {
        error (KIO::ERR_DOES_NOT_EXIST, path.m_path);
        return;
    }

    finished();
}

void AfcProtocol::copy( const KUrl &src, const KUrl &dest,
                        int mode, KIO::JobFlags flags )
{
    kDebug(KIO_AFC) << src << "to " << dest;
}

void AfcProtocol::rename( const KUrl &src, const KUrl &dest,
                          KIO::JobFlags flags )
{
    kDebug(KIO_AFC) << src << "to " << dest;

    // check (correct) URL
    const AfcPath path_src = checkURL(src);
    const AfcPath path_dest = checkURL(dest);

    //make sue we are on the same device

    if ( path_src.m_host == path_dest.m_host )
    {
        AfcDevice* device = _devices[path_src.m_host];

        if ( NULL == device )
        {
            error(KIO::ERR_DOES_NOT_EXIST, "Could not find specified device");
            return;
        }

        KIO::Error err;
        if ( ! device->rename(path_src.m_path, path_dest.m_path, flags, err ) )
        {
            error(err, path_src.m_path);
            return;
        }
    }
    else
    {
        error ( KIO::ERR_CANNOT_RENAME, "Cannot rename on different device");
        return;
    }
    finished();
}

void AfcProtocol::symlink( const QString &target, const KUrl &dest,
                           KIO::JobFlags flags )
{
    kDebug(KIO_AFC) << target << "to " << dest;

    // check (correct) URL
    const AfcPath path_dest = checkURL(dest);

    //make sue we are on the same device

    if ( target.contains( path_dest.m_host, Qt::CaseSensitive ) )
    {
        AfcDevice* device = _devices[path_dest.m_host];

        if ( NULL == device )
        {
            error(KIO::ERR_DOES_NOT_EXIST, "Could not find specified device");
            return;
        }

        QString src = target;
        src.remove("/" + path_dest.m_host, Qt::CaseSensitive );

        KIO::Error err;
        if ( ! device->symlink(src, path_dest.m_path, flags, err ) )
        {
            error(err, target);
            return;
        }
    }
    else
    {
        error ( KIO::ERR_CANNOT_SYMLINK, "Cannot symlink on different device");
        return;
    }
    finished();
}


void AfcProtocol::stat( const KUrl& url )
{
    kDebug(KIO_AFC) << url;

    // check (correct) URL
    const AfcPath path = checkURL(url);

    kDebug(KIO_AFC) << "cleaned path=" << path;

    // We can't stat root, but we know it's a dir.
    if( path.isRoot() )
    {
        UDSEntry entry;
        //entry.insert( KIO::UDSEntry::UDS_NAME, UDSField( QString() ) );
        entry.insert( KIO::UDSEntry::UDS_NAME, QString::fromLatin1( "." ) );
        entry.insert( KIO::UDSEntry::UDS_FILE_TYPE, S_IFDIR );
        entry.insert( KIO::UDSEntry::UDS_ACCESS, S_IRUSR | S_IXUSR | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH );
        entry.insert( KIO::UDSEntry::UDS_USER, QString::fromLatin1( "root" ) );
        entry.insert( KIO::UDSEntry::UDS_GROUP, QString::fromLatin1( "root" ) );
        // no size

        statEntry( entry );
        finished();
        return;
    }

    AfcDevice* device = _devices[path.m_host];

    if ( NULL == device )
    {
        error(KIO::ERR_DOES_NOT_EXIST, "Could not find specified device");
        return;
    }


    KIO::Error err;
    if ( ! device->stat(url.fileName() ,path.m_path, err ) )
    {
        error(KIO::ERR_DOES_NOT_EXIST, path.m_path);
        return;
    }

    finished();
}

void AfcProtocol::listDir( const KUrl& url )
{
    kDebug(KIO_AFC) << url;

    // check (correct) URL
    const AfcPath path = checkURL(url);

    if ( path.isRoot() )
    {
        //root case if only one device plugged, then redirect
        //otherwise display all devices

        QHash<QString, AfcDevice*>::const_iterator i = _devices.constBegin();

//        if (_devices.size() == 1)
//        {
//            KUrl newUrl ( kurl.url() + "/" + i.key() );
//            redirection(newUrl);
//            finished();
//            return;
//        }
//        else
//        {
        while (i != _devices.constEnd())
        {
            AfcDevice* dev = i.value();
            UDSEntry entry;
            dev->createRootUDSEntry(entry);
            listEntry( entry, false );
            ++i;
        }
//        }
        listEntry(UDSEntry(), true);
    }
    else
    {
        AfcDevice* device = _devices[path.m_host];

        if ( NULL == device )
        {
            error(KIO::ERR_DOES_NOT_EXIST, "Could not find specified device");
            return;
        }

        KIO::Error err;
        if ( !device->listDir(path.m_path, err) )
        {
            error ( err, path.m_path );
            return;
        }
    }
    finished();
}

void AfcProtocol::mkdir( const KUrl& url, int permissions )
{
    kDebug(KIO_AFC) << url;

    // check (correct) URL
    const AfcPath path = checkURL(url);

    AfcDevice* device = _devices[path.m_host];

    if ( NULL == device )
    {
        error(KIO::ERR_DOES_NOT_EXIST, "Could not find specified device");
        return;
    }

    KIO::Error err;
    if ( ! device->mkdir(path.m_path, err ) )
    {
        error(err, path.m_path);
        return;
    }

    finished();
}

void AfcProtocol::setModificationTime( const KUrl& url, const QDateTime& mtime )
{
    kDebug(KIO_AFC) << url << " time: " << mtime;

    // check (correct) URL
    const AfcPath path = checkURL(url);

    AfcDevice* device = _devices[path.m_host];

    if ( NULL == device )
    {
        error(KIO::ERR_DOES_NOT_EXIST, "Could not find specified device");
        return;
    }

    KIO::Error err;
    if ( ! device->setModificationTime(path.m_path, mtime, err ) )
    {
        error(err, path.m_path);
        return;
    }

    finished();
}

void AfcProtocol::del( const KUrl& url, bool isfile)
{
    kDebug(KIO_AFC) << url;

    // check (correct) URL
    const AfcPath path = checkURL(url);

    AfcDevice* device = _devices[path.m_host];

    if ( NULL == device )
    {
        error(KIO::ERR_DOES_NOT_EXIST, "Could not find specified device");
        return;
    }

    KIO::Error err;
    if ( ! device->del(path.m_path, err ) )
    {
        error(err, path.m_path);
        return;
    }

    finished();
}

void AfcProtocol::open( const KUrl &url, QIODevice::OpenMode mode )
{
    kDebug(KIO_AFC) << url;

    // check (correct) URL
    const AfcPath path = checkURL(url);

    _opened_device = _devices[path.m_host];

    if ( NULL == _opened_device )
    {
        error(KIO::ERR_DOES_NOT_EXIST, "Could not find specified device");
        return;
    }

    KIO::Error err;
    if ( ! _opened_device->open(path.m_path, mode, err ) )
    {
        error(err, path.m_path);
        return;
    }
    else
    {
        emit opened();
    }
}

void AfcProtocol::read( KIO::filesize_t size )
{
    Q_ASSERT(_opened_device != NULL);

    KIO::Error err;
    if ( !_opened_device->read(size,err) )
    {
        error (err, "Error while reading");
        return;
    }
}

void AfcProtocol::write( const QByteArray &data )
{
    Q_ASSERT(_opened_device != NULL);

    KIO::Error err;
    if ( !_opened_device->write(data,err) )
    {
        error (err, "Error while writing");
        return;
    }
}

void AfcProtocol::seek( KIO::filesize_t offset )
{
    Q_ASSERT(_opened_device != NULL);

    KIO::Error err;
    if ( !_opened_device->seek(offset,err) )
    {
        error (err, "Error while seeking");
        return;
    }
}

void AfcProtocol::close()
{
    Q_ASSERT(_opened_device != NULL);
    _opened_device->close();
    _opened_device = NULL;
    finished();
}


#include "kio_afc.moc"
