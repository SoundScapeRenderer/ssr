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

#include "qtimeedit.h"
//#include "ssr_global.h"

QSSRTimeEdit::QSSRTimeEdit(QWidget* parent) : QLineEdit(parent)
{
  QString qt_style_sheet = "* { background-color: white; \n"
                               "border-radius: 0;       \n"
                               "border-width: 1px;      \n"
                               "border-color: rgb(237,237,230); }";

  this->setStyleSheet(qt_style_sheet);
}

QSSRTimeEdit::~QSSRTimeEdit(){}

void QSSRTimeEdit::keyPressEvent(QKeyEvent *event)
{
  QLineEdit::keyPressEvent(event);

  // to avoid propagating of RETURN to main widget
  event->accept();
}
