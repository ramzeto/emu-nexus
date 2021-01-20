/*
 * Copyright (C) 2020 ram
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/* 
 * File:   GameBannerOverlayWidget.cpp
 * Author: ram
 * 
 * Created on September 4, 2020, 5:13 PM
 */

#include "GameBannerOverlayWidget.h"
#include "UiUtils.h"
#include "GameActivity.h"
#include "GameDeveloper.h"
#include "GamePublisher.h"
#include "GameGenre.h"
#include "Developer.h"
#include "Publisher.h"
#include "Genre.h"
#include "Utils.h"
#include "Preferences.h"
#include "Asset.h"
#include "Directory.h"
#include "EsrbRating.h"
#include "GameFavorite.h"
#include "Notifications.h"
#include "NotificationManager.h"
#include "GameLauncher.h"


const int GameBannerOverlayWidget::THUMBNAIL_IMAGE_WIDTH = 100;
const int GameBannerOverlayWidget::THUMBNAIL_IMAGE_HEIGHT = 100;

GameBannerOverlayWidget::GameBannerOverlayWidget(int64_t gameId) : MainBannerOverlayWidget("GameBannerOverlayWidget.ui", "gameBannerBox")
{
    gameImageEventBox = (GtkEventBox *)gtk_builder_get_object (builder, "gameImageEventBox");
    gameImage = (GtkImage *)gtk_builder_get_object (builder, "gameImage");
    nameLabel = (GtkLabel *)gtk_builder_get_object (builder, "nameLabel");
    fileNameLabel = (GtkLabel *)gtk_builder_get_object (builder, "fileNameLabel");
    imagesBox = (GtkBox *)gtk_builder_get_object (builder, "imagesBox");
    documentsBox = (GtkBox *)gtk_builder_get_object (builder, "documentsBox");
    informationLabel = (GtkLabel *)gtk_builder_get_object (builder, "informationLabel");
    playTimeLabel = (GtkLabel *)gtk_builder_get_object (builder, "playTimeLabel");
    lastPlayedLabel = (GtkLabel *)gtk_builder_get_object (builder, "lastPlayedLabel");
    launchButton = (GtkButton *)gtk_builder_get_object (builder, "launchButton");
    activityBox = (GtkBox *)gtk_builder_get_object (builder, "activityBox");
    spinner = (GtkSpinner *)gtk_builder_get_object (builder, "spinner");
    activityLabel = (GtkLabel *)gtk_builder_get_object (builder, "activityLabel");
    favoriteEventBox = (GtkEventBox *)gtk_builder_get_object (builder, "favoriteEventBox");
    favoriteImage = (GtkImage *)gtk_builder_get_object (builder, "favoriteImage");
    editEventBox = (GtkEventBox *)gtk_builder_get_object (builder, "editEventBox");
    editImage = (GtkImage *)gtk_builder_get_object (builder, "editImage");
    
    game = new Game(gameId);
    game->load();
    
    gameImages = GameImage::getItems(game->getId());
    gameDocuments = GameDocument::getItems(game->getId());
        
    gameImageBoxes = new map<GameImage *, GtkWidget *>;
    selectedGameImage = NULL;
    
    gameDocumentBoxes = new map<GameDocument *, GtkWidget *>;
    selectedGameDocument = NULL;
    
    selectGameImageTimestamp = 0;
    viewGameImageTimestamp = 0;
    selectGameImageBoxTimestamp = 0;
    viewGameImageBoxTimestamp = 0;
    selectGameDocumentBoxTimestamp = 0;
    viewGameDocumentBoxTimestamp = 0;    
    
    gtk_image_clear(gameImage);
    gtk_image_clear(favoriteImage);
    gtk_image_clear(editImage);
    
    
    g_signal_connect (gameImageEventBox, "button-press-event", G_CALLBACK(+[](GtkWidget* widget, GdkEvent* event, gpointer gameBannerOverlayWidget) -> gboolean {
        if(!((GameBannerOverlayWidget *)gameBannerOverlayWidget)->selectedGameImage)
        {
            return TRUE;
        }

        // Mouse left button
        if(event->button.button == 1)
        {
            time_t now = time(NULL);        
            if(now == ((GameBannerOverlayWidget *)gameBannerOverlayWidget)->selectGameImageTimestamp && (now - ((GameBannerOverlayWidget *)gameBannerOverlayWidget)->viewGameImageTimestamp) > 2)
            {
                 ((GameBannerOverlayWidget *)gameBannerOverlayWidget)->viewGameImageTimestamp = now;
                 ((GameBannerOverlayWidget *)gameBannerOverlayWidget)->viewGameImage(((GameBannerOverlayWidget *)gameBannerOverlayWidget)->selectedGameImage);
            }
            ((GameBannerOverlayWidget *)gameBannerOverlayWidget)->selectGameImageTimestamp = now;
        }
        // Mouse right button
        else if(event->button.button == 3)
        {
            GtkWidget *menu = gtk_menu_new();
            GtkWidget *viewMenuitem = gtk_menu_item_new_with_label("View");
            GtkWidget *saveMenuitem = gtk_menu_item_new_with_label("Save");

            gtk_menu_shell_append(GTK_MENU_SHELL(menu), viewMenuitem);
            g_signal_connect(viewMenuitem, "activate", G_CALLBACK(signalImageMenuViewActivate), gameBannerOverlayWidget);

            gtk_menu_shell_append(GTK_MENU_SHELL(menu), saveMenuitem);
            g_signal_connect(saveMenuitem, "activate", G_CALLBACK(signalImageMenuSaveActivate), gameBannerOverlayWidget);

            gtk_widget_show_all(menu);
            gtk_menu_popup_at_pointer(GTK_MENU(menu), event);        
        }

        return TRUE;
    }), this);
    
    g_signal_connect (favoriteEventBox, "button-press-event", G_CALLBACK(+[](GtkWidget *widget, GdkEvent *event, gpointer gameBannerOverlayWidget) -> gboolean {
        int favoriteImageWidth = gtk_widget_get_allocated_width(GTK_WIDGET(((GameBannerOverlayWidget *)gameBannerOverlayWidget)->favoriteImage));
        int favoriteImageHeight = gtk_widget_get_allocated_width(GTK_WIDGET(((GameBannerOverlayWidget *)gameBannerOverlayWidget)->favoriteImage));

        GameFavorite *gameFavorite = new GameFavorite(((GameBannerOverlayWidget *)gameBannerOverlayWidget)->game->getId());    
        if(gameFavorite->load())
        {
            gameFavorite->remove();
            UiUtils::getInstance()->loadImage(((GameBannerOverlayWidget *)gameBannerOverlayWidget)->favoriteImage, Asset::getInstance()->getImageNonFavorite(), favoriteImageWidth, favoriteImageHeight);
        }
        else
        {
            gameFavorite->setTimestamp(Utils::getInstance()->nowIsoDateTime());
            gameFavorite->save();
            gtk_image_set_from_icon_name(((GameBannerOverlayWidget *)gameBannerOverlayWidget)->favoriteImage, "emblem-favorite", GTK_ICON_SIZE_DND);
        }
        delete gameFavorite;

        NotificationManager::getInstance()->notify(NOTIFICATION_GAME_FAVORITE_UPDATED, "", 0, 0, new Game(*((GameBannerOverlayWidget *)gameBannerOverlayWidget)->game)); 

        return TRUE;
    }), this);
    
    g_signal_connect (editEventBox, "button-press-event", G_CALLBACK(+[](GtkWidget *widget, GdkEvent *event, gpointer gameBannerOverlayWidget) -> gboolean {
        NotificationManager::getInstance()->notify(NOTIFICATION_GAME_EDIT_REQUIRED, "", 0, 0, new Game(*((GameBannerOverlayWidget *)gameBannerOverlayWidget)->game));
        return TRUE;
    }), this);
    
    g_signal_connect (launchButton, "clicked", G_CALLBACK (+[](GtkButton* button, gpointer gameBannerOverlayWidget) -> void {
        if(!GameLauncher::getInstance()->launch(((GameBannerOverlayWidget *)gameBannerOverlayWidget)->game->getId()))
        {
            gtk_widget_hide(GTK_WIDGET(button));
        }
    }), this);
    
    NotificationManager::getInstance()->registerToNotification(NOTIFICATION_GAME_UPDATED, this, onNotification, 1);
    NotificationManager::getInstance()->registerToNotification(NOTIFICATION_GAME_FAVORITE_UPDATED, this, onNotification, 1);
    NotificationManager::getInstance()->registerToNotification(NOTIFICATION_GAME_ACTIVITY_UPDATED, this, onNotification, 1);
    NotificationManager::getInstance()->registerToNotification(NOTIFICATION_GAME_LAUNCHER_STATUS_CHANGED, this, onNotification, 1);
}

