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
/// QZoomLabel

#ifndef SSR_QZOOMLABEL_H
#define SSR_QZOOMLABEL_H

#include <QtCore/QPoint>
#include <QtGui/QMouseEvent>
#include <QtGui/QPaintEvent>
#include <QtWidgets/QLabel>

/// QZoomLabel
class QZoomLabel : public QLabel
{
  Q_OBJECT

  public:
    QZoomLabel( QWidget* parent = 0);

  protected:
    QPoint starting_point;
    int zoom;
    int zoom_buffer;

    void  set_zoom_buffer(int z);
    float distance(QPoint start, QPoint end);
    void  mousePressEvent(QMouseEvent *event);
    void  mouseMoveEvent(QMouseEvent *event);
    void  mouseDoubleClickEvent(QMouseEvent *event);
    void  mouseReleaseEvent(QMouseEvent *event);
    void  paintEvent( QPaintEvent * event);

    protected slots:
      void update_display(int z);

  signals:
    void signal_zoom_changed(int);
};

#endif

// Settings for Vim (http://www.vim.org/), please do not remove:
// vim:softtabstop=2:shiftwidth=2:expandtab:textwidth=80:cindent
// vim:fdm=expr:foldexpr=getline(v\:lnum)=~'/\\*\\*'&&getline(v\:lnum)!~'\\*\\*/'?'a1'\:getline(v\:lnum)=~'\\*\\*/'&&getline(v\:lnum)!~'/\\*\\*'?'s1'\:'='
