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

#include "HelperApp.h"
#include "UserSession.h"

#include "backend/PamHandle.h"

#include <QtCore/QTimer>
#include <QtCore/QFile>
#include <QtCore/QDebug>
#include <QtCore/QSettings>

#include <iostream>
#include <unistd.h>
#include <sys/socket.h>

#include <pwd.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#include "VirtualTerminal.h"

namespace SDDM {
    HelperApp::HelperApp(int& argc, char** argv)
            : QCoreApplication(argc, argv)
            , m_session(new UserSession(this))
    {
        QStringList args = QCoreApplication::arguments();
        int pos;

        int vt = SDDM::VirtualTerminal::setUpNewVt();
        SDDM::VirtualTerminal::jumpToVt(vt);
        
        if ((pos = args.indexOf("--exec")) >= 0) {
            if (pos >= args.length() - 1) {
                exit(-1);
            }
            m_session->setPath(args[pos + 1]);
        } else if ((pos = args.indexOf("--session")) >= 0) {
            if (pos >= args.length() - 1) {
                exit(-1);
            }

            QSettings sessionFile(args[pos + 1], QSettings::IniFormat);
            m_session->setPath(sessionFile.value(QLatin1String("Desktop Entry/Exec")).toString());
        }

        if ((pos = args.indexOf("--user")) >= 0) {
            if (pos >= args.length() - 1) {
                exit(-1);
            }
            m_user = args[pos + 1];
        }
        
        if (m_session->path().isEmpty() || m_user.isEmpty()) 
            qFatal("pass some args please");
        
        PamHandle *pamHandle = new PamHandle; //TODO fix leak

        if (! pamHandle->start("sddm-autologin" /*PAM session*/, m_user)) //Martin check this exists
            qFatal("Could not start PAM");

        QProcessEnvironment env = m_session->processEnvironment();

//         env.insert("DISPLAY", name());
        env.insert("XDG_SEAT", "seat0");
//         env.insert("XDG_SEAT_PATH", daemonApp->displayManager()->seatPath(seat()->name()));
//         env.insert("XDG_SESSION_PATH", daemonApp->displayManager()->sessionPath(QString("Session%1").arg(daemonApp->newSessionId())));
        env.insert("XDG_VTNR", QString::number(vt));
        env.insert("DESKTOP_SESSION", "KDE");
        env.insert("XDG_CURRENT_DESKTOP", "KDE");
        env.insert("XDG_SESSION_CLASS", "user");
        env.insert("XDG_SESSION_TYPE", "wayland");
//         env.insert("XDG_SESSION_DESKTOP", xdgSessionName);

        struct passwd *pw;
        pw = getpwnam(qPrintable(m_user));
        if (pw) {
            env.insert("HOME", pw->pw_dir);
            env.insert("PWD", pw->pw_dir);
            env.insert("SHELL", pw->pw_shell);
            env.insert("USER", pw->pw_name);
            env.insert("LOGNAME", pw->pw_name);
        }

        pamHandle->putEnv(env);

        pamHandle->setItem(PAM_TTY, qPrintable(QString("/dev/tty%1").arg(vt)));

        if (!pamHandle->authenticate())
            qFatal("Could not auth");

        if (!pamHandle->acctMgmt())
            qFatal("Could not do account management");

        if (!pamHandle->setCred(PAM_ESTABLISH_CRED))
            qFatal("Could not establish credentials");
 
        if (!pamHandle->openSession())
            qFatal("Could not open pam session");
        
        qDebug() << "startng process";

        connect(m_session, SIGNAL(finished(int)), SLOT(sessionFinished(int)));

        connect(m_session, &QProcess::readyReadStandardError, m_session, [this](){qDebug() << m_session->readAllStandardError();});
        connect(m_session, &QProcess::readyReadStandardOutput, m_session, [this](){qDebug() << m_session->readAllStandardOutput();});

        env.insert(pamHandle->getEnv());
        m_session->setProcessEnvironment(env);
        m_session->start();
    }

    void HelperApp::sessionFinished(int status) {
        //leave open so that simplelogin stays open and it's easier to debug
        //long term; probably restart kwin at least once
        exit(status);
    }

    const QString& HelperApp::user() const
    {
        return m_user;
    }

    HelperApp::~HelperApp() {

    }
}

int main(int argc, char** argv) {
    SDDM::HelperApp app(argc, argv);
    return app.exec();
}