GameBannerOverlayWidget::~GameBannerOverlayWidget()
{
    NotificationManager::getInstance()->unregisterToNotification(NOTIFICATION_GAME_UPDATED, this);
    NotificationManager::getInstance()->unregisterToNotification(NOTIFICATION_GAME_FAVORITE_UPDATED, this);
    NotificationManager::getInstance()->unregisterToNotification(NOTIFICATION_GAME_ACTIVITY_UPDATED, this);
    NotificationManager::getInstance()->unregisterToNotification(NOTIFICATION_GAME_LAUNCHER_STATUS_CHANGED, this);        
    
    delete game;
    
    GameImage::releaseItems(gameImages);
    delete gameImageBoxes;
    
    GameDocument::releaseItems(gameDocuments);
    delete gameDocumentBoxes;
}

int64_t GameBannerOverlayWidget::getGameId() const
{
    return game->getId();
}

void GameBannerOverlayWidget::update()
{
    gtk_label_set_text(nameLabel, game->getName().c_str());
    gtk_widget_set_tooltip_text(GTK_WIDGET(nameLabel), game->getName().c_str());
    
    gtk_label_set_text(fileNameLabel, Utils::getInstance()->getFileRelativeName(game->getFileName()).c_str());
            
    list<GameActivity *> *gameActivities = GameActivity::getItems(-1, game->getId());
    if(gameActivities->size() > 0)
    {
        gtk_label_set_text(lastPlayedLabel, gameActivities->back()->getTimestamp().substr(0, 10).c_str());
                
        int64_t duration = 0; 
        for(unsigned int c = 0; c < gameActivities->size(); c++)
        {
            GameActivity *gameActivity = GameActivity::getItem(gameActivities, c);
            duration += gameActivity->getDuration();
        }
        
        int64_t hours = (duration / 3600);
        int64_t minutes = (duration % 3600) / 60;
        int64_t seconds = (duration % 3600) % 60;
        
        string playTimeText = "";
        if(hours > 0)
        {
            playTimeText += to_string(hours);
            if(hours == 1)
            {
                playTimeText += " hour ";
            }
            else
            {
                playTimeText += " hours ";
            }
        }
        if(hours < 50 && minutes > 0)
        {
            playTimeText += to_string(minutes);
            if(minutes == 1)
            {
                playTimeText += " minute ";
            }
            else
            {
                playTimeText += " minutes ";
            }
        }
        if(hours == 0 && minutes == 0 && seconds > 0)
        {
            playTimeText += to_string(seconds) + " seconds";
        }
        gtk_label_set_text(playTimeLabel, playTimeText.c_str());
    }
    else
    {
        gtk_label_set_text(lastPlayedLabel, "Never");
        gtk_label_set_text(playTimeLabel, "-");
    }
    GameActivity::releaseItems(gameActivities);
    
    
    string html = "";
    list<GameDeveloper *> *gameDevelopers = GameDeveloper::getItems(game->getId());
    if(gameDevelopers->size() > 0)
    {
        html += "<b>Developed by</b>\n";
        for(unsigned int c = 0; c < gameDevelopers->size(); c++)
        {
            if(c > 0)
            {
                html += ", ";
            }
            
            GameDeveloper *gameDeveloper = GameDeveloper::getItem(gameDevelopers, c);
            
            Developer *developer = new Developer(gameDeveloper->getDeveloperId());
            developer->load();
            
            html += Utils::getInstance()->htmlEntities(developer->getName());
            
            delete developer;
        }
        html += "\n\n";
    }
    GameDeveloper::releaseItems(gameDevelopers);
    
    list<GamePublisher *> *gamePublishers = GamePublisher::getItems(game->getId());
    if(gamePublishers->size() > 0)
    {
        html += "<b>Published by</b>\n";
        for(unsigned int c = 0; c < gamePublishers->size(); c++)
        {
            if(c > 0)
            {
                html += ", ";
            }
            
            GamePublisher *gamePublisher = GamePublisher::getItem(gamePublishers, c);
            
            Publisher *publisher = new Publisher(gamePublisher->getPublisherId());
            publisher->load();
            
            html += Utils::getInstance()->htmlEntities(publisher->getName());
            
            delete publisher;
        }
        html += "\n\n";
    }
    GamePublisher::releaseItems(gamePublishers);
    
    
    list<GameGenre *> *gameGenres = GameGenre::getItems(game->getId());
    if(gameGenres->size() > 0)
    {
        if(gameGenres->size() == 1)
        {
            html += "<b>Genre</b>\n";
        }
        else
        {
            html += "<b>Genres</b>\n";
        }
        for(unsigned int c = 0; c < gameGenres->size(); c++)
        {
            if(c > 0)
            {
                html += ", ";
            }
            
            GameGenre *gameGenre = GameGenre::getItem(gameGenres, c);
            
            Genre *genre = new Genre(gameGenre->getGenreId());
            genre->load();
            
            html += Utils::getInstance()->htmlEntities(genre->getName());
            
            delete genre;
        }
        html += "\n\n";
    }
    GameGenre::releaseItems(gameGenres);
    
    if(game->getReleaseDate().length() > 0)
    {
        html += "<b>Released</b>\n";
        html += Utils::getInstance()->htmlEntities(game->getReleaseDate());
        html += "\n\n";
    }
    
    if(game->getEsrbRatingId() > 0)
    {
        EsrbRating *esrbRating = new EsrbRating(game->getEsrbRatingId());
        if(esrbRating->load())
        {
            html += "<b>ESRB rating</b>\n";
            html += Utils::getInstance()->htmlEntities(esrbRating->getName());
            html += "\n\n";
        }
        delete esrbRating;
    }        
    
    if(game->getDescription().length() > 0)
    {
        html += "<b>Description</b>\n";
        html += Utils::getInstance()->htmlEntities(game->getDescription());
    }
    
    if(game->getNotes().length() > 0)
    {
        html += "\n\n";
        html += "<b>Notes</b>\n";
        html += Utils::getInstance()->htmlEntities(game->getNotes());
    }
    
    gtk_label_set_markup(informationLabel, html.c_str());
    
    
    int favoriteImageWidth = gtk_widget_get_allocated_width(GTK_WIDGET(favoriteImage));
    int favoriteImageHeight = gtk_widget_get_allocated_width(GTK_WIDGET(favoriteImage));
    GameFavorite *gameFavorite = new GameFavorite(game->getId());    
    if(gameFavorite->load())
    {
        gtk_image_set_from_icon_name(favoriteImage, "emblem-favorite", GTK_ICON_SIZE_LARGE_TOOLBAR);
    }
    else
    {
        UiUtils::getInstance()->loadImage(favoriteImage, Asset::getInstance()->getImageNonFavorite(), favoriteImageWidth, favoriteImageHeight);
    }
    delete gameFavorite;
           
    gtk_image_set_from_icon_name(editImage, "accessories-text-editor", GTK_ICON_SIZE_LARGE_TOOLBAR);
                
    updateGameImagesGrid();
    updateGameDocumentsGrid();
    updateLaunchStatus(GameLauncher::getInstance()->getStatus(), GameLauncher::getInstance()->getError());
}

