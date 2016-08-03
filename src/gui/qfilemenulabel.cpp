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
/// TODO: add description

#include <QtGui/QPainter>

#include "qfilemenulabel.h"

QFileMenuLabel::QFileMenuLabel(QWidget* parent)
  : QClickTextLabel(parent)
{}

void QFileMenuLabel::paintEvent( QPaintEvent * event)
{
  // draw QLabel stuff
  QLabel::paintEvent(event);

  // draw custom stuff
  QPainter painter(this);

  // frame
  painter.setPen(QPen(QColor(237,237,230),1));

  painter.drawLine(QLine(0,0,width(),0));
  painter.drawLine(QLine(0,height()-1,width(),height()-1));
  painter.drawLine(QLine(0,0,0,height()));
  painter.drawLine(QLine(width()-1,0,width()-1,height()));

  // enable anti-aliasing
  painter.setRenderHint(QPainter::Antialiasing);

  painter.setPen(QPen(QColor(0,0,0),2));

  QLineF line_down(110.0, 13.0, 115.0, 18.0);
  QLineF line_up  (115.0, 18.0, 120.0, 13.0);

  // draw down-arrow
  painter.drawLine(line_down);
  painter.drawLine(line_up);
}

// Settings for Vim (http://www.vim.org/), please do not remove:
// vim:softtabstop=2:shiftwidth=2:expandtab:textwidth=80:cindent
// vim:fdm=expr:foldexpr=getline(v\:lnum)=~'/\\*\\*'&&getline(v\:lnum)!~'\\*\\*/'?'a1'\:getline(v\:lnum)=~'\\*\\*/'&&getline(v\:lnum)!~'/\\*\\*'?'s1'\:'='
