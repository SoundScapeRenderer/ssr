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

#include <QtCore/QTimer>
#include <QtGui/QPainter>

#include "qcpulabel.h"

QCPULabel::QCPULabel(QWidget* parent, unsigned int update_interval)
  : QLabel(parent), load(0.0f)
{
  setAlignment(Qt::AlignCenter);

  // update widget every update_interval msec
  QTimer *timer = new QTimer(this);
  connect(timer, SIGNAL(timeout()), this, SLOT(update()));
  timer->start(update_interval);
}

void QCPULabel::set_load(float l)
{
  load = l;

  // limit possible values
  load = std::min(load, 100.0f);
  load = std::max(load, 0.0f);

  //  update();
}

void QCPULabel::paintEvent(QPaintEvent * event)
{
  event->accept();

  // draw QLabel stuff
  QLabel::paintEvent(event);

  QPainter painter(this);

  // frame
  painter.setPen(QPen(QColor(237,237,230),1));

  painter.drawLine(QLine(0,0,width(),0));
  painter.drawLine(QLine(0,height()-1,width(),height()-1));
  painter.drawLine(QLine(0,0,0,height()));
  painter.drawLine(QLine(width()-1,0,width()-1,height()-1));

  // choose colors
  if (load <= 60.0f){
    painter.setPen(QPen(QColor(58,239,58)));
    painter.setBrush(QBrush(QColor(58,239,58))); // green
  }
  else if (load > 80.0f){
    painter.setPen(QPen(QColor(255,0,0)));
    painter.setBrush(QBrush(QColor(255,0,0))); // red // TODO: rgb
  }
  else {
    painter.setPen(QPen(QColor(255,255,0)));
    painter.setBrush(QBrush(QColor(255,255,0))); // yellow // TODO: rgb
  }

  // draw load bar
  painter.drawRect(1, 1, (int)(width()*load/100.0f)-1, height()-3);

  // set text
  //setText(QString().setNum((int)(load + 0.5f)));

  // quick-hack
  painter.setPen(QPen(QColor(0, 0, 0))); 
  painter.drawText(QRect(0, 1, width(), height()), Qt::AlignCenter, QString().setNum((int)(load + 0.5f)));

}

// Settings for Vim (http://www.vim.org/), please do not remove:
// vim:softtabstop=2:shiftwidth=2:expandtab:textwidth=80:cindent
// vim:fdm=expr:foldexpr=getline(v\:lnum)=~'/\\*\\*'&&getline(v\:lnum)!~'\\*\\*/'?'a1'\:getline(v\:lnum)=~'\\*\\*/'&&getline(v\:lnum)!~'/\\*\\*'?'s1'\:'='