void GameBannerOverlayWidget::updateGameImagesGrid()
{
    if(gameImages->size() == 0)
    {
        int width = gtk_widget_get_allocated_width(GTK_WIDGET(gameImage));
        int height = gtk_widget_get_allocated_width(GTK_WIDGET(gameImage));
        UiUtils::getInstance()->loadImage(gameImage, Asset::getInstance()->getImageNoGameImage(), width, height);        
        return;
    }
    
    gameImageBoxes->clear();
    UiUtils::getInstance()->clearContainer(GTK_CONTAINER(imagesBox), 1);
    
    int width = gtk_widget_get_allocated_width(GTK_WIDGET(imagesBox));
    int columns = width / THUMBNAIL_IMAGE_WIDTH;
    if(!columns)
    {
        return;
    }
    int rows = 1;

    unsigned int index = 0;
    for(int row = 0; row < rows; row++)
    {
        GtkBox *rowBox = (GtkBox *)gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
        
        for(int column = 0; column < columns; column++)
        {
            if(index < gameImages->size())
            {
                GameImage *gameImage = GameImage::getItem(gameImages, index);                               
                
                GtkBuilder *imageBoxBuilder = gtk_builder_new_from_file((Directory::getInstance()->getUiTemplatesDirectory() + "ImageBox.ui").c_str());
                GtkEventBox *imageBox = (GtkEventBox *)gtk_builder_get_object (imageBoxBuilder, "imageBox");
                GtkImage *image = (GtkImage *)gtk_builder_get_object (imageBoxBuilder, "image");

                if(Utils::getInstance()->fileExists(gameImage->getThumbnailFileName()))
                {
                    UiUtils::getInstance()->loadImage(image, gameImage->getThumbnailFileName(), THUMBNAIL_IMAGE_WIDTH - 10, THUMBNAIL_IMAGE_HEIGHT - 10);
                }
                else if(Utils::getInstance()->fileExists(gameImage->getFileName()))
                {
                    UiUtils::getInstance()->loadImage(image, gameImage->getFileName(), THUMBNAIL_IMAGE_WIDTH - 10, THUMBNAIL_IMAGE_HEIGHT - 10);
                }
                else
                {
                    gtk_image_set_from_icon_name(image, "emblem-downloads", GTK_ICON_SIZE_LARGE_TOOLBAR);
                }
                
                gtk_widget_set_name(GTK_WIDGET(imageBox), to_string(index).c_str());                                
                g_signal_connect (imageBox, "button-press-event", G_CALLBACK(signalImageBoxButtonPressedEvent), this);
                
                                
                if(selectedGameImage == gameImage)
                {
                    gtk_widget_set_state_flags(GTK_WIDGET(imageBox), GTK_STATE_FLAG_SELECTED, 1);
                }
                gameImageBoxes->insert(pair<GameImage*, GtkWidget *>(gameImage, GTK_WIDGET(imageBox)));
                
                gtk_widget_set_size_request(GTK_WIDGET(imageBox), THUMBNAIL_IMAGE_WIDTH, THUMBNAIL_IMAGE_HEIGHT);
                gtk_box_pack_start(rowBox, GTK_WIDGET(imageBox), 1, 1, 0);
                
                
                if(gameImage->getType() == GameImage::TYPE_BOX_FRONT)
                {
                    gtk_widget_set_tooltip_text(GTK_WIDGET(imageBox), "Box front");
                }
                else if(gameImage->getType() == GameImage::TYPE_BOX_BACK)
                {
                    gtk_widget_set_tooltip_text(GTK_WIDGET(imageBox), "Box back");
                }
                else if(gameImage->getType() == GameImage::TYPE_SCREENSHOT)
                {
                    gtk_widget_set_tooltip_text(GTK_WIDGET(imageBox), "Screenshot");
                }
                else if(gameImage->getType() == GameImage::TYPE_CLEAR_LOGO)
                {
                    gtk_widget_set_tooltip_text(GTK_WIDGET(imageBox), "Logo");
                }
                else if(gameImage->getType() == GameImage::TYPE_BANNER)
                {
                    gtk_widget_set_tooltip_text(GTK_WIDGET(imageBox), "Banner");
                }
                else if(gameImage->getType() == GameImage::TYPE_FANART)
                {
                    gtk_widget_set_tooltip_text(GTK_WIDGET(imageBox), "Fanart");
                }
            }
            else
            {
                GtkBox *dummyBox = (GtkBox *)gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
                
                gtk_widget_set_size_request(GTK_WIDGET(dummyBox), THUMBNAIL_IMAGE_WIDTH, THUMBNAIL_IMAGE_HEIGHT);
                gtk_box_pack_start(rowBox, GTK_WIDGET(dummyBox), 1, 1, 0);
            }
            
            index++;
        }
        
        gtk_box_pack_start(imagesBox, GTK_WIDGET(rowBox), 1, 1, 0);
        gtk_widget_show_all(GTK_WIDGET(rowBox));
    }
    
    if(!selectedGameImage && gameImages->size() > 0)
    {
        selectGameImage(GameImage::getItem(gameImages, 0));
    }
}

