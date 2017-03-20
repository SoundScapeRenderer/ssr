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
/// QSourceProperties

#ifndef SSR_QSOURCEPROPERTIES_H
#define SSR_QSOURCEPROPERTIES_H


#include <QtGui/QMouseEvent>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QFrame>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QRadioButton>

#include "qclicktextlabel.h"
#include "source.h"

/// QSourceProperties
class QSourceProperties : public QFrame
{
  Q_OBJECT

  public:
    QSourceProperties(QWidget* parent = 0);
    ~QSourceProperties();

    void update_displays(const Source& source, 
			 const DirectionalPoint& reference);

  private:
    QGridLayout*  _grid;
    QLineEdit*    _name_display;
    QLabel*       _coordinates_display;
    QLabel*       _distance_display;
    QLabel*       _azimuth_display;
    QCheckBox*    _position_fix_box;
    QCheckBox*    _doppler_box;
    QLabel*       _volume_display;
    QCheckBox*    _muted_check_box;
    QCheckBox*    _soloed_check_box;
    QRadioButton* _port_radio_button;
    QRadioButton* _file_radio_button;

    QLabel*      _source_model_label;
    QComboBox*   _source_model_display;
    QLabel*      _audio_source_label;
    QLineEdit*   _audio_source_display;
    QLabel*      _properties_label;
    QLineEdit*   _properties_display;
    QClickTextLabel* _create_source_button;
    QClickTextLabel* _close_button;

    QLabel* _create_text_label(const QString& text = QString());
   
    bool _create_new_source;

    virtual void mousePressEvent(QMouseEvent *event);
    virtual bool event(QEvent *e);

  private slots:
      void _set_source_mute(bool flag);
      void _set_source_solo(bool flag);
      void _set_source_position_fixed(bool flag);
      void _set_source_model(int index);
      void _set_doppler(bool flag);

      void _expand();
      void _collapse();

  public slots:
      void show(const bool create_new_source = false);

  signals:
    void signal_set_source_mute(bool);
    void signal_set_source_position_fixed(bool);
    void signal_set_source_model(int);
    void signal_set_source_solo(bool);

    //void signal_set_source_property( );
};

#endif

// Settings for Vim (http://www.vim.org/), please do not remove:
// vim:softtabstop=2:shiftwidth=2:expandtab:textwidth=80:cindent
// vim:fdm=expr:foldexpr=getline(v\:lnum)=~'/\\*\\*'&&getline(v\:lnum)!~'\\*\\*/'?'a1'\:getline(v\:lnum)=~'\\*\\*/'&&getline(v\:lnum)!~'/\\*\\*'?'s1'\:'='
