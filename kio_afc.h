/***************************************************************************
 *   Copyright (C) 2010 by Jonathan Beck <jonabeck@gmail.com>              *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#ifndef __kio_afc_h__
#define __kio_afc_h__

#include <kio/global.h>
#include <kio/slavebase.h>

#include <QtCore/QObject>
#include <QtCore/QHash>
#include <QtCore/QString>

#include "afcdevice.h"
#include "afcpath.h"

#include <libimobiledevice/libimobiledevice.h>

class AfcProtocol : public QObject, public KIO::SlaveBase
{
  Q_OBJECT
public:
  AfcProtocol( const QByteArray &pool, const QByteArray &app);
  virtual ~AfcProtocol();

  void ProcessEvent(const idevice_event_t *event);

  AfcPath checkURL( const KUrl& url );

  virtual void stat( const KUrl& url );

  virtual void get( const KUrl& url );

  virtual void put( const KUrl& url, int _mode,
                    KIO::JobFlags _flags );
  virtual void copy( const KUrl &src, const KUrl &dest,
                     int mode, KIO::JobFlags flags );
  virtual void rename( const KUrl &src, const KUrl &dest,
                       KIO::JobFlags flags );
  virtual void symlink( const QString &target, const KUrl &dest,
                        KIO::JobFlags flags );

  virtual void listDir( const KUrl& url );
  virtual void mkdir( const KUrl& url, int permissions );
  virtual void setModificationTime( const KUrl& url, const QDateTime& mtime );
  virtual void del( const KUrl& url, bool isfile);
  virtual void open( const KUrl &url, QIODevice::OpenMode mode );
  virtual void read( KIO::filesize_t size );
  virtual void write( const QByteArray &data );
  virtual void seek( KIO::filesize_t offset );
  virtual void close();

  //cached user and group
  static QString m_user;
  static QString m_group;

private:

  QHash<QString, AfcDevice*> _devices;
  AfcDevice* _opened_device;

};

#endif