void GameBannerOverlayWidget::selectGameImage(GameImage *gameImage)
{
    if(!Utils::getInstance()->fileExists(gameImage->getFileName()))
    {
        return;
    }
    
    if(selectedGameImage)
    {
        if(gameImageBoxes->find(selectedGameImage) != gameImageBoxes->end())
        {
            GtkWidget *imageBox = gameImageBoxes->at(selectedGameImage);
            gtk_widget_set_state_flags(GTK_WIDGET(imageBox), GTK_STATE_FLAG_NORMAL, 1);
        }        
    }
    
    selectedGameImage = gameImage;
    if(gameImageBoxes->find(selectedGameImage) != gameImageBoxes->end())
    {
        GtkWidget *imageBox = gameImageBoxes->at(selectedGameImage);
        gtk_widget_set_state_flags(GTK_WIDGET(imageBox), GTK_STATE_FLAG_SELECTED, 1);
    }
    
    
    int width = gtk_widget_get_allocated_width(GTK_WIDGET(this->gameImage));
    int height = gtk_widget_get_allocated_width(GTK_WIDGET(this->gameImage));
    UiUtils::getInstance()->loadImage(this->gameImage, selectedGameImage->getFileName(), width, height);
}

void GameBannerOverlayWidget::viewGameImage(GameImage* gameImage)
{
    string tempDirectoryName = Utils::getInstance()->getTempFileName();
    Utils::getInstance()->makeDirectory(tempDirectoryName);
    
    string imageName = game->getName();
    imageName = Utils::getInstance()->strReplace(imageName, "/", "_");
    imageName += " (";        
    if(gameImage->getType() == GameImage::TYPE_BOX_FRONT)
    {
        imageName += "Box front";
    }
    else if(gameImage->getType() == GameImage::TYPE_BOX_BACK)
    {
        imageName += "Box back";
    }
    else if(gameImage->getType() == GameImage::TYPE_SCREENSHOT)
    {
        imageName += "Screenshot";
    }
    else if(gameImage->getType() == GameImage::TYPE_CLEAR_LOGO)
    {
        imageName += "Logo";
    }
    else if(gameImage->getType() == GameImage::TYPE_BANNER)
    {
        imageName += "Banner";
    }
    else if(gameImage->getType() == GameImage::TYPE_FANART)
    {
        imageName += "Fanart";
    }
    imageName += ")";
    imageName = tempDirectoryName + "/" + imageName;    
    
    if(!Utils::getInstance()->copyFile(gameImage->getFileName(), imageName))
    {
        Utils::getInstance()->openFileWithDefaultApplication(imageName);
    }
}

