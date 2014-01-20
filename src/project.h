//////////////////////////////////////////////////////////////////////////
//									//
//  Project:	Utility library						//
//  Edit:	18-Jan-14						//
//									//
//////////////////////////////////////////////////////////////////////////
//									//
//  Copyright (c) 2012-2014 Jonathan Marten <jjm@keelhaul.me.uk>	//
//  Home and download page:  http://www.keelhaul.me.uk/TBD/		//
//									//
//  This program is free software; you can redistribute it and/or	//
//  modify it under the terms of the GNU General Public License as	//
//  published by the Free Software Foundation; either version 2 of	//
//  the License, or (at your option) any later version.			//
//									//
//  It is distributed in the hope that it will be useful, but		//
//  WITHOUT ANY WARRANTY; without even the implied warranty of		//
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the	//
//  GNU General Public License for more details.			//
//									//
//  You should have received a copy of the GNU General Public		//
//  License along with this program; see the file COPYING for further	//
//  details.  If not, write to the Free Software Foundation, Inc.,	//
//  59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.		//
//									//
//////////////////////////////////////////////////////////////////////////

#ifndef PROJECT_H
#define PROJECT_H


#include <kurl.h>

class KConfig;


/**
 * @short Maintains data for an application's project.
 *
 * A project is a file, a document, or a collection of these.  It is normally
 * displayed in a single application main window.
 *
 * This class as it is simply maintains a file name and a modification state.
 * It may be subclassed to add additional information.
 *
 * @author Jonathan Marten
 **/

class Project
{

public:
    /**
     * Create a new project.
     **/
    Project();

    /**
     * Destructor.
     **/
    ~Project();

    /**
     * Check whether the project is currently modified.
     *
     * @return @c true if the project is modified
     *
     * @note A newly-created project is not modified.  Resetting the
     * project via @c clear() sets the project to not modified.
     *
     * @see setModified()
     **/
    bool isModified() const			{ return (mModified); }

    /**
     * Set the project's modification status.
     *
     * @param mod The new setting
     *
     * @see isModified()
     **/
    void setModified(bool mod = true);

    /**
     * Get a short name suitable for displaying as a window title or
     * in messages.
     *
     * @param onlyIfValid If set to @true and the project is not valid (i.e.
     * does not have a file name set), return a null string.  Otherwise, if
     * the project is not valid then return the I18N'ed string "Untitled".
     *
     * @return The short name, or a null string if the project is not valid
     * and the @p onlyIfValid paramater is set to @c true.
     **/
    QString name(bool onlyIfValid = false) const;

    /**
     * Get the project's file name.
     *
     * @return The file name, or @c Kurl() if none has been set.
     *
     * @see hasFileName()
     * @see setFileName()
     **/
    KUrl fileName() const			{ return (mSaveFile); }

    /**
     * Check whether the project has a file name set (i.e. is valid).
     *
     * @return @c true if the project has a file name.
     *
     * @see fileName()
     * @see setFileName()
     **/
    bool hasFileName() const			{ return (mSaveFile.isValid()); }

    /**
     * Set the project's file name.
     *
     * @param file The new file name
     *
     * @see fileName()
     * @see hasFileName()
     **/
    void setFileName(const KUrl &file);

    /**
     * Save project data to a configuration file.
     *
     * @param conf The configuration to save to.
     * @return A null string if the save was successful, otherwise an error message.
     *
     * @see load()
     **/
    QString save(KConfig *conf);

    /**
     * Load project data from a configuration file.
     *
     * @param conf The configuration to load from.
     * @return A null string if the load was successful, otherwise an error message.
     *
     * @see save()
     **/
    QString load(const KConfig *conf);

    /**
     * Clear the project.
     *
     * The file name is set to null (invalid), and the modification state is set to
     * not modified.
     **/
    void clear();

private:
    bool mModified;
    KUrl mSaveFile;
};

 
#endif							// PROJECT_H
