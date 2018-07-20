
#include "exporterbase.h"

#include <string.h>
#include <errno.h>

#include <qdebug.h>
#include <qsavefile.h>
#include <qbuffer.h>
#include <qguiapplication.h>
#include <qclipboard.h>
#include <qmimedata.h>

#include <klocalizedstring.h>

#include "trackdata.h"
#include "errorreporter.h"



ExporterBase::ExporterBase()
    : ImporterExporterBase()
{
    qDebug();
    mSelectionId = 0;					// none set yet, export all
}


void ExporterBase::setSelectionId(unsigned long id)
{
    qDebug() << "to" << id;
    mSelectionId = id;					// export just selected items
}


bool ExporterBase::isSelected(const TrackDataItem *item) const
{
    return (mSelectionId==0 || item->selectionId()==mSelectionId);
}


bool ExporterBase::save(const QUrl &file, const TrackDataFile *item)
{
    qDebug() << "to" << file;
    reporter()->setFile(file);

    if (file.scheme()=="clipboard")			// output to the clipboard
    {
        QBuffer buf;
        if (!buf.open(QIODevice::WriteOnly))
        {
            reporter()->setError(ErrorReporter::Fatal, i18n("Cannot write to clipboard buffer"));
            return (false);
        }

        if (!saveTo(&buf, item)) return (false);

        buf.close();					// finished writing to buffer
        const QByteArray data = buf.data();
        qDebug() << "clipboard data size" << data.size();
        //qDebug() << "clipboard data" << data;

        // Prepare clipboard data with appropriate MIME type
        QMimeData *mime = new QMimeData;
        mime->setData("application/x-gpx+xml", data);

        // Set data on clipboard
        QClipboard *clip = QGuiApplication::clipboard();
        clip->setMimeData(mime);
    }
    else						// output to a real file
    {
        // verify/open save file
        if (!file.isLocalFile())
        {
            reporter()->setError(ErrorReporter::Fatal, i18n("Can only save to local files"));
            return (false);
        }

        QSaveFile mSaveFile;

        mSaveFile.setFileName(file.path());
        if (!mSaveFile.open(QIODevice::WriteOnly))
        {
            reporter()->setError(ErrorReporter::Fatal, i18n("Cannot open file, %1", strerror(errno)));
            return (false);
        }

        if (!saveTo(&mSaveFile, item))
        {
            reporter()->setError(ErrorReporter::Fatal, mSaveFile.errorString());
            mSaveFile.cancelWriting();
            return (false);
        }

        mSaveFile.commit();
    }

    return (true);					// export was successful
}
