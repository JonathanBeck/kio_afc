/***************************************************************************
 *   Copyright (C) 2010 by Jonathan Beck <jonabeck@gmail.com>              *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/
#ifndef AFCDEVICE_H
#define AFCDEVICE_H

#include <libimobiledevice/libimobiledevice.h>
#include <libimobiledevice/lockdown.h>
#include <libimobiledevice/afc.h>

#include <QtCore/QString>

#include <kio/global.h>
#include <kio/udsentry.h>
#include <kio/job.h>

class AfcProtocol;

class AfcDevice
{
public:
    AfcDevice( const char* id, AfcProtocol* proto );
    virtual ~AfcDevice();

    bool isValid();

    bool createRootUDSEntry( KIO::UDSEntry & entry );
    bool createUDSEntry( const QString & filename, const QString & path, KIO::UDSEntry & entry, KIO::Error& error );

    bool checkError( afc_error_t err, KIO::Error& error );

    bool get(const QString& path, KIO::Error& error);
    bool put( const QString& path, KIO::JobFlags _flags, KIO::Error& error );

    bool stat( const QString& filename, const QString& path, KIO::Error& error );
    bool open( const QString& path, QIODevice::OpenMode mode, KIO::Error& error );
    bool read( KIO::filesize_t size, KIO::Error& error );
    bool write( const QByteArray &data, KIO::Error& error );
    bool seek( KIO::filesize_t offset, KIO::Error& error );
    bool close();

    bool listDir(const QString& path, KIO::Error& error );

    bool mkdir( const QString& path, KIO::Error& error );
    bool setModificationTime( const QString& path, const QDateTime& mtime, KIO::Error& error );
    bool del( const QString& path, KIO::Error& error);

    bool rename( const QString& src, const QString& dest, KIO::JobFlags flags, KIO::Error& error );


private:
    AfcProtocol* _proto;
    idevice_t _dev;
    afc_client_t _afc;

    QString _id;
    QString _name;
    QString _icon;

    uint64_t openFd;
    QString openPath;
};

#endif // AFCDEVICE_H
