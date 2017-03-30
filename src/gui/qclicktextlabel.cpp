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

#include "qclicktextlabel.h"

QClickTextLabel::QClickTextLabel(QWidget* parent, int ID)
  : QLabel(parent), ID(ID)
{
  setTextFormat(Qt::RichText);
  setAlignment(Qt::AlignTop);
}

QClickTextLabel::QClickTextLabel( const QString& text, QWidget * parent)
  : QLabel(text, parent), ID(0)
{}

void QClickTextLabel::mousePressEvent(QMouseEvent *event)
{
  event->accept();

  if (ID)
    emit clicked(ID);
  else
    emit clicked();

  if (event->button() == Qt::RightButton)
    emit signal_right_clicked(event);
}

void QClickTextLabel::mouseDoubleClickEvent(QMouseEvent *event)
{
  emit signal_double_clicked();
  event->ignore();
}

// Settings for Vim (http://www.vim.org/), please do not remove:
// vim:softtabstop=2:shiftwidth=2:expandtab:textwidth=80:cindent
