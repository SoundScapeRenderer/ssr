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

#include <QtGui/QPalette> 
#include <QtWidgets/QSizePolicy>

#include "qsourceproperties.h"

#include "ssr_global.h"
#include "apf/math.h"

using namespace apf::math;

//#define COLLAPSEDHEIGHT 200
//#define EXPANDEDHEIGHT 300
//#define WIDTH 200
//#define ROWHEIGHT 40

QSourceProperties::QSourceProperties(QWidget* parent)
#ifdef __APPLE__
: QFrame(parent, Qt::Window | Qt::CustomizeWindowHint),
#else
    : QFrame(parent),
#endif
    _create_new_source(false)
{
  this->setAutoFillBackground(true);
  this->setPalette(QPalette(QColor(255,255,255)));
  this->setFrameStyle( QFrame::Panel | QFrame::Raised );
  this->setLineWidth(2);

  QFont widget_font = this->font();
  widget_font.setPointSize(16);
  this->setFont(widget_font);

  // create grid to organize layout
  // TODO: better use QFormLayout?
  _grid = new QGridLayout(this);

  unsigned int current_row = 0;

  // name

  _grid->addWidget(_create_text_label("<font color=\"#888888\">name</font>"), 0, 0);

  _name_display = new QLineEdit();
  _name_display->setObjectName("item");
  _name_display->setAlignment(Qt::AlignRight);
  _name_display->setFrame(false);
  _name_display->setReadOnly(true);
  _name_display->setFocusPolicy(Qt::NoFocus);
  
  _grid->addWidget(_name_display, current_row, 1, 1, 1);

  current_row++;

  // coordinates
  
  _grid->addWidget(_create_text_label("<font color=\"#888888\">x, y</font>"), current_row, 0);

  _coordinates_display = new QLabel();
  _coordinates_display->setObjectName("item");
  _coordinates_display->setAlignment(Qt::AlignRight);

  _grid->addWidget(_coordinates_display, current_row, 1);
  _grid->addWidget(_create_text_label(" mtrs"), current_row, 2);

  current_row++;
  
  // distance

  _grid->addWidget(_create_text_label("<font color=\"#888888\">distance</font>"), current_row, 0);

  _distance_display = new QLabel();
  _distance_display->setObjectName("item");
  _distance_display->setAlignment(Qt::AlignRight);

  _grid->addWidget(_distance_display, current_row, 1);
  _grid->addWidget(_create_text_label(" mtrs"), current_row, 2);

  current_row++;

  // azimuth

  _grid->addWidget(_create_text_label("<font color=\"#888888\">azimuth</font>"), current_row, 0);

  _azimuth_display = new QLabel();
  _azimuth_display->setObjectName("item");
  _azimuth_display->setAlignment(Qt::AlignRight);

  _grid->addWidget(_azimuth_display, current_row, 1);
  _grid->addWidget(_create_text_label(" degs"), current_row, 2);
  
  current_row++;

  // position fix

  _grid->addWidget(_create_text_label("<font color=\"#888888\">fixed position</font>"), current_row, 0);

  _position_fix_box = new QCheckBox("");
  _position_fix_box->setFocusPolicy(Qt::NoFocus);
  connect(_position_fix_box, SIGNAL( toggled(bool) ), this, SLOT( _set_source_position_fixed(bool) ));

  _grid->addWidget(_position_fix_box, current_row, 2, Qt::AlignLeft);
 
  current_row++;

  // doppler effect enabled

  //_grid->addWidget(_create_text_label("<font color=\"#888888\">Doppler effect</font>"), current_row, 0);

  //_doppler_box = new QCheckBox("");
  //_doppler_box->setFocusPolicy(Qt::NoFocus);
  //connect(_doppler_box, SIGNAL( toggled(bool) ), this, SLOT( _set_doppler(bool) ));

  //_grid->addWidget(_doppler_box, current_row, 2, Qt::AlignLeft);
 
  //current_row++;

  // volume

  _grid->addWidget(_create_text_label("<font color=\"#888888\">volume</font>"), current_row, 0);

  _volume_display = new QLabel();
  _volume_display->setObjectName("item");
  _volume_display->setAlignment(Qt::AlignRight);

  _grid->addWidget(_volume_display, current_row, 1);
  _grid->addWidget(_create_text_label(" dB"), current_row, 2);
  
  current_row++;

  // mute state

  _grid->addWidget(_create_text_label("<font color=\"#888888\">muted</font>"), current_row, 0);

  _muted_check_box = new QCheckBox("");
  _muted_check_box->setFocusPolicy(Qt::NoFocus);
  connect(_muted_check_box, SIGNAL( toggled(bool) ), this, SLOT( _set_source_mute(bool) ));

  _grid->addWidget(_muted_check_box, current_row, 2, Qt::AlignLeft);
 
  current_row++;

  // solo state

  //_grid->addWidget(_create_text_label("<font color=\"#888888\">soloed</font>"), current_row, 0);

  //_soloed_check_box = new QCheckBox("");
  //_soloed_check_box->setFocusPolicy(Qt::NoFocus);
  //connect(_soloed_check_box, SIGNAL( toggled(bool) ), this, SLOT( _set_source_solo(bool) ));

  //_grid->addWidget(_soloed_check_box, current_row, 2, Qt::AlignLeft);
  
  //current_row++;

  // source model

  _source_model_label = _create_text_label("<font color=\"#888888\">model</font>");
  _grid->addWidget(_source_model_label, current_row, 0);

  _source_model_display = new QComboBox();
  _source_model_display->setObjectName("item");
  _source_model_display->setFrame(false);
  _source_model_display->insertItem(0, "plane wave");
  _source_model_display->insertItem(1, "point source");
  //_source_model_display->insertItem(2, "directional source");
  _grid->addWidget(_source_model_display, current_row, 1, 1, 2);
  connect(_source_model_display, SIGNAL(activated(int)), this, SLOT( _set_source_model(int)));
  current_row++;

  // audio source

  _audio_source_label = _create_text_label("<font color=\"#888888\">audio source</font>");
  _grid->addWidget(_audio_source_label, current_row, 0);

  _audio_source_display = new QLineEdit();
  _audio_source_display->setObjectName("item");
  _audio_source_display->setFrame(false);
  _audio_source_display->setAlignment(Qt::AlignRight);
  _audio_source_display->setReadOnly(true);
  
  _grid->addWidget(_audio_source_display, current_row, 1, 1, 2);

  current_row++;

  _port_radio_button = new QRadioButton("port");
  _port_radio_button->setFocusPolicy(Qt::NoFocus);

  _file_radio_button = new QRadioButton("file");
  _file_radio_button->setFocusPolicy(Qt::NoFocus);

  //_grid->addWidget(_port_radio_button, current_row, 1, 1, 1);
  //_grid->addWidget(_file_radio_button, current_row, 2, 1, 1);

  //current_row++;

  // properties file

  _properties_label = _create_text_label("<font color=\"#888888\">properties</font>");
  _grid->addWidget(_properties_label, current_row, 0);

  _properties_display = new QLineEdit();
  _properties_display->setObjectName("item");
  _properties_display->setFrame(false);
  _properties_display->setAlignment(Qt::AlignRight);
  _properties_display->setReadOnly(true);

  _grid->addWidget(_properties_display, current_row, 1, 1, 2);

  current_row++;


  // close button

  _close_button = new QClickTextLabel("Close");
  _close_button->setObjectName("item");
  connect(_close_button, SIGNAL( clicked() ), this, SLOT( hide() ));
  _grid->addWidget(_close_button, current_row, 0);


  // create source button

  _create_source_button = new QClickTextLabel("Create source");
  _create_source_button->setObjectName("item");
  //connect(_less_button, SIGNAL( clicked() ), this, SLOT( _collapse() ));
  //_grid->addWidget(_create_source_button, current_row, 1, 1, 2);
 
  current_row++;



  setLayout(_grid);

  // size
  _grid->setColumnMinimumWidth (0, 80);
  _grid->setColumnMinimumWidth (1, 80);
  _grid->setColumnMinimumWidth (2, 40);

  for (int n = 0; n < _grid->rowCount(); n++)
  {
    _grid->setRowMinimumHeight (n, 20);
  }

  _grid->setSpacing(3);
  _grid->setSizeConstraint(QLayout::SetNoConstraint);

  // _collapse();
  _expand();
  
}

