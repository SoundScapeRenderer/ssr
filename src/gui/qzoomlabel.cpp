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
#include <algorithm>

#include <QtGui/QPainter>

#include "qzoomlabel.h"

#define ZOOMACCELERATIONDOWNWARD 0.002f
#define ZOOMACCELERATIONUPWARD 0.03f

QZoomLabel::QZoomLabel(QWidget* parent)
  : QLabel(parent), zoom(100), zoom_buffer(100)
{
  setAlignment(Qt::AlignCenter);

  // display number
  setText(QString().setNum(zoom_buffer));

  // notify others // TODO: I think nobody receives it...
  emit signal_zoom_changed(zoom_buffer);
}

void QZoomLabel::mousePressEvent(QMouseEvent *event)
{
  event->accept();

  // remember where movement started
  starting_point = event->globalPos();
}

void QZoomLabel::mouseMoveEvent(QMouseEvent *event)
{
  event->accept();

  float acceleration;

  if (event->globalY() - starting_point.y() > 0)
    // mouse moves downward
    acceleration = ZOOMACCELERATIONDOWNWARD;
  else
    // mouse moves upward
    acceleration = ZOOMACCELERATIONUPWARD;

  // set zoom buffer according to distance
  set_zoom_buffer((int)(zoom - zoom * (event->globalY() - starting_point.y())
        * acceleration));
}

void QZoomLabel::mouseReleaseEvent(QMouseEvent *event)
{
  event->accept();

  // set zoom
  zoom = zoom_buffer;
}

void QZoomLabel::mouseDoubleClickEvent(QMouseEvent *event)
{
  event->accept();

  // Return to 100%
  // TODO: Make it more elegant
  set_zoom_buffer(100);
  zoom = 100;

  emit signal_zoom_changed(zoom_buffer);
}

void QZoomLabel::set_zoom_buffer(int z)
{
  // limit values
  z = std::min(z, 300);
  z = std::max(z, 10);

  zoom_buffer = z;

  // display number
  setText(QString().setNum(zoom_buffer));

  // tell the others
  emit signal_zoom_changed(zoom_buffer);
}

void QZoomLabel::update_display(int z)
{
  // avoid infinite loop
  if (zoom_buffer == z)
    return;

  zoom = z;

  setText(QString().setNum(zoom));
}

void QZoomLabel::paintEvent( QPaintEvent * event)
{
  QLabel::paintEvent(event);

  QPainter painter(this);

  // frame
  painter.setPen(QPen(QColor(237,237,230),1));

  painter.drawLine(QLine(0,0,width(),0));
  painter.drawLine(QLine(0,height()-1,width(),height()-1));
  painter.drawLine(QLine(0,0,0,height()));
  painter.drawLine(QLine(width()-1,0,width()-1,height()));

  // enable anti-aliasing
  painter.setRenderHint(QPainter::Antialiasing);

  painter.setPen(QPen(QColor(0,0,0),1)); // black

  QLineF line_down_1(40.0f, 9.0f, 44.0f, 12.0f);
  QLineF line_up_1  (44.0f, 12.0f, 48.0f, 9.0f);

  QLineF line_down_2(40.0f, 6.0f, 44.0f, 3.0f);
  QLineF line_up_2  (44.0f, 3.0f, 48.0f, 6.0f);

  // draw down-arrow
  painter.drawLine(line_down_1);
  painter.drawLine(line_up_1);

  // draw up-arrow
  painter.drawLine(line_down_2);
  painter.drawLine(line_up_2);
}

// Settings for Vim (http://www.vim.org/), please do not remove:
// vim:softtabstop=2:shiftwidth=2:expandtab:textwidth=80:cindent
// vim:fdm=expr:foldexpr=getline(v\:lnum)=~'/\\*\\*'&&getline(v\:lnum)!~'\\*\\*/'?'a1'\:getline(v\:lnum)=~'\\*\\*/'&&getline(v\:lnum)!~'/\\*\\*'?'s1'\:'='
