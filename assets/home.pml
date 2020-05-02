<span size="large"><b>EMU-nexus</b> is a front-end designed to launch emulators and to organize ROM collections. It is developed in C++ and it uses GTK 3 as graphical user interface. It does not try to compete with any other front-end and it is not designed for an HTPC. It is intended to be used in an everyday not specific purpose PC. It is distributed under the <a href="https://www.gnu.org/licenses/gpl-3.0.en.html">GPLv3 license</a>.</span>


<span size="large">Information and images of the consoles, systems and games comes from <a href="https://thegamesdb.net/">TheGamesDB</a>. To avoid stress on TheGamesDB servers, a concise version of the database is processed in the cloud. This concise version is updated regularly and downloaded automatically.</span>


<span size="large">This database is stored and indexed using <a href="https://www.elastic.co/downloads/elasticsearch-oss">Elasticsearch</a> so it can be easily searchable. When <b>EMU-nexus</b> is launched, it starts the Elasticsearch process in the background. By default, Elasticsearch listens in the port 9301. The <a href="https://www.apache.org/licenses/LICENSE-2.0">Apache 2.0</a> licensed version of Elasticseach is used.</span>


<span size="large">EMU-nexus scans specific directories and matches files with entries in the database, if a match is found, it pulls the correct name, description, publishers, developers, release date, ESRB rating and images. You can also add ROM collections that use MAME filenames. If you already have sets of images downloaded, EMU-nexus can use those instead of downloading new ones. Optionally, you can add PDF documents, other images and text notes to your games.</span>


<span size="large">ROM files could be uncompressed on the go. The uncompressed files are stored in cache so they can be used directly next time. Tar, Tar.gz, Zip and 7zip formats are supported.</span>


<span size="large"><b>EMU-nexus</b> stores all its data in <i>$HOME/.config/emu-nexus/</i>. Inside this directory, you will find the next items:</span>


<span size="large">     * <i>cache/</i>    Uncompressed ROMs are stored here. By default has a max limit of 9GB. When this limit is reached, the oldest files are deleted.</span>
<span size="large">     * <i>elasticseach/</i>    Contains the Elasticsearch binary and TheGamesDB concise database.</span>
<span size="large">     * <i>media/</i>    Contains the images of your ROM collection.</span>
<span size="large">     * <i>emu-nexus.sqlite</i>    SQLite database containing the data of your collection.</span>


<span size="large"><b>EMU-nexus</b> source code is available in its <a href="https://github.com/ramzeto/emu-nexus">Github repository</a>. These are the libraries on which <b>EMU-nexus</b> was built on:</span>


<span size="large">     * <a href="http://www.digip.org/jansson/"><i>Jansson</i></a>    Library for encoding, decoding and manipulating JSON data. It is used to process the Elasticseach responses.</span>
<span size="large">     * <a href="https://www.sqlite.org/index.html"><i>SQLite</i></a>    SQL database engine. Is used for managing your collection data.</span>
<span size="large">     * <a href="https://curl.haxx.se/libcurl/"><i>cURL</i></a>    Free and easy-to-use client-side URL transfer library. Used for making HTTP request. A libcurl version with SSL support is required <i>(libcurl4-openssl-dev, libcurl-nss-dev or libcurl4-gnutls-dev)</i>.</span>
<span size="large">     * <a href="http://man7.org/linux/man-pages/man7/pthreads.7.html"><i>pthread (POSIX thread)</i></a>    Set of interfaces (functions, header files) for threaded programming. <b>EMU-nexus</b> uses multiple threads to smooth its performance.</span>
<span size="large">     * <a href="http://www.xmlsoft.org/"><i>libxml2</i></a>    XML C parser and toolkit developed for the Gnome project. Used to process MAME response data when processing ROMs that use MAME filenames.</span>
<span size="large">     * <a href="http://dlib.net/"><i>Dlib</i></a>    Modern C++ toolkit containing machine learning algorithms and tools for creating complex software in C++ to solve real world problems. <b>EMU-nexus</b> only uses a small set of Dlib to create thumbnails of the platforms and games artwork.</span>
<span size="large">     * <a href="https://poppler.freedesktop.org/"><i>Poppler</i></a>    PDF rendering library based on the xpdf-3.0 code base. Used for creating the thumbnails of game documents.</span>
<span size="large">     * <a href="https://www.libarchive.org/"><i>libarchive</i></a>    Multi-format archive and compression library. Used to uncompress ROM archives on the go.</span>
<span size="large">     * <a href="https://www.gtk.org/"><i>GTK+ 3</i></a>    Multi-platform toolkit for creating graphical user interfaces.</span>


<span size="large">No custom styles are used, the appearance relies on the system theme.</span>


<span size="large">The icon is based in the work of <a href="https://www.flaticon.com/authors/pixel-perfect">Pixel perfect</a>.</span>