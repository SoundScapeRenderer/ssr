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

/// @file qgui.h
/// GUI wrapper class (definition).

#ifndef SSR_QGUI_H
#define SSR_QGUI_H

#include <QtCore/QObject>
#include <QtOpenGL/QGLFormat>
#include <QtWidgets/QApplication>

#include "quserinterface.h"
#include "publisher.h"
#include "scene.h"

namespace ssr
{

/** GUI wrapper class.
 * This class allows for comfortable creation, running, and stopping of the
 * SSR graphical user interface.
 **/
class QGUI : public QObject
{
  Q_OBJECT

  public:
    QGUI(Publisher& controller, const Scene& scene, int &argc
        , char *argv[], const std::string& path_to_gui_images
        , const std::string& path_to_scene_menu);

    ~QGUI();

    QGLFormat format () const;
    int run(); ///< start the GUI.

  private:
    QApplication _qt_app; ///< every Qt application has this
    QUserInterface _gui;  ///< the main GUI class
};

}  // namespace ssr

#endif

// Settings for Vim (http://www.vim.org/), please do not remove:
// vim:softtabstop=2:shiftwidth=2:expandtab:textwidth=80:cindent
// vim:fdm=expr:foldexpr=getline(v\:lnum)=~'/\\*\\*'&&getline(v\:lnum)!~'\\*\\*/'?'a1'\:getline(v\:lnum)=~'\\*\\*/'&&getline(v\:lnum)!~'/\\*\\*'?'s1'\:'='
