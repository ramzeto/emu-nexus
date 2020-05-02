# EMU-nexus

EMU-nexus is a front-end designed to launch emulators and to organize ROM collections. It is developed in C++ and it uses GTK3 as graphical user interface. It does not try to compete with any other front-end. It is not designed for an HTPC, it is intended to be used in an everyday not specific purpose PC. It is distributed under the [GPLv3 license](https://www.gnu.org/licenses/gpl-3.0.en.html).

![EMU-nexus](https://www.dropbox.com/s/iocuppdwj9cbdur/emu-nexus_screenshot_2.png?raw=1)

Information and images of the consoles, systems and games comes from [TheGamesDB](https://thegamesdb.net/). To avoid stress on TheGamesDB servers, a concise version of the database is processed in the cloud. This concise version is updated regularly and downloaded automatically.


This database is stored and indexed using [Elasticsearch](https://www.elastic.co/downloads/elasticsearch-oss) so it can be easily searchable. When EMU-nexus is launched, it starts the Elasticsearch process in the background. By default, Elasticsearch listens in the port 9301. The [Apache 2.0](https://www.apache.org/licenses/LICENSE-2.0) licensed version of Elasticseach is used.


EMU-nexus scans specific directories and matches files with entries in the database, if a match is found, it pulls the correct name, description, publishers, developers, release date, ESRB rating and images. You can also add ROM collections that use MAME filenames. If you already have sets of images downloaded, EMU-nexus can use those instead of downloading new ones. Optionally, you can add PDF documents, other images and text notes to your games.


![EMU-nexus](https://www.dropbox.com/s/oocwel8euup9qdt/emu-nexus_screenshot_3.png?raw=1)


ROM files could be uncompressed on the go. The uncompressed files are stored in cache so they can be used directly next time. Tar, Tar.gz, Zip and 7zip formats are supported.


EMU-nexus stores all its data in **$HOME/.config/emu-nexus/**. Inside this directory, you will find the next items:

* **cache/**    Uncompressed ROMs are stored here. By default has a max limit of 9GB. When this limit is reached, the oldest files are deleted.
* **elasticseach/**    Contains the Elasticsearch binary and TheGamesDB concise database.
* **media/**    Contains the images of your ROM collection.
* **emu-nexus.sqlite**    SQLite database containing the data of your collection.


These are the libraries on which EMU-nexus was built on:

* **[Jansson](http://www.digip.org/jansson/)**    Library for encoding, decoding and manipulating JSON data. It is used to process the Elasticseach responses.
* **[SQLite](https://www.sqlite.org/index.html)**    SQL database engine. Used for managing your collection data.
* **[cURL](https://curl.haxx.se/libcurl/)**    Free and easy-to-use client-side URL transfer library. Used for making HTTP requests. A libcurl version with SSL support is required *(libcurl4-openssl-dev, libcurl-nss-dev or libcurl4-gnutls-dev)*.
* **[pthread (POSIX thread)](http://man7.org/linux/man-pages/man7/pthreads.7.html)**    Set of interfaces (functions, header files) for threaded programming. EMU-nexus uses multiple threads to smooth its performance.
* **[libxml2](http://www.xmlsoft.org/)**    XML C parser and toolkit developed for the Gnome project. Used to process MAME response data when processing ROMs that use MAME filenames.
* **[Dlib](http://dlib.net/)**    Modern C++ toolkit containing machine learning algorithms and tools for creating complex software in C++ to solve real world problems. EMU-nexus only uses a small set of Dlib to create thumbnails of the platforms and games artwork.
* **[Poppler](https://poppler.freedesktop.org/)**    PDF rendering library based on the xpdf-3.0 code base. Used for creating the thumbnails of game documents.
* **[libarchive](https://www.libarchive.org/)**    Multi-format archive and compression library. Used to uncompress ROM files on the go.
* **[GTK+ 3](https://www.gtk.org/)**    Multi-platform toolkit for creating graphical user interfaces.


No custom styles are used, the appearance relies on the system theme. The screenshots presented here are using the Gnome Adwaita-dark theme.


The icon is based in the work of [Pixel perfect](https://www.flaticon.com/authors/pixel-perfect).


To try it yourself download the source code or clone the repository and run these commands:

    cd <EMU-nexus source code directory>
    make
    sudo make install


