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

#include "afcdevice.h"
#include "kio_afc.h"
#include <kdebug.h>

#include <QtCore/QVarLengthArray>


#define AFC_PROTO "com.apple.afc"
#define KIO_AFC 7002

using namespace KIO;

AfcDevice::AfcDevice( const char* id, AfcProtocol* proto ) :_proto(proto), openFd(-1)
{
    _id = id;
    _dev = NULL;
    _afc = NULL;

    idevice_new (&_dev, id);

    lockdownd_client_t lockdown_cli = NULL;
    if ( LOCKDOWN_E_SUCCESS == lockdownd_client_new_with_handshake (_dev, &lockdown_cli, "kio_afc") )
    {
        //Afc service
        uint16_t port;
        if ( LOCKDOWN_E_SUCCESS == lockdownd_start_service (lockdown_cli, AFC_PROTO, &port) )
        {
            afc_client_new (_dev, port, &_afc);
        }

        //device name
        char* name = NULL;
        lockdownd_get_device_name (lockdown_cli, &name);
        _name = name;
        free (name);

        //device model
        if ( NULL != _afc )
        {
            char* model = NULL;
            afc_get_device_info_key (_afc, "Model", &model );

            if ( NULL != model )
            {
                if ( strstr(model, "iPod") != NULL)
                {
                    _icon = "multimedia-player-apple-ipod-touch";
                }
                else if ( strstr( model, "iPad" ) )
                {
                    _icon = "computer-apple-ipad";
                }
                else
                {
                    _icon = "phone-apple-iphone";
                }
            }
            free (model);
        }
    }
    lockdownd_client_free (lockdown_cli);
}

AfcDevice::~AfcDevice()
{
    idevice_free(_dev);
    afc_client_free(_afc);
    _dev = NULL;
    _afc = NULL;
}

bool AfcDevice::isValid()
{
    if ( NULL != _dev && NULL != _afc )
        return true;
    return false;
}

UDSEntry AfcDevice::getRootUDSEntry()
{
    UDSEntry udsentry;
    udsentry.insert( UDSEntry::UDS_NAME, _id );
    udsentry.insert( UDSEntry::UDS_DISPLAY_NAME, _name );
    udsentry.insert( UDSEntry::UDS_ICON_NAME, _icon );

    return udsentry;
}

QByteArray AfcDevice::get(const QString& path)
{
    QByteArray data;
    //    int bytes = 0;
    //    char *buf = NULL;
    //    afc_error_t err = afc_file_read(afc, openFd, buf, size, &bytes);
    //
    //
    //    QByteArray  filedata;
    //
    //    _afc_client = fuse_get_context()->private_data;
    //
    //    if (size == 0)
    //        return 0;
    //
    //    afc_error_t err = afc_file_seek(_afc_client, fi->fh, offset, SEEK_SET);
    //    if (err != AFC_E_SUCCESS) {
    //    int res = get_afc_error_as_errno(err);
    //    return -res;
    //    }
    //
    //    err = afc_file_read(afc, fi->fh, buf, size, &bytes);
    //    if (err != AFC_E_SUCCESS) {
    //        int res = get_afc_error_as_errno(err);
    //        return -res;
    //    }
    //
    //    return bytes;
    return data;
}

bool AfcDevice::stat( const QString& path, UDSEntry& entry )
{
    bool rc = false;
    char **info = NULL;

    afc_error_t ret = afc_get_file_info(_afc, (const char*) path.toLocal8Bit(), &info);

    if ( AFC_E_SUCCESS == ret && NULL != info )
    {
        rc = true;
        // get file attributes from info list
        for (int i = 0; info[i]; i += 2)
        {
            if (!strcmp(info[i], "st_size"))
            {
                entry.insert( UDSEntry::UDS_SIZE, atoll(info[i+1]) );
            }
            else if (!strcmp(info[i], "st_blocks"))
            {
//                stbuf->st_blocks = atoi(info[i+1]);
            }
            else if (!strcmp(info[i], "st_ifmt"))
            {
                if (!strcmp(info[i+1], "S_IFREG"))
                {
                    entry.insert( UDSEntry::UDS_FILE_TYPE, S_IFREG );
                    entry.insert( UDSEntry::UDS_ACCESS, 0644 );
                }
                else if (!strcmp(info[i+1], "S_IFDIR"))
                {
                    entry.insert( UDSEntry::UDS_FILE_TYPE, S_IFDIR );
                    entry.insert( UDSEntry::UDS_ACCESS, 0755 );
                }
                else if (!strcmp(info[i+1], "S_IFLNK"))
                {
                    entry.insert( UDSEntry::UDS_FILE_TYPE, S_IFLNK );
                    entry.insert( UDSEntry::UDS_ACCESS, 0777 );
                }
                else if (!strcmp(info[i+1], "S_IFBLK"))
                {
                    entry.insert( UDSEntry::UDS_FILE_TYPE, S_IFBLK );
                    entry.insert( UDSEntry::UDS_ACCESS, 0644 );
                }
                else if (!strcmp(info[i+1], "S_IFCHR"))
                {
                    entry.insert( UDSEntry::UDS_FILE_TYPE, S_IFCHR );
                    entry.insert( UDSEntry::UDS_ACCESS, 0644 );
                }
                else if (!strcmp(info[i+1], "S_IFIFO"))
                {
                    entry.insert( UDSEntry::UDS_FILE_TYPE, S_IFIFO );
                    entry.insert( UDSEntry::UDS_ACCESS, 0644 );
                }
                else if (!strcmp(info[i+1], "S_IFSOCK"))
                {
                    entry.insert( UDSEntry::UDS_FILE_TYPE, S_IFSOCK );
                    entry.insert( UDSEntry::UDS_ACCESS, 0644 );
                }
            }
            else if (!strcmp(info[i], "st_nlink"))
            {
                entry.insert( UDSEntry::UDS_NUMBER, atoi(info[i+1]));
            }
            else if (!strcmp(info[i], "st_mtime"))
            {

                entry.insert( UDSEntry::UDS_TIME, atoll(info[i+1]) / 1000000000 );
            }
            free (info[i]);
        }
        free(info);
    }

    entry.insert( UDSEntry::UDS_USER, AfcProtocol::m_user );
    entry.insert( UDSEntry::UDS_GROUP, AfcProtocol::m_group );

    return rc;
}