QSourceProperties::~QSourceProperties()
{
  // TODO: Delete, delete, delete
}

QLabel* QSourceProperties::_create_text_label(const QString& text)
{
  QLabel* buffer = new QLabel(text);

  buffer->setObjectName("item");
  buffer->setTextFormat(Qt::RichText);
  buffer->setAlignment(Qt::AlignLeft);

  return buffer;
}

void QSourceProperties::update_displays(const Source& source, 
					const DirectionalPoint& reference)
{
  // do not update display if dialog is used to create a new sound source
  if ( _create_new_source ) return;
 
  if (!_name_display->hasFocus())
     _name_display->setText( QString::fromStdString( source.name.c_str() ) );

  if (!_audio_source_display->hasFocus())
     _audio_source_display->setText( 
                        QString::fromStdString( source.port_name.c_str() ) );

  _coordinates_display->setText(QString().setNum(source.position.x,'f',2) + 
				", " + QString().setNum(source.position.y,'f',2));

  // calculate distance between reference point and source
  const float dist = sqrt(square(source.position.x - reference.position.x) +
			  square(source.position.y - reference.position.y));
  
  _distance_display->setText(QString().setNum(dist,'f',2));

  // calculate angle from which the source is seen
  float ang = rad2deg(angle(source.position, reference.orientation));

  // confine angle to interval ]-180, 180] TODO: Make it more elegant
  ang = std::fmod(ang, 360.0f);
  if (ang > 180.0f) ang -= 360.0f;
  else if (ang <= -180.0f) ang += 360.0f;
  
  _azimuth_display->setText(QString().setNum(ang,'f',2));
  _position_fix_box->setChecked(source.fixed_position);
  // set source model
  switch(source.model){
    case Source::point:
      _source_model_display->setCurrentIndex(1);
      break;
    case Source::plane:
      _source_model_display->setCurrentIndex(0);
      break;
    default:
      break;
  }
  //_doppler_box->setChecked(source.doppler_effect);
  _volume_display->setText(QString().setNum(linear2dB(source.gain),'f',1));
  _muted_check_box->setChecked(source.mute);

  if (source.properties_file.empty())
  {
    _properties_display->setText("No file specified.");
  }
  else
  {
    _properties_display->setText(source.properties_file.c_str());
  }

}

