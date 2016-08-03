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
#include <cmath>

#include <QtCore/QTimer>
#include <QtGui/QPainter>

#include "qssrtimeline.h"
#include "ssr_global.h"

#define ACCELERATION 0.2f
#define TIMESPAN 120

// margin in pixels
#define TOPMARGIN 5
#define BOTTOMMARGIN 12
#define LEFTMARGIN 20
#define RIGHTMARGIN 20

QSSRTimeLine::QSSRTimeLine(QWidget* parent, unsigned int update_interval)
  : QLabel(parent), _progress(0.0f), _lower_time_boundary(0u),
    _mouse_pos_at_click(0), _progress_at_mouse_click(0) //, _previous_time("")
{
  (void)update_interval;

   _time_edit = new  QTimeEdit(this);
   connect(_time_edit, SIGNAL(returnPressed()), this, SLOT(_interpret_time_edit()));
   _time_edit->setVisible(false);

  // update widget every update_interval msec
  //QTimer *timer = new QTimer(this);
  //connect(timer, SIGNAL(timeout()), this, SLOT(update()));
  //timer->start(update_interval);
}

void QSSRTimeLine::set_progress(float prog)
{
  _progress = prog;

  // limit possible values
  _progress = std::max(_progress, 0.0f);

  _lower_time_boundary = static_cast<unsigned int>(_progress/TIMESPAN) * TIMESPAN;

  update();

}

void QSSRTimeLine::mousePressEvent(QMouseEvent *event)
{
  event->accept();

  _time_edit->setVisible(false);

  _mouse_pos_at_click = event->x();
  _progress_at_mouse_click = _progress;

  if (event->buttons() == Qt::LeftButton && event->x()-LEFTMARGIN-1 <= 0)
  {
    //emit signal_transport_locate(0.0f);
    emit signal_transport_locate(_progress - 5.0f);
  }
  else if (event->buttons() == Qt::LeftButton && event->x() <= width()-RIGHTMARGIN)
  {
    emit signal_transport_locate(static_cast<float>(event->x()-LEFTMARGIN-1)/
				 static_cast<float>(width()-(LEFTMARGIN+RIGHTMARGIN))*
                                 TIMESPAN + _lower_time_boundary);
  }
  else if (event->buttons() == Qt::LeftButton && event->x() > width()-RIGHTMARGIN)
  {
    emit signal_transport_locate(_progress + 5.0f);
  }
  else if (event->buttons() == Qt::RightButton)
  { 
    show_time_edit();
  }

}

void QSSRTimeLine::mouseMoveEvent(QMouseEvent *event)
{
  event->accept();

  float current_progress = static_cast<float>(event->x() - _mouse_pos_at_click)/
    static_cast<float>(width()-(LEFTMARGIN+RIGHTMARGIN))*
    TIMESPAN + _progress_at_mouse_click;

  if (current_progress > 0)
  {
    emit signal_transport_locate(current_progress);
  }
  else emit signal_transport_locate(0.0f);
 
}

void QSSRTimeLine::mouseDoubleClickEvent(QMouseEvent *event)
{
  event->accept();

  if (event->x()-LEFTMARGIN-1 > 0 && event->x() < width()-RIGHTMARGIN)
  {
    emit signal_transport_locate(0.0f);
  }

}

void QSSRTimeLine::mouseReleaseEvent(QMouseEvent *event)
{
  event->accept();
}

