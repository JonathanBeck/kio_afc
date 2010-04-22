/***************************************************************************
 *   Copyright (C) 2010 by Jonathan Beck <jonabeck@gmail.com>              *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#ifndef AFCPATH_H
#define AFCPATH_H

#include <QtCore/QString>
#include <QtCore/QDebug>

class AfcPath
{
public:
    AfcPath(const QString& host, const QString& path);

    bool isRoot() const;

    QString m_host;
    QString m_path;
};

QDebug &operator<<(QDebug &out, const AfcPath &path);

#endif // AFCPATH_H
