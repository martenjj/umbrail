// -*-mode:c++ -*-

#ifndef APPLICATIONDATAINTERFACE_H
#define APPLICATIONDATAINTERFACE_H

class QObject;
class ApplicationData;
class FilesController;
class MapController;
class MainWindow;
 

/**
 * @short Provide access to the main window and its associated controllers.
 *
 * It may be necessary for an arbitrary subclass (e.g. a dialogue) to access
 * the MainWindow, or the FilesController or MapController associated with it.
 * In order to avoid having to pass a pointer to those all the way down to
 * anything that may need it (sometimes through a long call chain), this
 * interface uses the Qt object hierarchy.  When constructed, it searches
 * upwards from the specified parent widget until it finds an object which
 * inherits ApplicationData (which will be the MainWindow) at the top of the
 * tree, and saves it.  The application data can then be accessed via the
 * functions provided.
 *
 * @note The parent @p pnt parameter must be non-NULL and a valid @c QObject
 * that is a descendent of an object that inherits @c ApplicationData, otherwise
 * the application will exit with a fatal error.
 *
 * @see ApplicationData
 * @author Jonathan Marten
 **/

class ApplicationDataInterface
{
protected:
    ApplicationDataInterface(QObject *pnt);

    ApplicationData *applicationData() const		{ return (mApplicationData); }

    FilesController *filesController() const;
    MapController *mapController() const;
    MainWindow *mainWindow() const;
    bool isReadOnly() const;

private:
    ApplicationData *mApplicationData;
};

#endif							// APPLICATIONDATAINTERFACE_H
