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

class AfcProtocol;

class AfcDevice
{
public:
    AfcDevice( const char* id, AfcProtocol* proto );
    virtual ~AfcDevice();

    bool isValid();


    KIO::UDSEntry getRootUDSEntry();

    QByteArray get(const QString& path);
    bool stat( const QString& path, KIO::UDSEntry& entry );
    bool open( const QString& path, QIODevice::OpenMode mode );
    void read( KIO::filesize_t size );
    void write( const QByteArray &data );
    void seek( KIO::filesize_t offset );
    void close();

    KIO::UDSEntryList listDir(const QString& path);


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
