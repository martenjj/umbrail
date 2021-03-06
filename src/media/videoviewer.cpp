//////////////////////////////////////////////////////////////////////////
//									//
//  Project:	Umbrail - GPX track viewer and editor			//
//									//
//////////////////////////////////////////////////////////////////////////
//									//
//  Copyright (c) 2014-2021 Jonathan Marten <jjm@keelhaul.me.uk>	//
//  Home and download page: <http://github.com/martenjj/umbrail>	//
//									//
//  This program is free software; you can redistribute it and/or	//
//  modify it under the terms of the GNU General Public License as	//
//  published by the Free Software Foundation, either version 3 of	//
//  the License or (at your option) any later version.			//
//									//
//  It is distributed in the hope that it will be useful, but		//
//  WITHOUT ANY WARRANTY;  without even the implied warranty of		//
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the	//
//  GNU General Public License for more details.			//
//									//
//  You should have received a copy of the GNU General Public License	//
//  along with this program;  see the file COPYING for further		//
//  details.  If not, see <http://gnu.org/licenses/gpl>.      		//
//									//
//////////////////////////////////////////////////////////////////////////

#include "videoviewer.h"

#include <qgridlayout.h>
#include <qapplication.h>
#include <qdesktopwidget.h>
#include <qlabel.h>
#include <qtimer.h>
#include <qdebug.h>
#include <qpushbutton.h>

#include <klocalizedstring.h>
#include <kstandardguiitem.h>

#include <kfdialog/dialogstatesaver.h>

#ifdef HAVE_PHONON
#include <phonon/videoplayer.h>
#endif


VideoViewer::VideoViewer(const QUrl &url, QWidget *pnt)
    : QWidget(pnt)
{
    qDebug() << url;

    setObjectName("VideoViewer");
    setWindowTitle(i18n("Video Viewer"));
    setAttribute(Qt::WA_DeleteOnClose);
 
    QGridLayout *gl = new QGridLayout(this);
    mPlayer = new Phonon::VideoPlayer(this);
    QObject::connect(mPlayer, &Phonon::VideoPlayer::finished, this, &VideoViewer::slotFinished);
    gl->addWidget(mPlayer, 0, 0, 1, -1);
    gl->setRowStretch(0, 1);

    mPlayButton = new QPushButton(QIcon::fromTheme("media-playback-start"), i18nc("@action:button", "Play"), this);
    mPlayButton->setEnabled(false);
    connect(mPlayButton, &QAbstractButton::clicked, this, &VideoViewer::slotPlay);
    gl->addWidget(mPlayButton, 1, 0, Qt::AlignLeft);

    mPauseButton = new QPushButton(QIcon::fromTheme("media-playback-pause"), i18nc("@action:button", "Pause"), this);
    connect(mPauseButton, &QAbstractButton::clicked, this, &VideoViewer::slotPause);
    gl->addWidget(mPauseButton, 1, 1, Qt::AlignLeft);

    mRewindButton = new QPushButton(QIcon::fromTheme("media-skip-backward"), i18nc("@action:button", "Rewind"), this);
    connect(mRewindButton, &QAbstractButton::clicked, this, &VideoViewer::slotRewind);
    gl->addWidget(mRewindButton, 1, 3, Qt::AlignLeft);

    mStopButton = new QPushButton(QIcon::fromTheme("media-playback-stop"), i18nc("@action:button", "Stop"), this);
    connect(mStopButton, &QAbstractButton::clicked, this, &VideoViewer::slotStop);
    gl->addWidget(mStopButton, 1, 5, Qt::AlignRight);

    QPushButton *but = new QPushButton(this);
    KGuiItem::assign(but, KStandardGuiItem::close());
    connect(but, &QAbstractButton::clicked, this, &QWidget::close);
    gl->addWidget(but, 1, 6, Qt::AlignRight);

    mTimeLabel = new QLabel(this);
    gl->addWidget(mTimeLabel, 1, 4, Qt::AlignHCenter);
    gl->setColumnStretch(4, 1);

    mTickTimer = new QTimer(this);
    mTickTimer->setSingleShot(false);
    mTickTimer->setInterval(100);
    connect(mTickTimer, &QTimer::timeout, this, &VideoViewer::slotTickTimer);

    setMinimumSize(400, 300);
    DialogStateSaver::restoreWindowState(this);

    mPlayer->play(url);
    mTickTimer->start();
}


VideoViewer::~VideoViewer()
{
    DialogStateSaver::saveWindowState(this);

    mPlayer->deleteLater();
    qDebug() << "done";
}


void VideoViewer::slotFinished()
{
    qDebug();
    mPlayButton->setEnabled(true);
    mPauseButton->setEnabled(false);
    mStopButton->setEnabled(false);
    mRewindButton->setEnabled(false);
    mTickTimer->stop();
}


void VideoViewer::slotPlay()
{
    mPlayButton->setEnabled(false);
    mPauseButton->setEnabled(true);
    mRewindButton->setEnabled(true);
    mStopButton->setEnabled(true);
    mPlayer->play();
    mTickTimer->start();
    slotTickTimer();
}


void VideoViewer::slotPause()
{
    if (mPlayer->isPaused())
    {
        mPlayButton->setEnabled(false);
        mPlayer->play();
        mTickTimer->start();
    }
    else
    {
        mPlayButton->setEnabled(true);
        mPlayer->pause();
        mTickTimer->stop();
    }
    slotTickTimer();
}


void VideoViewer::slotStop()
{
    mPlayButton->setEnabled(true);
    mPauseButton->setEnabled(false);
    mRewindButton->setEnabled(false);
    mStopButton->setEnabled(false);
    mPlayer->stop();
    mTickTimer->stop();
    slotTickTimer();
}


void VideoViewer::slotRewind()
{
    mPlayer->seek(0);
    slotTickTimer();
}


static QString formatTime(qint64 msecs)
{
    if (msecs<0) return ("--:--");

    msecs = (msecs+500)/1000;
    int secs = msecs % 60;
    int mins = msecs / 60;
    return (QString("%1:%2").arg(mins).arg(secs, 2, 10, QLatin1Char('0')));

}


void VideoViewer::slotTickTimer()
{
    mTimeLabel->setText(formatTime(mPlayer->currentTime())+" / "+formatTime(mPlayer->totalTime()));
}
