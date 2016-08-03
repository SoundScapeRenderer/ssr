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

#include "qvolumeslider.h"

#define UPPERLIMIT 12.0f
#define LOWERLIMIT -50.0f
#define ACCELERATION 0.2f

// margin in pixels
#define SIDEMARGIN 3
#define BOTTOMMARGIN 7


QVolumeSlider::QVolumeSlider(QWidget* parent)
  : QLabel(parent)
{
  setAlignment(Qt::AlignCenter);

  // set default values for level and volume
  update_displays(LOWERLIMIT, LOWERLIMIT);

  // clear text
  setText(QString());
}

void QVolumeSlider::update_displays(float lev, float vol)
{
  // save values
  lev = std::min(lev, UPPERLIMIT);
  lev = std::max(lev, LOWERLIMIT);
  level = lev;

  vol = std::min(vol, UPPERLIMIT);
  vol = std::max(vol, LOWERLIMIT);
  volume_dB = vol;

  update();
}

void QVolumeSlider::mousePressEvent(QMouseEvent *event)
{
  event->accept();

  // display volume as number
  setText(QString().setNum((int)(volume_dB + 0.5f)));

  // remember where movement started
  starting_point = event->globalPos();
}

void QVolumeSlider::mouseMoveEvent(QMouseEvent *event)
{
  event->accept();

  float vol_tmp = volume_dB + (event->globalX() - starting_point.x())*ACCELERATION;

  // limit possible values
  vol_tmp = std::min(vol_tmp, UPPERLIMIT);
  vol_tmp = std::max(vol_tmp, LOWERLIMIT);

  // display volume as number
  setText(QString().setNum((int)(vol_tmp + 0.5f)));

  emit signal_volume_changed(vol_tmp); // TODO: dB or linear??

  // store reference point for mouse motion
  starting_point = event->globalPos();
}

void QVolumeSlider::mouseReleaseEvent(QMouseEvent *event)
{
  event->accept();

  // clear text
  setText(QString());
}


void QVolumeSlider::paintEvent(QPaintEvent * event)
{
  event->accept();

  // draw QLabel stuff
  QLabel::paintEvent(event);

  QPainter painter(this);

  // draw white background for level meter
  painter.setPen(QPen(QColor(255,255,255)));
  painter.setBrush(QBrush(QColor(255,255,255)));

  painter.drawRect(SIDEMARGIN, 0, width() - 2*SIDEMARGIN, height() - BOTTOMMARGIN);

  // choose color for level bar
  if (level > 0.0f)
  {
    painter.setPen(QPen(QColor(255,0,0)));// red  // TODO: rgb
    painter.setBrush(QBrush(QColor(255,0,0)));

    // draw level bar
    painter.drawRect(SIDEMARGIN+1, 1, static_cast<int>((level-LOWERLIMIT)/(UPPERLIMIT-LOWERLIMIT)
				* (width()-2*SIDEMARGIN - 1)), height()-BOTTOMMARGIN-2);
  }
  else if (level <= -6.0f){
    painter.setPen(QPen(QColor(58,239,58)));
    painter.setBrush(QBrush(QColor(58,239,58))); // green

    // draw level bar
    painter.drawRect(SIDEMARGIN+1, 1, static_cast<int>((level-LOWERLIMIT)/(UPPERLIMIT-LOWERLIMIT)
			         * (width()-2*SIDEMARGIN - 1)), height()-BOTTOMMARGIN-2);
  }
  else // between -6 and 0 dB
  {
    painter.setPen(QPen(QColor(58,239,58)));
    painter.setBrush(QBrush(QColor(58,239,58))); // green
   
    // draw green part of level bar
    painter.drawRect(SIDEMARGIN+1, 1, static_cast<int>((-6.0f-LOWERLIMIT)/(UPPERLIMIT-LOWERLIMIT)
		     * (width()-2*SIDEMARGIN - 1)), height()-BOTTOMMARGIN-2);

    painter.setPen(QPen(QColor(255,255,0)));// yellow // TODO: rgb
    painter.setBrush(QBrush(QColor(255,255,0)));

    // draw yellow part of level bar
    painter.drawRect(static_cast<int>((-6.0f-LOWERLIMIT)/(UPPERLIMIT-LOWERLIMIT)
				      * (width()-2*SIDEMARGIN - 1)) + SIDEMARGIN + 2, 
		     1, 
		     static_cast<int>((level+6.0f)/(UPPERLIMIT-LOWERLIMIT)
				      * (width()-2*SIDEMARGIN - 1)), 
		     height()-BOTTOMMARGIN-2);
  }

  // draw volume mark
  const float vol_position = ((UPPERLIMIT-LOWERLIMIT) - (UPPERLIMIT - volume_dB))
    /(UPPERLIMIT-LOWERLIMIT) * (width()-2*SIDEMARGIN) + SIDEMARGIN;

  // define triangle
  const QPointF points[3] = {
    QPointF(vol_position - 4.0f, 29.0f),
    QPointF(vol_position + 4.0f, 29.0f),
    QPointF(vol_position       , 20.0f)
  };

  // draw frame
  painter.setPen(QPen(QColor(237,237,230),1));

  painter.drawLine(QLine(SIDEMARGIN,0,width()-SIDEMARGIN,0));
  painter.drawLine(QLine(SIDEMARGIN,height()-BOTTOMMARGIN,width()-SIDEMARGIN,height()-BOTTOMMARGIN));
  painter.drawLine(QLine(SIDEMARGIN,0,SIDEMARGIN,height()-BOTTOMMARGIN));
  painter.drawLine(QLine(width()-SIDEMARGIN,0,width()-SIDEMARGIN,height()-BOTTOMMARGIN));

  // indicate 0dB with the same color
  painter.drawLine(QLine(static_cast<int>((2-LOWERLIMIT)/(UPPERLIMIT-LOWERLIMIT)
                    * (width()-2*SIDEMARGIN - 1)),
		    1,
		    static_cast<int>((2-LOWERLIMIT)/(UPPERLIMIT-LOWERLIMIT)
                    * (width()-2*SIDEMARGIN - 1)),
                    height()-BOTTOMMARGIN));

  // enable anti-aliasing
  painter.setRenderHint(QPainter::Antialiasing);

  painter.setPen(QPen(QColor(0,0,0))); // black
  painter.setBrush(QBrush(QColor(0,0,0)));

  painter.drawPolygon(points, 3);

  // draw QLabel stuff (i.e. text) on top
  //QLabel::paintEvent(event);

  // this is a quick-hack to avoid error messages 
  //this->setText(this->text());
  painter.setPen(QPen(QColor(0, 0, 0)));
  painter.drawText(QRect(0, -3, width(), height()), Qt::AlignCenter, text());

}

// Settings for Vim (http://www.vim.org/), please do not remove:
// vim:softtabstop=2:shiftwidth=2:expandtab:textwidth=80:cindent
// vim:fdm=expr:foldexpr=getline(v\:lnum)=~'/\\*\\*'&&getline(v\:lnum)!~'\\*\\*/'?'a1'\:getline(v\:lnum)=~'\\*\\*/'&&getline(v\:lnum)!~'/\\*\\*'?'s1'\:'='
