/******************************************************************************
 * Copyright © 2012-2014 Institut für Nachrichtentechnik, Universität Rostock *
 * Copyright © 2006-2012 Quality & Usability Lab,                             *
 *                       Telekom Innovation Laboratories, TU Berlin           *
 *                                                                            *
 * This file is part of the SoundScape Renderer (SSR).                        *
 *                                                                            *
 * The SSR is free software:  you can redistribute it and/or modify it  under *
 * the terms of the  GNU  General  Public  License  as published by the  Free *
 * Software Foundation, either version 3 of the License,  or (at your option) *
 * any later version.                                                         *
 *                                                                            *
 * The SSR is distributed in the hope that it will be useful, but WITHOUT ANY *
 * WARRANTY;  without even the implied warranty of MERCHANTABILITY or FITNESS *
 * FOR A PARTICULAR PURPOSE.                                                  *
 * See the GNU General Public License for more details.                       *
 *                                                                            *
 * You should  have received a copy  of the GNU General Public License  along *
 * with this program.  If not, see <http://www.gnu.org/licenses/>.            *
 *                                                                            *
 * The SSR is a tool  for  real-time  spatial audio reproduction  providing a *
 * variety of rendering algorithms.                                           *
 *                                                                            *
 * http://spatialaudio.net/ssr                           ssr@spatialaudio.net *
 ******************************************************************************/

/// @file
/// GUI wrapper class (implementation).

#include "qgui.h"

// Qt stylesheet

QString qt_style_sheet =
#ifdef __APPLE__
  "QWidget {                                               \n"
  "   color: black;                                        \n"
  "   font: normal 13pt;                                   \n"
  "   }                                                    \n"
#else
  "QWidget {                                               \n"
  "   color: black;                                        \n"
  "   font: normal 9pt \"Monospace\";                      \n"
  "   }                                                    \n"
