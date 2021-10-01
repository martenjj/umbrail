Umbrail - GPX Track Viewer and Editor
=====================================

Umbrail is a GPX file viewer.  GPX (https://www.topografix.com/gpx.asp)
is the standard file format for the interchange of GPS data (recorded
tracks, waypoints and routes) which is supported by almost all GPS
hardware (including smartphones and dedicated receivers) and
applications.  The application can:

 * View recorded tracks on a map, any Marble theme or from WMS server
 * Basic analysis: location and elevation information, timings, distances
 * Advanced analysis: elevation/speed profile, stop detection
 * Edit the data: move or delete points, split/join segments, annotation
 * Organise waypoints, photos, notes and audio recordings
 * Plan and analyse routes

It uses the KDE Frameworks libraries, including Marble for the map display.

The application is named after the [Umbrail Pass](https://en.wikipedia.org/wiki/Umbrail_Pass)
in eastern Switzerland, although if you really want it to be an
acronym then it could stand for "Useful Map BRowser And Inspector of
Logs".

There are some screen shots in the [doc/screenshots](./doc/screenshots.md) directory.


Building and installing
-----------------------

This package does not require a full KDE desktop installation, but
only the Frameworks libraries (https://develop.kde.org/products/frameworks)
and Marble (https://marble.kde.org).  CMake is required for building.
Your distro should have packages for all of these, either installed as
standard or additionally available.

The libkfdialog library (https://github.com/martenjj/libkfdialog) is
also required.

QCustomPlot (https://www.qcustomplot.com) is used for the
profile plotting.  If this is not available as an optional package in
whatever distro you are using, you can download and install it from
its official site as above.  If this is not available then an internal
copy will be used.

Some additional KDE libraries can be used if they are available:

Phonon4Qt5 - Multimedia playback library, for playback of audio/video notes
https://invent.kde.org/libraries/phonon

KExiv2 - Wrapper around the Exiv2 library, for geolocation of photos
https://invent.kde.org/graphics/libkexiv2

Assuming that you have all of these installed or already provided by
your distro, go to a suitable build location (e.g. your home
directory) and do:

     git clone https://github.com/martenjj/umbrail.git
     cd umbrail
     mkdir build
     cd build
     cmake ..
     make
     sudo make install


Running
-------

Umbrail works as a standard desktop application, so start it from
whatever application launcher you use or by typing its name at the
command line.  If you have set up appropriate file associations for
GPX files (exactly how to do that will depend on the desktop system
that you are using, in KDE Plasma go to "System Settings -
Personalisation - Applications - File Associations" and select the
association for files of type "application/gpx+xml") you can also open
a GPX file from a file manager.


User interface
--------------

The tree structure of the GPX file is shown on the left: the top level
file, then below that tracks/routes, segments and track points.
Waypoints and audio/video notes are placed in folders; these do not
correspond to any structure in the GPX file, but can be used here for
organising waypoints.

Any part of the tree can be selected.  There are some restrictions on
what can be selected: only items of the same type and having the same
parent can be selected together.  The selected item is highlighted on
the map, and the map can be centered to there using "Data - Show on
Map" (useful shortcut: Ctrl+G).

The default map display is OpenStreetMap (https://openstreetmap.org).

The map can be zoomed and panned in the usual way.  A context menu is
available to display address and elevation information, or to create a
waypoint or route point, at the clicked position.

The map theme can be changed to any supported by Marble.  Run the KDE
Marble application to add a new theme, or to connect to a WMS server.


Item Properties
---------------

The "Properties" dialogue shows all of the information that is
available for the selected item or items.  Some information can be
changed, which will be reflected in the display and a subsequently
saved GPX file.


Editing
-------

The operations in the "Edit" menu operate on the selected items.  For
creating new items, only those which can be placed at the appropriate
place in the data tree will be available (for example, a new waypoint
can only be created in a folder).  "Move Mode" enables dragging and
dropping of points (track points, waypoints or route points) on the
map and of moveable items in the data tree; this mode is normally
disabled to avoid inadvertent changes and to allow drag selection, and
will be turned off again if a different item is selected.

The operations in the "Data" menu also mostly operate on the selected
items.  The "Move Waypoint/Segment/Folder" action moves the selected
item to another container (only containers which can accept the item
are allowed to be selected).  "Locate Stops", "Statistics/Quality" and
"Profile" operate on the selected series of points (even across tracks
and segments).


Time Zone
---------

Times recorded in a GPX file are UTC.  Setting a file time zone
displays all times in that time zone instead.

The time zone can be set using the "Properties" of the top level file
item, or by "Set Time Zone" in the "Data" menu.  The time zone can be
changed using the latter operation even if the file is read only,
although the setting will not be saved.  To set the time zone, use
"Change" to select the time zone from a list or use "Get from
Location" to look up the time zone from the file location.  This
requires a configured user name for geonames.org (see "Online
Services" below).

If the option "Check time zone when loading" is set in "Configure -
Files", if a loaded GPX file does not have a saved time zone then you
will be prompted to select one.  Resave the file to store the time
zone permanently in it.


Elevation/Speed Profile
-----------------------

This shows a plot of the elevation and/or speed for the selected set of
points.  Elevation can be selected to be either as recorded by the GPS
receiver (if this information is available), or by an online DEM (see
"Online Services" below).  Speed can be selected to be either as
recorded by the GPS receiver (if this information is available), or
calculated from the incremental track time and distance.

If using the DEM for elevation, then downloading the data the first
time it is needed for a particular location may take some time.  The
plot will be updated when data is available.

An elevation profile is also available for routes, using DEM data
only.


Stop Detection
--------------

This looks for locations in the selected set of points where the
position does not appear to change (very much) over a time span.  Some
movement and position noise is accepted, the detection parameters can
be adjusted in the "Locate Stops" dialogue.

The detected stops are initially previewed on the map.  If two or more
stops are detected close together (possibly there has been a slight
change in position), they can be combined by selecting them in the
scrolling list and doing "Merge Stops".  The stops whose check boxes
are turned on will be added to the GPX file, in a folder named as
specified, when "Commit" is clicked.


Photo Import
------------

Photo and image files can be added to the data, in order to locate
them on the map or annotate a recording.  Select a file via "File -
Import Photo", the photo will be positioned on the map using whatever
data is available.  If it contains embedded EXIF data with a position,
then it will be placed there; otherwise, if it contains a time then it
will be placed at the position of the nearest point with that recorded
time.  The file time zone must be set correctly for this to work.

A photo can be viewed using "Data - View Media" (useful shortcut: Ctrl+P).

Only a link to the photo file is stored in the GPX file, so it needs
to remain in its original place in order to be found.  Currently only
JPEG files (with the extension ".jpg") are recognised.

Options are available in "Configure - Media" to control photo import
and viewing.


Audio/Video Notes
-----------------

If you use the [OsmAnd+](http://osmand.net) app (on Android or iOS) to
take notes during trips, you can import and view them along with the
track recordings.  You will need to download them from your device to
a suitable location, then use "Configure - Files - Audio Notes" to
specify where they are stored.  Loading a GPX file which contains
notes will then display them as such, and you can listen to the audio
or view the video using "Data - View Media" as above.  Photo notes are
also accepted as for "Photo Import" above.

Information on similar facilities provided by any other apps would be
welcome.

Currently only the file extensions ".3gp" for audio notes and ".mp4"
for video notes are recognised.

For notes or waypoints, the "Waypoint Status" options can be used to
mark those needing to be done or those that have been actioned or
discarded.


Online Services
---------------

The displayed map area or the selected items can be opened in a
browser to view OpenStreetMap, Google Maps or Bing Maps.  If you need
to use a specific web browser to view these maps, then the browser to
use can be set in "Configure - Services".  Note that the area
displayed in the browser may not exactly correspond to the displayed
map area, but it should be close enough.

DEM elevation data is provided by [Open Topography](http://opentopography.org),
which is a free service with no registration required.  The data used
is the SRTMGL3 model.

If using the "Get time zone from location" facility, the time zone
corresponding to a position is found from [GeoNames](http://www.geonames.org).
Using this service requires a user name, so register on GeoNames and
then enter your user name in "Configure - Services".  You do not need
to enter a password here.

The "Position Information" address lookup uses
[Nominatim](https://nominatim.openstreetmap.org).  The default Marble
map display also uses OpenStreetMap tiles.

Please see those services' privacy policies for information on how
they may store and use your data.  This may include any user
information that you provide for registration, the queries that you
make, or the URLs of any data that you download.


Resaving Files - Caution
------------------------

If a GPX file is saved, it may not be in the same form as it was
originally loaded. All of the GPX objects and tags should be present,
but they will have been converted back again from their internal
representation which means that there may be a change in textual
ordering or in data formats.  All of the information should in theory
still be there, but this cannot be guaranteed especially if
vendor-specific data is included in the file.

For this reason, a backup is made the first time that a GPX file is
overwritten (if no previous backup exists).


Problems?
---------

Please raise an issue on GitHub (at http://github.com/martenjj/umbrail)
if there are any problems with installing or using this package.  In
particular, if there are any incompatibilities with other GPS
applications or any desktop environments apart from KDE), or if there
are any enhancements you would like to see implemented (or are willing
to implement!).


Thank you for your interest!
----------------------------

Jonathan Marten, http://github.com/martenjj
