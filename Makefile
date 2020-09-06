OBJS=main.o Utils.o Directory.o CallbackResult.o Logger.o\
Database.o ApplicationVersion.o Preferences.o Platform.o PlatformImage.o Genre.o Developer.o Publisher.o EsrbRating.o Game.o GameGenre.o GameDeveloper.o GamePublisher.o GameImage.o GameDocument.o ApiDatabase.o GameCache.o GameActivity.o GameFavorite.o ParseDirectory.o ParseDirectoryGame.o\
thegamesdb.o\
GameLauncher.o\
HttpConnector.o\
NotificationManager.o\
SerialProcessExecutor.o SerialProcess.o ElasticsearchProcess.o ParseDirectoryProcess.o DownloadGameImagesProcess.o\
Asset.o MainWindow.o UiThreadBridge.o UiUtils.o\
Dialog.o MessageDialog.o PlatformEditDialog.o GameEditDialog.o DevelopersSelectDialog.o PublishersSelectDialog.o GenresSelectDialog.o GameSearchDialog.o AddDirectoryDialog.o GameDialog.o SelectFromListDialog.o\
Panel.o HomePanel.o GamesPanel.o PlatformPanel.o RecentsPanel.o FavoritePanel.o FirstSetupPanel.o\
Widget.o GameGridItemWidget.o MainBannerWidget.o GameBannerWidget.o\
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
VERSION=1.2

$(BIN) : $(OBJS)	
	$(CC) $(OBJS) $(LFLAGS) $(GTKFLAGS) -o $(BIN)

$(OBJS):
	sed s/{BUILD_VERSION}/$(VERSION)/ < $(BUILDH).template > $(SRCDIR)/$(BUILDH)
	$(CC) $(CFLAGS) $(GTKFLAGS) -I$(SRCDIR) -I$(SRCDIR)/data -I$(SRCDIR)/data/thegamesdb -I$(SRCDIR)/file -I$(SRCDIR)/launcher -I$(SRCDIR)/network -I$(SRCDIR)/serialprocesses -I$(SRCDIR)/notifications -I$(SRCDIR)/ui -I$(SRCDIR)/ui/dialogs -I$(SRCDIR)/ui/panels -I$(SRCDIR)/ui/widgets $(SRCDIR)/*.cpp $(SRCDIR)/data/*.cpp $(SRCDIR)/data/thegamesdb/*.cpp $(SRCDIR)/file/*.cpp $(SRCDIR)/launcher/*.cpp $(SRCDIR)/network/*.cpp $(SRCDIR)/serialprocesses/*.cpp $(SRCDIR)/notifications/*.cpp $(SRCDIR)/ui/*.cpp $(SRCDIR)/ui/dialogs/*.cpp $(SRCDIR)/ui/panels/*.cpp $(SRCDIR)/ui/widgets/*.cpp

clean:
	rm -f *.o *~ $(SRCDIR)/*~ $(ASSETSDIR)/*~ $(UIIMGDIR)/*~ $(BIN) $(BIN).desktop $(SRCDIR)/$(BUILDH)

install:
	if [ ! -d $(INSTALLASSETSDIR) ]; then mkdir $(INSTALLASSETSDIR); fi 
	rm -rf $(INSTALLASSETSDIR)/$(ASSETSDIR)
	cp -rf $(ASSETSDIR) $(INSTALLASSETSDIR)/$(ASSETSDIR)
	cp $(BIN) $(INSTALLBINDIR)
	sed s/{BUILD_VERSION}/$(VERSION)/ < $(BIN).desktop.template > $(BIN).desktop
	cp $(BIN).desktop /usr/share/applications
