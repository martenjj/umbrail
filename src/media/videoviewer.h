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

#ifndef VIDEOVIEWER_H
#define VIDEOVIEWER_H

#include <qwidget.h>

class QLabel;
class QPushButton;
class QTimer;
class QUrl;

namespace Phonon { class VideoPlayer; }


class VideoViewer : public QWidget
{
    Q_OBJECT

public:
    VideoViewer(const QUrl &url, QWidget *pnt = nullptr);
    virtual ~VideoViewer();

protected slots:
    void slotFinished();
    void slotPlay();
    void slotPause();
    void slotStop();
    void slotRewind();

private slots:
    void slotTickTimer();

private:
    Phonon::VideoPlayer *mPlayer;

    QPushButton *mPlayButton;
    QPushButton *mPauseButton;
    QPushButton *mStopButton;
    QPushButton *mRewindButton;
    QLabel *mTimeLabel;

    QTimer *mTickTimer;
};

#endif							// VIDEOVIEWER_H
