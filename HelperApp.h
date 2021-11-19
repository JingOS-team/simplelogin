/*
 * Main authentication application class
 * Copyright (C) 2013 Martin B????za <mbriza@redhat.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 */

#ifndef Auth_H
#define Auth_H

#include <QtCore/QCoreApplication>
#include <QtCore/QProcessEnvironment>

namespace SDDM {
    class UserSession;
    class HelperApp : public QCoreApplication
    {
        Q_OBJECT
    public:
        HelperApp(int& argc, char** argv);
        virtual ~HelperApp();

        const QString &user() const;

    private Q_SLOTS:
        void sessionFinished(int status);

    private:
        qint64 m_id { -1 };
        UserSession *m_session { nullptr };
        QString m_user { };
    };
}

#endif // Auth_H