void GameBannerOverlayWidget::saveGameImage(GameImage* gameImage)
{
    string imageName = game->getName();
    imageName = Utils::getInstance()->strReplace(imageName, "/", "_");
    imageName += " (";        
    if(gameImage->getType() == GameImage::TYPE_BOX_FRONT)
    {
        imageName += "Box front";
    }
    else if(gameImage->getType() == GameImage::TYPE_BOX_BACK)
    {
        imageName += "Box back";
    }
    else if(gameImage->getType() == GameImage::TYPE_SCREENSHOT)
    {
        imageName += "Screenshot";
    }
    else if(gameImage->getType() == GameImage::TYPE_CLEAR_LOGO)
    {
        imageName += "Logo";
    }
    else if(gameImage->getType() == GameImage::TYPE_BANNER)
    {
        imageName += "Banner";
    }
    else if(gameImage->getType() == GameImage::TYPE_FANART)
    {
        imageName += "Fanart";
    }
    imageName += ")";

    GtkWidget *fileChooserDialog = gtk_file_chooser_dialog_new ("Save ", NULL, GTK_FILE_CHOOSER_ACTION_SAVE, "Cancel", GTK_RESPONSE_CANCEL, "Save", GTK_RESPONSE_ACCEPT, NULL);
    gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(fileChooserDialog), Preferences::getInstance()->getLastPath().c_str());
    gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER(fileChooserDialog), imageName.c_str());

    if (gtk_dialog_run (GTK_DIALOG (fileChooserDialog)) == GTK_RESPONSE_ACCEPT)
    {
        char *cFileName = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (fileChooserDialog));

        Preferences::getInstance()->setLastPath(Utils::getInstance()->getFileDirectory(string(cFileName)));
        Preferences::getInstance()->save();

        Utils::getInstance()->copyFile(gameImage->getFileName(), string(cFileName));

        g_free(cFileName);
    }
    gtk_widget_destroy(fileChooserDialog);
}