void QSourceProperties::_expand()
{
  _audio_source_label->show();
  _audio_source_display->show();
  _source_model_label->show();
  _source_model_display->show();
  _properties_label->show();
  _properties_display->show();
  _close_button->show();
  //_create_source_button->show();

  _grid->setSizeConstraint(QLayout::SetMinimumSize);
}

void QSourceProperties::_collapse()
{
  //  _more_button->show();
  _audio_source_label->hide();
  _audio_source_display->hide();
  _source_model_label->hide();
  _source_model_display->hide();
  _properties_label->hide();
  _properties_display->hide();
  //  _less_button->hide();

  _grid->setSizeConstraint(QLayout::SetMinimumSize);
}

void QSourceProperties::mousePressEvent(QMouseEvent *event)
{
  event->accept();
}

void QSourceProperties::show(const bool create_new_source)
{
  _create_new_source = create_new_source;
  QFrame::show();
}



void QSourceProperties::_set_source_mute(bool flag)
{
  emit signal_set_source_mute(flag);
}

void QSourceProperties::_set_source_solo(bool flag)
{
  emit signal_set_source_solo(flag);
}

void QSourceProperties::_set_source_position_fixed(bool flag)
{
  emit signal_set_source_position_fixed(flag);
}

void QSourceProperties::_set_source_model(int index)
{
  emit signal_set_source_model(index);
}

void QSourceProperties::_set_doppler(bool flag)
{
  (void)flag;
  //emit signal_set_source_property( );
}

bool QSourceProperties::event(QEvent *e)
{
  // catch mouse events
  if (e->type() == QEvent::MouseButtonDblClick ||
      e->type() == QEvent::MouseButtonPress ||
      e->type() == QEvent::MouseButtonRelease ||
      e->type() == QEvent::MouseMove)
  {
    e->accept();
    return true;
  }
  else return false;

}
// Settings for Vim (http://www.vim.org/), please do not remove:
// vim:softtabstop=2:shiftwidth=2:expandtab:textwidth=80:cindent
// vim:fdm=expr:foldexpr=getline(v\:lnum)=~'/\\*\\*'&&getline(v\:lnum)!~'\\*\\*/'?'a1'\:getline(v\:lnum)=~'\\*\\*/'&&getline(v\:lnum)!~'/\\*\\*'?'s1'\:'='