UDSEntryList AfcDevice::listDir(const QString& path)
{
    UDSEntryList entryList;

    char **list = NULL;
    if ( AFC_E_SUCCESS == afc_read_directory (_afc, (const char*) path.toLocal8Bit(), &list) )
    {        
        char** ptr = list;
        while ( NULL != *ptr )
        {
            if ( 0 != QString::compare(*ptr, ".") && 0 != QString::compare(*ptr, "..") )
            {
                UDSEntry entry;
                QString subPath = path.compare("/") ? path : "";
                subPath += "/";
                subPath += *ptr;

                entry.insert( UDSEntry::UDS_NAME, *ptr);

                if ( stat (subPath, entry) )
                    entryList.append(entry);
            }
            free(*ptr);
            ptr++;
        }
        free (list);
    }
    return entryList;
}

bool AfcDevice::open( const QString& path, QIODevice::OpenMode mode )
{
    bool ret = false;
    afc_file_mode_t file_mode;

    if ( QIODevice::ReadOnly == mode )
    {
        file_mode = AFC_FOPEN_RDONLY;
    }
    else if ( QIODevice::ReadWrite == mode )
    {
        file_mode = AFC_FOPEN_RW;
    }
    else if ( QIODevice::WriteOnly == mode )
    {
        file_mode = AFC_FOPEN_WRONLY;
    }
    else if ( ( QIODevice::ReadWrite | QIODevice::Truncate ) == mode )
    {
        file_mode = AFC_FOPEN_WR;
    }
    else if ( QIODevice::Append == mode )
    {
        file_mode = AFC_FOPEN_APPEND;
    }
    else if ( QIODevice::Truncate == mode )
    {
        file_mode = AFC_FOPEN_RDAPPEND;
    }
    else
    {
        _proto->error(KIO::ERR_COULD_NOT_ACCEPT, "Unsupported mode");
    }

    afc_error_t er = afc_file_open(_afc, (const char*) path.toLocal8Bit(), file_mode, &openFd);

    if ( AFC_E_SUCCESS == er )
    {
        ret = true;
        openPath = path;
        UDSEntry entry;
        stat(path, entry);

        _proto->totalSize( entry.numberValue(UDSEntry::UDS_SIZE, 0) );
        _proto->position( 0 );

    }
    return ret;
}

void AfcDevice::read( KIO::filesize_t size )
{
    Q_ASSERT(openFd != -1);

    QVarLengthArray<char> buffer(size);
    while (true) {
        uint32_t bytes_read = 0;
        afc_error_t  err;
        do {
            err = afc_file_read(_afc, openFd, buffer.data(), size, &bytes_read);
        } while (bytes_read < size && err == AFC_E_SUCCESS);

        if (bytes_read > 0) {
            QByteArray array = QByteArray::fromRawData(buffer.data(), bytes_read);
            _proto->data( array );
            size -= bytes_read;
        } else {
            // empty array designates eof
            _proto->data(QByteArray());
            if (bytes_read != 0) {
                _proto->error(KIO::ERR_COULD_NOT_READ, openPath);
                _proto->close();
            }
            break;
        }
        if (size <= 0) break;
    }
}

void AfcDevice::write( const QByteArray &data )
{
    Q_ASSERT( openFd != -1 );

    uint32_t bytes_written = 0;
    afc_error_t er = afc_file_write (_afc, openFd, data.constData(), data.size(), &bytes_written);

    if ( AFC_E_SUCCESS == er)
    {
        _proto->written(data.size());
    }
    else
    {
        if ( bytes_written < data.size() )
        {
            _proto->error(KIO::ERR_DISK_FULL, openPath);
            _proto->close();
        }
        else
        {
            _proto->error(KIO::ERR_COULD_NOT_WRITE, openPath);
            _proto->close();
        }
    }
}

void AfcDevice::seek( KIO::filesize_t offset )
{
    Q_ASSERT(openFd != -1);

    afc_error_t er = afc_file_seek (_afc, openFd, offset, SEEK_SET);

    if ( AFC_E_SUCCESS == er )
    {
        _proto->position( offset );
    }
    else
    {
        _proto->error(KIO::ERR_COULD_NOT_SEEK, openPath);
        _proto->close();
    }
}

void AfcDevice::close()
{
    Q_ASSERT(openFd != -1);

     afc_file_close (_afc, openFd);
    openFd = -1;
    openPath = "";

    _proto->finished();
}