void GameBannerOverlayWidget::updateGameDocumentsGrid()
{
    if(gameDocuments->size() == 0)
    {
        gtk_widget_hide(GTK_WIDGET(documentsBox));        
        return;
    }
    
    gameDocumentBoxes->clear();
    UiUtils::getInstance()->clearContainer(GTK_CONTAINER(documentsBox), 1);
    
    int width = gtk_widget_get_allocated_width(GTK_WIDGET(documentsBox));
    int columns = width / THUMBNAIL_IMAGE_WIDTH;
    int rows = 1;
    /*int rows = gameDocuments->size() / columns;
    if(gameDocuments->size() % columns)
    {
        rows++;
    }*/

    unsigned int index = 0;
    for(int row = 0; row < rows; row++)
    {
        GtkBox *rowBox = (GtkBox *)gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
        
        for(int column = 0; column < columns; column++)
        {
            if(index < gameDocuments->size())
            {                
                GameDocument *gameDocument = GameDocument::getItem(gameDocuments, index);
                
                GtkBuilder *imageBoxBuilder = gtk_builder_new_from_file((Directory::getInstance()->getUiTemplatesDirectory() + "ImageBox.ui").c_str());
                GtkEventBox *imageBox = (GtkEventBox *)gtk_builder_get_object (imageBoxBuilder, "imageBox");
                GtkImage *image = (GtkImage *)gtk_builder_get_object (imageBoxBuilder, "image");

                UiUtils::getInstance()->loadImage(image, gameDocument->getPreviewImageFileName(), THUMBNAIL_IMAGE_WIDTH - 10, THUMBNAIL_IMAGE_HEIGHT - 10);
                
                gtk_widget_set_name(GTK_WIDGET(imageBox), to_string(index).c_str());                                
                g_signal_connect (imageBox, "button-press-event", G_CALLBACK(signalDocumentBoxButtonPressedEvent), this);                                
                
                                
                if(selectedGameDocument == gameDocument)
                {
                    gtk_widget_set_state_flags(GTK_WIDGET(imageBox), GTK_STATE_FLAG_SELECTED, 1);
                }
                gameDocumentBoxes->insert(pair<GameDocument*, GtkWidget *>(gameDocument, GTK_WIDGET(imageBox)));
                
                gtk_widget_set_size_request(GTK_WIDGET(imageBox), THUMBNAIL_IMAGE_WIDTH, THUMBNAIL_IMAGE_HEIGHT);
                gtk_box_pack_start(rowBox, GTK_WIDGET(imageBox), 1, 1, 0);
                
                if(gameDocument->getName().length() > 0)
                {
                    gtk_widget_set_tooltip_text(GTK_WIDGET(imageBox), gameDocument->getName().c_str());
                }
                else
                {
                    if(gameDocument->getType() == GameDocument::TYPE_BOOK)
                    {
                        gtk_widget_set_tooltip_text(GTK_WIDGET(imageBox), "Book");
                    }
                    else if(gameDocument->getType() == GameDocument::TYPE_GUIDE)
                    {
                        gtk_widget_set_tooltip_text(GTK_WIDGET(imageBox), "Guide");
                    }
                    else if(gameDocument->getType() == GameDocument::TYPE_MAGAZINE)
                    {
                        gtk_widget_set_tooltip_text(GTK_WIDGET(imageBox), "Magazine");
                    }
                    else if(gameDocument->getType() == GameDocument::TYPE_MANUAL)
                    {
                        gtk_widget_set_tooltip_text(GTK_WIDGET(imageBox), "Manual");
                    }
                    else if(gameDocument->getType() == GameDocument::TYPE_OTHER)
                    {
                        gtk_widget_set_tooltip_text(GTK_WIDGET(imageBox), "Other");
                    }                    
                }
            }
            else
            {
                GtkBox *dummyBox = (GtkBox *)gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
                
                gtk_widget_set_size_request(GTK_WIDGET(dummyBox), THUMBNAIL_IMAGE_WIDTH, THUMBNAIL_IMAGE_HEIGHT);
                gtk_box_pack_start(rowBox, GTK_WIDGET(dummyBox), 1, 1, 0);
            }
            
            index++;
        }
        
        gtk_box_pack_start(documentsBox, GTK_WIDGET(rowBox), 1, 1, 0);
        gtk_widget_show_all(GTK_WIDGET(rowBox));
    }
}

void GameBannerOverlayWidget::selectGameDocument(GameDocument *gameDocument)
{
    if(selectedGameDocument)
    {
        if(gameDocumentBoxes->find(selectedGameDocument) != gameDocumentBoxes->end())
        {
            GtkWidget *imageBox = gameDocumentBoxes->at(selectedGameDocument);
            gtk_widget_set_state_flags(GTK_WIDGET(imageBox), GTK_STATE_FLAG_NORMAL, 1);
        }        
    }
    
    selectedGameDocument = gameDocument;
    if(gameDocumentBoxes->find(selectedGameDocument) != gameDocumentBoxes->end())
    {
        GtkWidget *imageBox = gameDocumentBoxes->at(selectedGameDocument);
        gtk_widget_set_state_flags(GTK_WIDGET(imageBox), GTK_STATE_FLAG_SELECTED, 1);
    }
}

void GameBannerOverlayWidget::viewGameDocument(GameDocument *gameDocument)
{    
    string tempDirectoryName = Utils::getInstance()->getTempFileName();
    Utils::getInstance()->makeDirectory(tempDirectoryName);
    
    string name = gameDocument->getName();
    if(name.length() == 0)
    {
        if(gameDocument->getType() == GameDocument::TYPE_BOOK)
        {
            name = "Book";
        }
        else if(gameDocument->getType() == GameDocument::TYPE_GUIDE)
        {
            name = "Guide";
        }
        else if(gameDocument->getType() == GameDocument::TYPE_MAGAZINE)
        {
            name = "Magazine";
        }
        else if(gameDocument->getType() == GameDocument::TYPE_MANUAL)
        {
            name = "Manual";
        }
        else if(gameDocument->getType() == GameDocument::TYPE_OTHER)
        {
            name = "Other";
        }
    }
    
    string documentName = game->getName() + " (" + gameDocument->getName() + ")";
    documentName = Utils::getInstance()->strReplace(documentName, "/", "_");
    documentName = tempDirectoryName + "/" + documentName;    
    
    if(!Utils::getInstance()->copyFile(gameDocument->getFileName(), documentName))
    {
        Utils::getInstance()->openFileWithDefaultApplication(documentName);
    }
}

