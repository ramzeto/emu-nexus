OBJS=main.o Utils.o Directory.o Logger.o ThreadManager.o\
Database.o ApplicationVersion.o Preferences.o Platform.o PlatformImage.o Genre.o Developer.o Publisher.o EsrbRating.o Game.o GameGenre.o GameDeveloper.o GamePublisher.o GameImage.o GameDocument.o ApiDatabase.o GameCache.o GameActivity.o GameFavorite.o ParseDirectory.o ParseDirectoryGame.o\
thegamesdb.o\
GameLauncher.o\
HttpConnector.o\
NotificationManager.o Notification.o\
SerialProcessExecutor.o SerialProcess.o ParseDirectoryProcess.o DownloadGameImagesProcess.o SetupDatabaseProcess.o\
Asset.o MainWindow.o UiUtils.o\
Dialog.o MessageDialog.o PlatformEditDialog.o GameEditDialog.o DevelopersSelectDialog.o PublishersSelectDialog.o GenresSelectDialog.o GameSearchDialog.o AddDirectoryDialog.o SelectFromListDialog.o\
Panel.o HomePanel.o GamesPanel.o PlatformPanel.o RecentsPanel.o FavoritePanel.o\
Widget.o GameGridItemWidget.o MainBannerWidget.o MainBannerOverlayWidget.o SetupBannerOverlayWidget.o WelcomeBannerOverlayWidget.o GameBannerOverlayWidget.o\
FileExtractor.o\



CC=g++
DEBUG=-ggdb
CFLAGS=-Wall -c $(DEBUG) -std=c++11
LFLAGS=-Wall -ljansson -lsqlite3 -lcurl -lpthread -lxml2 -lm -ldlib -lpoppler-cpp -larchive $(DEBUG)
GTKFLAGS=-export-dynamic `pkg-config --cflags --libs gtk+-3.0`
BIN=emu-nexus
SRCDIR=./src
ASSETSDIR=assets
INSTALLBINDIR=/usr/bin
INSTALLASSETSDIR=/usr/share/$(BIN)
BUILDH=Build.h
VERSION=1.3

$(BIN) : $(OBJS)	
	$(CC) $(OBJS) $(LFLAGS) $(GTKFLAGS) -o $(BIN)

$(OBJS):
	sed s/{BUILD_VERSION}/$(VERSION)/ < $(BUILDH).template > $(SRCDIR)/$(BUILDH)
	$(CC) $(CFLAGS) $(GTKFLAGS) -I$(SRCDIR) -I$(SRCDIR)/data -I$(SRCDIR)/data/thegamesdb -I$(SRCDIR)/file -I$(SRCDIR)/launcher -I$(SRCDIR)/network -I$(SRCDIR)/serialprocess -I$(SRCDIR)/notification -I$(SRCDIR)/ui -I$(SRCDIR)/ui/dialog -I$(SRCDIR)/ui/panel -I$(SRCDIR)/ui/widget $(SRCDIR)/*.cpp $(SRCDIR)/data/*.cpp $(SRCDIR)/data/thegamesdb/*.cpp $(SRCDIR)/file/*.cpp $(SRCDIR)/launcher/*.cpp $(SRCDIR)/network/*.cpp $(SRCDIR)/serialprocess/*.cpp $(SRCDIR)/notification/*.cpp $(SRCDIR)/ui/*.cpp $(SRCDIR)/ui/dialog/*.cpp $(SRCDIR)/ui/panel/*.cpp $(SRCDIR)/ui/widget/*.cpp

clean:
	rm -f *.o *~ $(SRCDIR)/*~ $(ASSETSDIR)/*~ $(UIIMGDIR)/*~ $(BIN) $(BIN).desktop $(SRCDIR)/$(BUILDH)

install:
	if [ ! -d $(INSTALLASSETSDIR) ]; then mkdir $(INSTALLASSETSDIR); fi 
	rm -rf $(INSTALLASSETSDIR)/$(ASSETSDIR)
	cp -rf $(ASSETSDIR) $(INSTALLASSETSDIR)/$(ASSETSDIR)
	cp $(BIN) $(INSTALLBINDIR)
	sed s/{BUILD_VERSION}/$(VERSION)/ < $(BIN).desktop.template > $(BIN).desktop
	cp $(BIN).desktop /usr/share/applications