#endif
  "QLabel {                                                \n"
  "   background-color: rgb(244, 244, 244);                \n"
  "   color: black;                                        \n"
  "   }                                                    \n"
  "                                                        \n"
  "QLabel#notifier {                                       \n"
  "   background-color: rgb(244, 0, 0, 255);               \n"
  /* "   color: black;                                        \n"*/
  "   }                                                    \n"
  "                                                        \n"
  // define style of processing button
  "QPushButton#_processing_button {                        \n"
  "   border-image: url(images/processing_button.png) 0;   \n"
  "   }                                                    \n"
  "                                                        \n"
  "QPushButton#_processing_button:pressed {                 \n"
  "   border-image: url(images/processing_button_pressed.png) 0; \n"
  "   }                                                    \n"
  "                                                        \n"
  "QPushButton#_processing_button:checked {                \n"
  "   border-image: url(images/processing_button_pressed.png)0; \n"
  "   }                                                    \n"
  "                                                        \n"
  // define style of stop button
  "QPushButton#_skip_back_button {                          \n"
  "   border-image: url(images/skip_back_button.png) 0;     \n"
  "   }                                                     \n"
  "                                                         \n"
  "QPushButton#_skip_back_button:pressed {                  \n"
  "   border-image: url(images/skip_back_button_pressed.png) 0; \n"
  "   }                                                    \n"
  "                                                        \n"
  // define style of pause button
  "QPushButton#_pause_button {                             \n"
  "   border-image: url(images/pause_button.png)0;         \n"
  "   }                                                    \n"
  "                                                        \n"
  "QPushButton#_pause_button:checked {                     \n"
  "   border-image: url(images/pause_button_pressed.png)0; \n"
  "   }                                                    \n"
  "                                                        \n"
  "QPushButton#_pause_button:pressed {                     \n"
  "   border-image: url(images/pause_button_pressed.png)0; \n"
  "   }                                                    \n"
  "                                                        \n"
  // define style of play button
  "QPushButton#_play_button {                              \n"
  "   border-image: url(images/play_button.png)0;          \n"
  "   }                                                    \n"
  "                                                        \n"
  "QPushButton#_play_button:checked {                      \n"
  "   border-image: url(images/play_button_pressed.png)0;  \n"
  "   }                                                    \n"
  "                                                        \n"
  "QPushButton#_play_button:pressed {                      \n"
  "   border-image: url(images/play_button_pressed.png)0;  \n"
  "   }                                                    \n"
  "                                                        \n"
  // define style of other labels
  "QSceneButton {                                          \n"
  "   border-image: url(images/scene_menu_item.png)0;       \n"
  "   }                                                    \n"
  "                                                        \n"
  "QSceneButton:pressed {                                  \n"
  "   border-image: url(images/scene_menu_item_selected.png)0; \n"
  "   }                                                    \n"
  "                                                        \n"
  "QSceneButton:checked {                                  \n"
  "   border-image: url(images/scene_menu_item_selected.png)0; \n"
  "   }                                                    \n"
  "                                                        \n"
  "QVolumeSlider {                                         \n"
  "   margin: 4px;                                         \n"
  "   background-color: rgb(244, 244, 244);                \n"
  //"   border-width: 1px;                                   \n"
  //"   border-color: red;                                   \n"
  //"   border-style: solid;                                 \n"
  "   border-radius: 0;                                    \n"
  "   }                                                    \n"
  "                                                        \n"
  "QTimeLine {                                             \n"
  "    background-color: rgb(244, 244, 244);               \n"
  "   }                                                    \n"
  "                                                        \n"
  "QCPULabel {                                             \n"
  "    background-color: white;                            \n"
  "   }                                                    \n"
  "                                                        \n"
  "                                                        \n"
  "QZoomLabel {                                            \n"
  "    background-color: white;                            \n"
  "   }                                                    \n"
  "                                                        \n"
  "QFileMenuLabel {                                        \n"
  "   background-color: white;                             \n"
  "   font: bold 11pt;                                     \n"
  //"   border-width: 1px;                                   \n"
  //"   border-color: red;                                   \n"
  //"   border-style: solid;                                 \n"
  "   border-radius: 0;                                    \n"
  "   }                                                    \n"
  "                                                        \n"
  // define style of popup menu
  "QMenu {                                                 \n"
  "   background-color: rgba(255, 255, 255, 92%);          \n"
  "   border-width: 1px;                                   \n"
  "   border-color: black;                                 \n"
  "   border-style: solid;                                 \n"
  "   border-radius: 0;                                    \n"
  "   }                                                    \n"
  "                                                        \n"
  "QMenu::item:hover {                                     \n"
  "   selection-background-color: rgba(208, 245, 247, 92%);\n"
  "   selection-color: black;                              \n"
  "   }                                                    \n"
  "QSourceProperties {                                     \n"
  "   background-color: rgba(255, 255, 255, 92%);          \n"
  "   border-width: 1px;                                   \n"
  "   border-color: black;                                 \n"
  "   border-style: solid;                                 \n"
  "   border-radius: 0;                                    \n"
  "   }                                                    \n"
  "                                                        \n"
  "QLabel#item {                                           \n"
  "   background-color: rgb(255, 255, 255);                \n"
  "   color: black;                                        \n"
  "   }                                                    \n"
  "                                                        \n"
  "QClicktextLabel#item:hover {                            \n"
  "   background-color: rgba(208, 245, 247, 92%);          \n"
  "   color: red;                                          \n"
  "   }                                                    \n"
  "                                                        \n"
  "QRadioButton {                                          \n"
  "   background-color: rgb(255, 255, 255);                \n"
  "   color: black;                                        \n"
  "   }                                                    \n"
  "                                                        \n"
  "QCheckBox {                                             \n"
  "   background-color: rgb(255, 255, 255);                \n"
  "   color: black;                                        \n"
  "   }                                                    \n"
  "                                                        \n"
  ; // don't forget this semi-colon

/** ctor.
 * @param controller this can be either the Controller or, if the IP interface
 * is used, an IPClient.
 * @param scene reference to the MasterScene or to a client-side copy of the
 * Scene.
 * @param argc number of command line arguments passed to the GUI.
 * @param argv the arguments themselves.
 **/
ssr::QGUI::QGUI(Publisher& controller, const Scene& scene, int &argc, char *argv[]
        , const std::string& path_to_gui_images
        , const std::string& path_to_scene_menu) :
  _qt_app(argc, argv), _gui(controller, scene
    , path_to_gui_images, path_to_scene_menu)
{
  // this is a quick hack to allow dynamic specification of path
  qt_style_sheet.replace(QString("images"), QString( path_to_gui_images.c_str() ));

  // set stylesheet
  _qt_app.setStyleSheet(qt_style_sheet);
}

ssr::QGUI::~QGUI()
{}

/**
 * This function is used to verify if setting the sample buffer (enabling 
 * anti-aliasing) has succeeded.
 * @return current QGLFormat
 */
QGLFormat ssr::QGUI::format() const
{
  return _gui.format();
}

/**
 * This function starts the GUI.
 * @return application exit code
*/
int ssr::QGUI::run()
{
  // TODO: check return values
  _gui.show();
  
  // bring window to front
  _gui.raise();
  _gui.activateWindow();

  // TODO: check if _qt_app is valid
  _qt_app.connect(&_qt_app, SIGNAL(lastWindowClosed()), &_qt_app, SLOT(quit()));
  return _qt_app.exec();
}

// Settings for Vim (http://www.vim.org/), please do not remove:
// vim:softtabstop=2:shiftwidth=2:expandtab:textwidth=80:cindent