void QSSRTimeLine::paintEvent( QPaintEvent * event)
{
  event->accept();

  QPainter painter(this);

  // draw white background for level meter
  painter.setPen(QPen(QColor(255,255,255)));
  painter.setBrush(QBrush(QColor(255,255,255)));

  painter.drawRect(LEFTMARGIN, TOPMARGIN+1,
                   width() - (LEFTMARGIN+RIGHTMARGIN),
                   height()-(BOTTOMMARGIN+TOPMARGIN+2));

  // draw frame
  painter.setPen(QPen(QColor(237,237,230),1));

  painter.drawLine(QLine(LEFTMARGIN,TOPMARGIN,
                         width()-RIGHTMARGIN,TOPMARGIN));
  painter.drawLine(QLine(LEFTMARGIN,height()-BOTTOMMARGIN,
                         width()-RIGHTMARGIN,height()-BOTTOMMARGIN));
  painter.drawLine(QLine(LEFTMARGIN,TOPMARGIN,LEFTMARGIN,
                         height()-BOTTOMMARGIN));
  painter.drawLine(QLine(width()-RIGHTMARGIN,TOPMARGIN,
                         width()-RIGHTMARGIN,height()-BOTTOMMARGIN));

  // draw progress bar
  /*
  QLinearGradient gradient(0, 0, (int)(width()*0.7f), 0);
  gradient.setColorAt(0.7, QColor(43,174,247));
  gradient.setColorAt(0, Qt::blue);
  painter.setBrush(gradient);
  */

  // enable anti-aliasing
  painter.setRenderHint(QPainter::Antialiasing);

  //  painter.setPen(QPen(QColor(43,174,247)));
  painter.setBrush(QBrush(QColor(43,174,247)));

  const float handle_position = std::min(_progress - _lower_time_boundary, static_cast<float>(TIMESPAN))/TIMESPAN *
    (width()-(LEFTMARGIN+RIGHTMARGIN)) + LEFTMARGIN + 1;

  // draw progress bar
  painter.drawRect(LEFTMARGIN+1, TOPMARGIN+1,
                   static_cast<int>(handle_position - LEFTMARGIN)-1,
                   height()-(TOPMARGIN+BOTTOMMARGIN+1));

  painter.setPen(QPen(QColor(0,0,0))); // black
  painter.setBrush(QBrush(QColor(0,0,0))); // black

  // define vertical bar of handle
  const QPointF points_quad[4] = {
    QPointF(handle_position - 0.5f, TOPMARGIN),
    QPointF(handle_position - 0.5f, static_cast<float>(height()-(BOTTOMMARGIN+1))),
    QPointF(handle_position + 0.5f, static_cast<float>(height()-(BOTTOMMARGIN+1))),
    QPointF(handle_position + 0.5f, TOPMARGIN)
  };

  painter.drawPolygon(points_quad, 4);

  // define triangle for handle
  const QPointF points_tri[3] = {
    QPointF(handle_position - 3.0f, 0.0f),
    QPointF(handle_position + 3.0f, 0.0f),
    QPointF(handle_position, TOPMARGIN + 1.0f)
  };

  painter.drawPolygon(points_tri, 3);

  int hours = static_cast<int>(_progress/3600);
  int min   = static_cast<int>((_progress - hours*3600)/60);
  int sec   = static_cast<int>(fmod(_progress,60));

  // set seconds
  QString time = QString().setNum(sec);

  if (sec < 10) time.prepend("0"); 

  // set minutes
  time.prepend(QString().setNum(min) + ":");

  if (hours)
  {
    if (min < 10) time.prepend("0");

    // set hours
    time.prepend(QString().setNum(hours) + ":");
  }

 // enable anti-aliasing
  painter.setRenderHint(QPainter::TextAntialiasing);
  
  painter.drawText(QPointF(handle_position - 7.0f, 35.0f), time);
}

void QSSRTimeLine::_interpret_time_edit()
{
  float new_time = 0.0f;
  bool conversion_ok = false;

  QString text = _time_edit->text();

  if ((text.size() > 3) && (QString(text.at(text.size() - 3)) == ":"))
  {
    // seconds
    new_time = text.section(':',-1).toFloat(&conversion_ok);

    if (conversion_ok)
    {
      // minutes
      new_time += text.section(':',-2,-2).toFloat(&conversion_ok) * 60;
    }

    if (conversion_ok && (text.size() > 6) && 
	(QString(text.at(text.size() - 6)) == ":"))
    {
      // hours
      new_time += text.section(':',0,-3).toFloat(&conversion_ok) * 3600;
    }
  }    
  else 
  {
    // interpret all as seconds
    new_time = text.toFloat(&conversion_ok);
  }

  if (conversion_ok) signal_transport_locate(new_time);

  hide_time_edit();
}


void QSSRTimeLine::show_time_edit()
{
  _time_edit->setGeometry(width()/2 - 20, 0, 40, height());
  _time_edit->clear();
  _time_edit->setVisible(true);
  _time_edit->setFocus();
}

void QSSRTimeLine::hide_time_edit()
{
  _time_edit->clearFocus();
  _time_edit->setVisible(false);
}

// Settings for Vim (http://www.vim.org/), please do not remove:
// vim:softtabstop=2:shiftwidth=2:expandtab:textwidth=80:cindent
// vim:fdm=expr:foldexpr=getline(v\:lnum)=~'/\\*\\*'&&getline(v\:lnum)!~'\\*\\*/'?'a1'\:getline(v\:lnum)=~'\\*\\*/'&&getline(v\:lnum)!~'/\\*\\*'?'s1'\:'='