void GameBannerOverlayWidget::saveGameDocument(GameDocument *gameDocument)
{
    string imageName = game->getName() + " (" + gameDocument->getName() + ")";
    imageName = Utils::getInstance()->strReplace(imageName, "/", "_");
        
    GtkWidget *fileChooserDialog = gtk_file_chooser_dialog_new ("Save ", NULL, GTK_FILE_CHOOSER_ACTION_SAVE, "Cancel", GTK_RESPONSE_CANCEL, "Save", GTK_RESPONSE_ACCEPT, NULL);
    gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(fileChooserDialog), Preferences::getInstance()->getLastPath().c_str());
    gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER(fileChooserDialog), imageName.c_str());

    if (gtk_dialog_run (GTK_DIALOG (fileChooserDialog)) == GTK_RESPONSE_ACCEPT)
    {
        char *cFileName = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (fileChooserDialog));

        Preferences::getInstance()->setLastPath(Utils::getInstance()->getFileDirectory(string(cFileName)));        
        Preferences::getInstance()->save();

        Utils::getInstance()->copyFile(gameDocument->getFileName(), string(cFileName));
        
        g_free(cFileName);
    }
    gtk_widget_destroy(fileChooserDialog);
}

void GameBannerOverlayWidget::updateLaunchStatus(int status, int error, int progress)
{
    if(status == GameLauncher::STATUS_IDLE)
    {
        gtk_widget_hide(GTK_WIDGET(activityBox));
        gtk_widget_show_all(GTK_WIDGET(launchButton));
        gtk_widget_show_all(GTK_WIDGET(editEventBox));
        
        return;
    }
    
    gtk_widget_hide(GTK_WIDGET(launchButton));
    gtk_widget_hide(GTK_WIDGET(editEventBox));
    gtk_widget_show_all(GTK_WIDGET(activityBox));
    
    string message = "";
    if(status == GameLauncher::STATUS_STARTING)
    {
        message = "Starting...";
    }
    else if(status == GameLauncher::STATUS_INFLATING)
    {
        message = "Inflating...";
        
        if(progress >= 0)
        {
            message += "(" + to_string(progress) + "%)";
        }
    }
    else if(status == GameLauncher::STATUS_SELECTING_FILE)
    {
        message = "Selecting...";
    }
    else if(status == GameLauncher::STATUS_FOUND_MULTIPLE_FILES)
    {
        message = "Multiple files";
    }        
    else if(status == GameLauncher::STATUS_RUNNING)
    {
        message = "Running...";
    }
    else if(status == GameLauncher::STATUS_FINISHED)
    {
        message = "Finished";
    }
    
    gtk_label_set_text(activityLabel, message.c_str());
}

gboolean GameBannerOverlayWidget::signalImageBoxButtonPressedEvent(GtkWidget *widget, GdkEvent *event, gpointer gameBannerOverlayWidget)
{
    // Mouse left button
    if(event->button.button == 1)
    {
        GameImage *gameImage = GameImage::getItem(((GameBannerOverlayWidget *)gameBannerOverlayWidget)->gameImages, atoi(gtk_widget_get_name(widget)));
        
        if(gameImage == ((GameBannerOverlayWidget *)gameBannerOverlayWidget)->selectedGameImage)
        {
            time_t now = time(NULL);
            if(now == ((GameBannerOverlayWidget *)gameBannerOverlayWidget)->selectGameImageBoxTimestamp && (now - ((GameBannerOverlayWidget *)gameBannerOverlayWidget)->viewGameImageBoxTimestamp) > 2)
            {
                 ((GameBannerOverlayWidget *)gameBannerOverlayWidget)->viewGameImageBoxTimestamp = now;
                 ((GameBannerOverlayWidget *)gameBannerOverlayWidget)->viewGameImage(((GameBannerOverlayWidget *)gameBannerOverlayWidget)->selectedGameImage);
            }
            ((GameBannerOverlayWidget *)gameBannerOverlayWidget)->selectGameImageBoxTimestamp = now;
        }
        else
        {
            ((GameBannerOverlayWidget *)gameBannerOverlayWidget)->selectGameImage(gameImage);
        }                
    }    
    
    return TRUE;
}

void GameBannerOverlayWidget::signalImageMenuViewActivate(GtkMenuItem* menuitem, gpointer gameBannerOverlayWidget)
{
    ((GameBannerOverlayWidget *)gameBannerOverlayWidget)->viewGameImage(((GameBannerOverlayWidget *)gameBannerOverlayWidget)->selectedGameImage);    
}

void GameBannerOverlayWidget::signalImageMenuSaveActivate(GtkMenuItem* menuitem, gpointer gameBannerOverlayWidget)
{
    ((GameBannerOverlayWidget *)gameBannerOverlayWidget)->saveGameImage(((GameBannerOverlayWidget *)gameBannerOverlayWidget)->selectedGameImage);
}

