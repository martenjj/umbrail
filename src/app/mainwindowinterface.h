// -*-mode:c++ -*-

#ifndef MAINWINDOWINTERFACE_H
#define MAINWINDOWINTERFACE_H

class QObject;
class MainWindow;
class FilesController;
class MapController;
 

/**
 * @short Provide access to the main window and its associated controllers.
 *
 * It may be necessary for an arbitrary subclass (e.g. a dialogue) to access
 * the MainWindow, or the FilesController or MapController associated with it.
 * In order to avoid having to pass a pointer to those all the way down to
 * anything that may need it (sometimes through a long call chain), this
 * interface uses the Qt object hierarchy.  When constructed, it searches
 * upwards from the specified parent widget until it finds the MainWindow at
 * the top of the tree, and saves it.  The MainWindow or the controllers can
 * then be accessed via the functions provided.
 *
 * @note The parent @p pnt parameter must be non-NULL and a valid @c QObject
 * that is a descendent of the @c MainWindow, otherwise the application will
 * exit with a fatal error.
 *
 * @see MainWindow
 * @author Jonathan Marten
 **/

class MainWindowInterface
{
protected:
    MainWindowInterface(QObject *pnt);

    MainWindow *mainWindow() const		{ return (mMainWindow); }
    FilesController *filesController() const;
    MapController *mapController() const;
    bool isReadOnly() const;

private:
    MainWindow *mMainWindow;
};
 
#endif							// MAINWINDOWINTERFACE_H
