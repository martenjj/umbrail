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

#ifndef APPLICATIONDATA_H
#define APPLICATIONDATA_H

#include <qurl.h>

class QWidget;

class FilesController;
class FilesView;
class MapController;


/**
 * @short Maintains global data for the main application.
 *
 * The data directly relevant to the user is the document file name,
 * its modification state and read-only state.
 *
 * Internal data also maintained here is the MainWindow, FilesController
 * and MapController associated with the document.  They are only kept
 * here as pointers - no access is made to them - so that this class and
 * its associated ApplicationDataInterface can be in 'core' and therefore
 * can be used by classes in 'gui' as well as 'app'.
 *
 * @author Jonathan Marten
 **/

class ApplicationData
{

public:
    /**
     * Constructor.
     *
     * The data is set to the initial state as noted for @c clear().
     *
     * @see clear()
     **/
    explicit ApplicationData();

    /**
     * Check whether the document is currently modified.
     *
     * @return @c true if the document is modified
     *
     * @see setModified()
     **/
    bool isModified() const			{ return (mModified); }

    /**
     * Set the document's modification status.
     *
     * @param mod The new setting
     *
     * @see isModified()
     **/
    void setModified(bool mod = true);

    /**
     * Get a short document name suitable for displaying as a window title
     * or in messages.
     *
     * @param onlyIfValid If set to @true and the data is not valid
     * (i.e. no file name is set), return a null string.  Otherwise, if
     * the data is not valid then return the I18N'ed string "Untitled".
     *
     * @return The short name, or a null string if the data is not valid
     * and the @p onlyIfValid paramater is set to @c true.
     **/
    QString documentName(bool onlyIfValid = false) const;

    /**
     * Get the document's file name.
     *
     * @return The file name, or @c QUrl() if none has been set.
     *
     * @see hasFileName()
     * @see setFileName()
     **/
    const QUrl &fileName() const		{ return (mSaveFile); }

    /**
     * Check whether the document has a file name set (i.e. is valid).
     *
     * @return @c true if a file name has been set.
     *
     * @see fileName()
     * @see setFileName()
     **/
    bool hasFileName() const			{ return (mSaveFile.isValid()); }

    /**
     * Set the document's file name.
     *
     * @param file The new file name
     *
     * @see fileName()
     * @see hasFileName()
     **/
    void setFileName(const QUrl &file);

    /**
     * Clear the document data.
     *
     * The file name is set to null (invalid), the modification state
     * is set to not modified, and the read only state to not read only.
     * The FilesController and MapController are not changed or cleared.
     **/
    void clear();

    /**
     * Check whether the document is set to read only.
     *
     * @return @c true if the document is read only
     *
     * @see setReadOnly()
     **/
    bool isReadOnly() const			{ return (mReadOnly); }

    /**
     * Set the document's read only status.
     *
     * @param ro The new setting
     *
     * @see isModified()
     **/
    void setReadOnly(bool ro = true);

    /**
     * Get the FilesController for the document.
     *
     * @return the @c FilesController object
     **/
    FilesController *filesController() const	{ Q_ASSERT(mFilesController!=nullptr); return (mFilesController); }

    /**
     * Get the FilesView for the document.
     *
     * @return the @c FilesView object
     **/
    FilesView *filesView() const		{ Q_ASSERT(mFilesView!=nullptr); return (mFilesView); }

    /**
     * Get the MapController for the document.
     *
     * @return the @c MapController object
     **/
    MapController *mapController() const	{ Q_ASSERT(mMapController!=nullptr); return (mMapController); }

    /**
     * Get the main window for the document, as a @c QWidget.
     *
     * @return the @c main window object
     **/
    QWidget *mainWidget() const			{ Q_ASSERT(mMainWidget!=nullptr); return (mMainWidget); }

protected:
    /**
     * The FilesController object.
     *
     * Needs to be set by the MainWindow which inherits ApplicationData.
     **/
    FilesController *mFilesController;

    /**
     * The FilesView object.
     *
     * Needs to be set by the MainWindow which inherits ApplicationData.
     **/
    FilesView *mFilesView;

    /**
     * The MapController object.
     *
     * Needs to be set by the MainWindow which inherits ApplicationData.
     **/
    MapController *mMapController;

    /**
     * The QWidget corresponding to the MainWindow object.
     *
     * Needs to be set to 'this' by the MainWindow which inherits ApplicationData.
     *
     * @note This returns the main window as a @c QWidget.  It can therefore be
     * used by code outside of 'app', for example as a widget parent or being
     * cast to a @c KXmlGuiWindow.  It can only, however, be cast to a @c MainWindow
     * within 'app'.
     **/
    QWidget *mMainWidget;

private:
    bool mModified;
    bool mReadOnly;
    QUrl mSaveFile;
};

#endif							// APPLICATIONDATA_H