gboolean GameBannerOverlayWidget::signalDocumentBoxButtonPressedEvent(GtkWidget *widget, GdkEvent *event, gpointer gameBannerOverlayWidget)
{        
    // Mouse left button
    if(event->button.button == 1)
    {
        GameDocument *gameDocument = GameDocument::getItem(((GameBannerOverlayWidget *)gameBannerOverlayWidget)->gameDocuments, atoi(gtk_widget_get_name(widget)));
        if(gameDocument == ((GameBannerOverlayWidget *)gameBannerOverlayWidget)->selectedGameDocument)
        {
            time_t now = time(NULL);
            if(now == ((GameBannerOverlayWidget *)gameBannerOverlayWidget)->selectGameDocumentBoxTimestamp && (now - ((GameBannerOverlayWidget *)gameBannerOverlayWidget)->viewGameDocumentBoxTimestamp) > 2)
            {
                 ((GameBannerOverlayWidget *)gameBannerOverlayWidget)->viewGameDocumentBoxTimestamp = now;
                 ((GameBannerOverlayWidget *)gameBannerOverlayWidget)->viewGameDocument(((GameBannerOverlayWidget *)gameBannerOverlayWidget)->selectedGameDocument);
            }
            ((GameBannerOverlayWidget *)gameBannerOverlayWidget)->selectGameDocumentBoxTimestamp = now;
        }
        else
        {
            ((GameBannerOverlayWidget *)gameBannerOverlayWidget)->selectGameDocument(gameDocument);
        }
    }
    // Mouse right button
    else if(event->button.button == 3)
    {
        GameDocument *gameDocument = GameDocument::getItem(((GameBannerOverlayWidget *)gameBannerOverlayWidget)->gameDocuments, atoi(gtk_widget_get_name(widget)));
        ((GameBannerOverlayWidget *)gameBannerOverlayWidget)->selectGameDocument(gameDocument);
    
        GtkWidget *menu = gtk_menu_new();
        GtkWidget *viewMenuitem = gtk_menu_item_new_with_label("View");
        GtkWidget *saveMenuitem = gtk_menu_item_new_with_label("Save");

        gtk_menu_shell_append(GTK_MENU_SHELL(menu), viewMenuitem);
        g_signal_connect(viewMenuitem, "activate", G_CALLBACK(signalDocumentMenuViewActivate), gameBannerOverlayWidget);
        
        gtk_menu_shell_append(GTK_MENU_SHELL(menu), saveMenuitem);
        g_signal_connect(saveMenuitem, "activate", G_CALLBACK(signalDocumentMenuSaveActivate), gameBannerOverlayWidget);
        
        gtk_widget_show_all(menu);
        gtk_menu_popup_at_pointer(GTK_MENU(menu), event);        
    }
    
    
    return TRUE;
}

void GameBannerOverlayWidget::signalDocumentMenuViewActivate(GtkMenuItem *menuitem, gpointer gameBannerOverlayWidget)
{
    ((GameBannerOverlayWidget *)gameBannerOverlayWidget)->viewGameDocument(((GameBannerOverlayWidget *)gameBannerOverlayWidget)->selectedGameDocument);
}

void GameBannerOverlayWidget::signalDocumentMenuSaveActivate(GtkMenuItem *menuitem, gpointer gameBannerOverlayWidget)
{
    ((GameBannerOverlayWidget *)gameBannerOverlayWidget)->saveGameDocument(((GameBannerOverlayWidget *)gameBannerOverlayWidget)->selectedGameDocument);
}

void GameBannerOverlayWidget::onNotification(Notification *notification)
{
    GameBannerOverlayWidget *gameBannerOverlayWidget = (GameBannerOverlayWidget *)notification->getListener();

    if(notification->getName().compare(NOTIFICATION_GAME_UPDATED) == 0)
    {
        Game *game = (Game *)notification->getData();
        if(gameBannerOverlayWidget->game->getId() == game->getId())
        {
            gameBannerOverlayWidget->game->load();

            gameBannerOverlayWidget->selectedGameImage = NULL;
            GameImage::releaseItems(gameBannerOverlayWidget->gameImages);
            gameBannerOverlayWidget->gameImages = GameImage::getItems(game->getId());
            
            gameBannerOverlayWidget->selectedGameDocument = NULL;
            GameDocument::releaseItems(gameBannerOverlayWidget->gameDocuments);
            gameBannerOverlayWidget->gameDocuments = GameDocument::getItems(game->getId());
            
            gameBannerOverlayWidget->update();
        }
    }
    else if(notification->getName().compare(NOTIFICATION_GAME_ACTIVITY_UPDATED) == 0)
    {
        Game *game = (Game *)notification->getData();

        if(gameBannerOverlayWidget->game->getId() == game->getId())
        {
            gameBannerOverlayWidget->game->load();
            gameBannerOverlayWidget->update();
        }
    }
    else if(notification->getName().compare(NOTIFICATION_GAME_FAVORITE_UPDATED) == 0)
    {
        Game *game = (Game *)notification->getData();

        if(gameBannerOverlayWidget->game->getId() == game->getId())
        {
            gameBannerOverlayWidget->game->load();
            gameBannerOverlayWidget->update();
        }
    }
    else if(notification->getName().compare(NOTIFICATION_GAME_LAUNCHER_STATUS_CHANGED) == 0)
    {
        gameBannerOverlayWidget->updateLaunchStatus(notification->getStatus(), notification->getError(), notification->getProgress());
    }
}