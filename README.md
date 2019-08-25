# EMU-nexus

EMU-nexus is a front-end designed to launch emulators and to organize ROM collections, but it can be used to launch any program. It is developed in C++ and it uses GTK3 as graphical user interface. It does not try to compete with any other front-end. It is not designed for an HTPC, it is intended to be used in an everyday not specific purpose PC.


The information about consoles, systems and games comes from TheGamesDB. To avoid stress on TheGamesDB servers, a concise version of the database is processed in the cloud. This concise version is updated regularly and downloaded automatically by EMU-nexus.


The concise version of the database is stored and indexed using Elasticsearch so it can be easily searchable. When EMU-nexus is launched, it starts the Elasticsearch process in the background. By default, Elasticsearch listens in the port 9301. The Apache 2.0 licensed version of Elasticseach is used.


EMU-nexus stores all its data in $HOME/.config/emu-nexus/. Inside this directory, you will find the next items:

     * cache/    Uncompressed ROMs are stored here. By default has a max limit of 9GB. When this limit is reached, the oldest files are deleted.
     * elasticseach/    Contains the Elasticsearch binary and TheGamesDB concise database.
     * media/    Contains the images of your ROM collection.
     * emu-nexus.sqlite    SQLite database containing the data of your collection.



EMU-nexus is distributed under the GPLv3 license.



EMU-nexus source code is available in its Github repository. These are the libraries on which EMU-nexus was built:

     * Jansson    Library for encoding, decoding and manipulating JSON data. It used to process the Elasticseach responses.
     * SQLite    SQL database engine. Used for managing your collection data.
     * cURL    Free and easy-to-use client-side URL transfer library. Used for making HTTP requests to Elasticsearch, download images and the concise version of TheGamesDB database (A libcurl version with SSL support is required).
     * pthread (POSIX thread)    Set of interfaces (functions, header files) for threaded programming. EMU-nexus uses multiple threads to smooth its performance.
     * libxml2    XML C parser and toolkit developed for the Gnome project. Used to process MAME response data when processing ROMs that use MAME file names.
     * Dlib    Modern C++ toolkit containing machine learning algorithms and tools for creating complex software in C++ to solve real world problems. EMU-nexus only uses a small set of Dlib to create thumbnails of the platforms and games artwork.
     * Poppler    PDF rendering library based on the xpdf-3.0 code base. Used for creating the thumbnails of game documents.
     * libarchive    Multi-format archive and compression library. Used to uncompress ROM archives on the go.
     * gtk+ 3    Multi-platform toolkit for creating graphical user interfaces.


To insttall it:

    1.- Download the source code or clone the repository.
    2.- cd <EMU-nexus source code directory>
    3.- make
    4.- sudo make install

