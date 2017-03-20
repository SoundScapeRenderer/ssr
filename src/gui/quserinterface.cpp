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
#include <string>
#include <fstream>
#include <cmath>

#include <QtCore/QTimer>
#include <QtGui/QMouseEvent>
#include <QtGui/QResizeEvent>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDesktopWidget>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QMenu>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QVBoxLayout>

#include "quserinterface.h"
#include "apf/math.h"

using apf::math::dB2linear;
using apf::math::linear2dB;

#include "apf/stringtools.h"
#include "posixpathtools.h"

#define FILEMENUWIDTH 128
#define BETWEENBUTTONSPACE 6
#define BETWEENLABELSPACE 20
#define SCENEBUTTONWIDTH 102
#define BETWEENSCENEBUTTONSPACE 18

// define frame geometry in pixels
#define DEFAULTFRAMETOP 90
#define DEFAULTFRAMEBOTTOM 30
#define DEFAULTFRAMELEFT 30
#define DEFAULTFRAMERIGHT 30

// define button size in pixels
#define BUTTONWIDTH 55
#define BUTTONHEIGHT 31

#define UPDATEINTERVALFORQTWIDGETS 300u

// in dB
#define MAXVOLUME 12.0f
// given by qvolumeslider
#define MINVOLUME -50.0f

/** ctor.
 * @param controller this can be either the Controller or, if the IP interface
 * is used, an IPClient.
 * @param scene reference to the MasterScene or to a client-side copy of the
 * Scene.
 * @param parent parent Qt widget. If left as \a NULL (default) then the window
 * is startet as an independent main window.
 **/
ssr::QUserInterface::QUserInterface(Publisher& controller, const Scene& scene
        , const std::string& path_to_gui_images
        , const std::string& path_to_scene_menu
        , unsigned int update_frequency
        , QWidget *parent)
  : QOpenGLPlotter(controller, scene, path_to_gui_images, parent),
    _active_scene(0),
    _ignore_mouse_events(false),
    _controlsParent(this)
{
  // set window title
  std::string type = _controller.get_renderer_name();
  if      (type == "wfs")        setWindowTitle("SSR - WFS");
  else if (type == "binaural")   setWindowTitle("SSR - Binaural");
  else if (type == "brs")        setWindowTitle("SSR - BRS");
  else if (type == "vbap")       setWindowTitle("SSR - VBAP");
  else if (type == "aap")        setWindowTitle("SSR - AAP");
  else if (type == "generic")    setWindowTitle("SSR - Generic Renderer");
  else if (type == "ambisonics") setWindowTitle("SSR - Ambisonics");
  else if (type == "") setWindowTitle("SSR");
  else setWindowTitle(type.c_str());

  setWindowIcon(QIcon("images/ssr_logo.png"));

  // make sure that buttons look good
  setMinimumSize(580, 215);

  // setWindowState( Qt::WindowFullScreen );
  // setWindowState( Qt::WindowMaximized );

  // set default size
  setGeometry(200, 100, 900, 800);
  
#ifdef ENABLE_FLOATING_CONTROL_PANEL
  // TODO: use screen size for initial window positions
  //QRect screenSize = QApplication::desktop()->screenGeometry();
  setGeometry(200, 70, 900, 700);
  _controlsParent = new QLabel(this, Qt::Window | Qt::CustomizeWindowHint | Qt::WindowTitleHint);
  _controlsParent->setFixedSize(900, 75);
  _controlsParent->move(200, 780);
  _controlsParent->setWindowTitle("Controls");
  _controlsParent->installEventFilter(this);
  _controlsParent->show();
#else
  // build up frame around OpenGL window
  _frame = new QGUIFrame(this);
#endif

  _source_properties = new QSourceProperties(this);
  connect(_source_properties, SIGNAL(signal_set_source_mute(bool)), this, SLOT(_set_source_mute(bool)));
  connect(_source_properties, SIGNAL(signal_set_source_position_fixed(bool)), this, SLOT(_set_source_position_fixed(bool)));
  connect(_source_properties, SIGNAL(signal_set_source_model(int)), this, SLOT(_set_source_model(int)));
  _source_properties->hide();
  
  // set window icon
  QString path_to_image( _path_to_gui_images.c_str() ) ;
  setWindowIcon(QIcon(QPixmap(path_to_image.append("/ssr_logo_large.png"))));

  // build up buttons etc.
  _file_menu_label = new QFileMenuLabel(_controlsParent);
  //  _file_menu_label->setStyleSheet("border-width: 1px; border-color: black; border-style: solid");
  _file_menu_label->setIndent(5);
  _file_menu_label->setText("File");
  _file_menu_label->show();

  connect(_file_menu_label, SIGNAL(clicked()), this, SLOT(_show_file_menu()));

  _processing_button = new QPushButton(_controlsParent);
  _processing_button->setObjectName("_processing_button");
  _processing_button->setText("on/off");
  _processing_button->setCheckable(true);
  _processing_button->setFocusPolicy(Qt::NoFocus);
  _processing_button->show();

  connect(_processing_button, SIGNAL(clicked()), this, SLOT(_processing_button_pressed()));

  _skip_back_button = new QPushButton(_controlsParent);
  _skip_back_button->setObjectName("_skip_back_button");
  _skip_back_button->setFocusPolicy(Qt::NoFocus);
  _skip_back_button->show();

  connect(_skip_back_button, SIGNAL(clicked()), this, SLOT(_skip_back()));

  _pause_button = new QPushButton(_controlsParent);
  _pause_button->setObjectName("_pause_button");
  _pause_button->setCheckable(true);
  _pause_button->setFocusPolicy(Qt::NoFocus);
  _pause_button->show();

  connect(_pause_button, SIGNAL(clicked()), this, SLOT(_pause_button_pressed()));

  _play_button = new QPushButton(_controlsParent);
  _play_button->setObjectName("_play_button");
  _play_button->setCheckable(true);
  _play_button->setFocusPolicy(Qt::NoFocus);
  _play_button->show();

  connect(_play_button, SIGNAL(clicked()), this, SLOT(_play_button_pressed()));

  _time_line = new QSSRTimeLine(_controlsParent, UPDATEINTERVALFORQTWIDGETS);
  _time_line->show();
  connect(_time_line, SIGNAL(signal_transport_locate(float)), this, SLOT(_transport_locate(float)));

  _cpu_label = new QCPULabel(_controlsParent, UPDATEINTERVALFORQTWIDGETS);
  _cpu_label->show();

  _cpu_label_text_tag = new QLabel(_controlsParent);
  _cpu_label_text_tag->setAlignment(Qt::AlignCenter);
  _cpu_label_text_tag->setText("cpu");

  _zoom_label = new QZoomLabel(_controlsParent);
  _zoom_label->show();

  connect(_zoom_label, SIGNAL(signal_zoom_changed(int)), this, SLOT(_set_zoom(int)));
  connect(this, SIGNAL(signal_zoom_set(int)), _zoom_label, SLOT(update_display(int)));

  _zoom_label_text_tag = new QLabel(_controlsParent);
  _zoom_label_text_tag->setAlignment(Qt::AlignCenter);
  _zoom_label_text_tag->setText("zoom");
  _zoom_label_text_tag->show();

  _volume_slider = new QVolumeSlider(_controlsParent);
  _volume_slider->show();

  connect(_volume_slider, SIGNAL(signal_volume_changed(float)), this, SLOT(_set_master_volume(float)));

  _volume_slider_text_tag = new QLabel(_controlsParent);
  _volume_slider_text_tag->setAlignment(Qt::AlignCenter);
  _volume_slider_text_tag->setText("level");
  _volume_slider_text_tag->show();

  //  open_file_label = new QClickTextLabel( this );
  //  open_file_label->setGeometry( QRect( XREF, YREF, 270, 39 ) );
  //  open_file_label->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)1, (QSizePolicy::SizeType)1, 0, 0, open_file_label->sizePolicy().hasHeightForWidth() ) );

  // put labels into correct position
  _file_menu_label->setGeometry(DEFAULTFRAMELEFT, 24, FILEMENUWIDTH, 24);
  _processing_button->setGeometry(DEFAULTFRAMELEFT+FILEMENUWIDTH+
                                 BETWEENBUTTONSPACE, 20,
                                 BUTTONWIDTH, BUTTONHEIGHT);
  _skip_back_button->setGeometry(DEFAULTFRAMELEFT+FILEMENUWIDTH+
                                BETWEENBUTTONSPACE+BUTTONWIDTH+
                                BETWEENBUTTONSPACE, 20,
                                BUTTONWIDTH, BUTTONHEIGHT);
  _pause_button->setGeometry(DEFAULTFRAMELEFT+FILEMENUWIDTH+
                            2*(BETWEENBUTTONSPACE+BUTTONWIDTH)+
                            BETWEENBUTTONSPACE, 20,
                            BUTTONWIDTH, BUTTONHEIGHT);
  _play_button->setGeometry(DEFAULTFRAMELEFT+FILEMENUWIDTH+
                           3*(BETWEENBUTTONSPACE+BUTTONWIDTH)+
                           BETWEENBUTTONSPACE, 20,
                           BUTTONWIDTH, BUTTONHEIGHT);

  // functional inits
  _deselect_all_sources(); // no source selected

#ifdef ENABLE_FLOATING_CONTROL_PANEL
  // scene menu is not shown if floating control panel is used
  VERBOSE("Floating control panel is used, scene menu will not be shown.");
  (void)path_to_scene_menu;
#else
  // create the scene menu if an according config file is present
  _create_scene_menu(path_to_scene_menu);
#endif

  // update screen with update_frequency
  QTimer *timer = new QTimer(this);
  connect(timer, SIGNAL(timeout()), this, SLOT(_update_screen()));
  timer->start(static_cast<unsigned int>(1.0f/static_cast<float>(update_frequency) * 1000.0f));

#ifdef ENABLE_FLOATING_CONTROL_PANEL
  _resizeControls(_controlsParent->width());
#endif
}

/// Dtor.
ssr::QUserInterface::~QUserInterface()
{
  // TODO: DELETE, DELETE, DELETE Widgets

  // clear memory if scene button list has been in use
  if (!_scene_button_list.empty())
  {
    for (scene_button_list_t::iterator i = _scene_button_list.begin(); 
	 i != _scene_button_list.end(); i++)
    {
      delete *i;
    }
  }
  _scene_button_list.clear();

}

/// Skips back to the beginning of the scene
void ssr::QUserInterface::_skip_back()
{
  _controller.transport_locate(0);
}

/** Skips the scene to a specified instant of time
 * @ param time instant of time in sec to locate
 */
void ssr::QUserInterface::_transport_locate(float time)
{
  if (time >= 0.0f)
  {
    _controller.transport_locate(time);
  }
  else 
  {
    _skip_back();
  }

  // update time line
  _time_line->set_progress(static_cast<float>(_scene.get_transport_position())/
			   static_cast<float>(_scene.get_sample_rate()));
}

void ssr::QUserInterface::_solo_selected_sources()
{
  _soloed_sources.clear();

  for (selected_sources_map_t::iterator i = _selected_sources_map.begin(); 
       i != _selected_sources_map.end(); i++)
  {
    _soloed_sources.insert(i->second);

    // make sure it's not muted
    _controller.set_source_mute(i->second, false);
   
  } // for

  // get sources
  source_buffer_list_t source_buffer_list;
  _scene.get_sources(source_buffer_list);

  // mute other sources
  for (source_buffer_list_t::const_iterator i = source_buffer_list.begin(); 
       i != source_buffer_list.end(); i++)
  {
    // if source is not soloed
    if (_soloed_sources.find(i->id) == _soloed_sources.end())
    {
      // then mute it
      _controller.set_source_mute(i->id, true);
    }
  }

}


/// this function is not so useful
void ssr::QUserInterface::_unsolo_selected_sources()
{
  for (selected_sources_map_t::iterator i = _selected_sources_map.begin(); 
       i != _selected_sources_map.end(); i++)
  {
    _soloed_sources.erase(i->second);
  } // for

  // if no source is soloed anymore
  if (_soloed_sources.empty())
  {
    _unsolo_all_sources();
  }
  // if other sources are soloed
  else
  {
    for (selected_sources_map_t::iterator i = _selected_sources_map.begin(); 
	 i != _selected_sources_map.end(); i++)
    {
      // then mute the unsoloed sources
      _controller.set_source_mute(i->second, true); 
    } // for
  }

}

void ssr::QUserInterface::_toggle_solo_state_of_selected_sources()
{
  for (selected_sources_map_t::iterator i = _selected_sources_map.begin(); 
       i != _selected_sources_map.end(); i++)
  {
    if (_soloed_sources.find(i->second) == _soloed_sources.end())
    {
      // solo source
      _soloed_sources.insert(i->second); 
    }
    else 
    {
      // unsolo source
      _soloed_sources.erase(i->second);
    }
  } // for

  // get sources
  source_buffer_list_t source_buffer_list;
  _scene.get_sources(source_buffer_list);

  for (source_buffer_list_t::const_iterator i = source_buffer_list.begin(); 
       i != source_buffer_list.end(); i++)
  {
    if (_soloed_sources.find(i->id) == _soloed_sources.end())
    {
      // mute 
      _controller.set_source_mute(i->id, true);
    }
    else 
    {
      // unmute
      _controller.set_source_mute(i->id, false);
    }
  }
}

void ssr::QUserInterface::_unsolo_all_sources()
{
  _soloed_sources.clear();

  // get sources
  source_buffer_list_t source_buffer_list;
  _scene.get_sources(source_buffer_list);

  for (source_buffer_list_t::const_iterator i = source_buffer_list.begin(); 
       i != source_buffer_list.end(); i++)
  {
    _controller.set_source_mute(i->id, false);
  }
}

/// This slot is called when the \a processing \a button was clicked by the user.
void ssr::QUserInterface::_processing_button_pressed()
{
  if(_scene.get_processing_state())
  {
    _controller.stop_processing();
  }
  else
  {
    _controller.start_processing();
  }
}

/// This slot is called when the \a pause \a button was clicked by the user.
void ssr::QUserInterface::_pause_button_pressed()
{
  _controller.transport_stop();
}

/// This slot is called when the \a play \a button was clicked by the user.
void ssr::QUserInterface::_play_button_pressed()
{
  _controller.transport_start();
}

/** This function is called whenever the fiel menu actions (open/close etc.)
 * are demanded.
 */
void ssr::QUserInterface::_show_file_menu()
{
  QMenu file_menu(this);

  file_menu.setStyleSheet("min-width: 87px;");
  // "background-color: rgba(255, 0, 0, 5%)");

  QAction open_act("open",this);
  QAction save_as_act("save scene as...",this);
  //  QAction new_act("new scene",this);
  QAction close_act("quit",this);

  file_menu.addAction(&open_act);
  file_menu.addAction(&save_as_act);
  //  file_menu.addAction(&new_act);
  file_menu.addAction(&close_act);

  // connect actions to the respective functions
  connect(&open_act, SIGNAL( triggered() ), this, SLOT( _open_file() ));
  connect(&save_as_act, SIGNAL( triggered() ), this, SLOT( _save_file_as() ));
  //  connect(&new_act, SIGNAL( triggered() ), this, SLOT( _new_scene() ));
  connect(&close_act, SIGNAL( triggered() ), this, SLOT( close() ));

  //  file_menu.exec(_file_menu_label->mapToGlobal(QPoint(1,50)));
  file_menu.exec(_file_menu_label->mapToGlobal(QPoint(1,_file_menu_label->height())));
}

void ssr::QUserInterface::_new_scene()
{
  WARNING("Not implemented yet.");
}

/** This function reads the file \a _scene_menu.conf and creates the accoding
 * tabs that give fast access to a number of desired _scenes.
 */
void ssr::QUserInterface::_create_scene_menu(const std::string& path_to_scene_menu)
{
  // clear memory if list has already been in use
  if (!_scene_button_list.empty())
  {
    for (scene_button_list_t::iterator i = _scene_button_list.begin();
         i != _scene_button_list.end(); i++)
    {
      delete *i;
    }
  }

  _scene_button_list.clear();

  //std::string file_name = "scene_menu.conf";

  // open file
  std::ifstream config_file(path_to_scene_menu.c_str());

  if (!config_file.is_open())
  {
    WARNING("Cannot open scene config file '" << path_to_scene_menu << "'.");
    return;
  }

  VERBOSE("Creating scene menu from file '" << path_to_scene_menu << "'.");

  std::string line;
  std::string::size_type index;
  unsigned int no_of_scenes = 0u;
  QSceneButton* button_buffer;


  // parse file and create a button for each scene
  while (!config_file.eof())
  {
    // read line from config file
    getline(config_file, line);

    // check if line contains a comment
    index = line.find_first_of("#",0);
    if (index != line.npos) line.erase(index);

    // process the data
    if(!line.empty())
    {
      index = line.find_first_of(" ",0);

      // TODO: Is this good???
      button_buffer = new QSceneButton(_controlsParent, line.substr(index+1).c_str(), line.substr(0, index).c_str());

      button_buffer->setGeometry(DEFAULTFRAMELEFT + (SCENEBUTTONWIDTH+BETWEENSCENEBUTTONSPACE)
                                 * no_of_scenes, 67, SCENEBUTTONWIDTH, 23);
      button_buffer->setCheckable(true);
      button_buffer->setFocusPolicy(Qt::NoFocus);
      button_buffer->show();
      connect(button_buffer, SIGNAL(signal_open_scene(const QString&)), this, SLOT(_load_scene(const QString&)));

      // Add button to list
      _scene_button_list.push_back(button_buffer);
      
      // increment _scene counter
      no_of_scenes++;
    } // if

  } // while

  config_file.close();
}

/// This function opens a file dialog.
void ssr::QUserInterface::_open_file()
{
  QString path = QFileDialog::getOpenFileName(this, "Load audio scene", ".",
        "scene descriptions and audio files (*.asd *.wav);; all files (*)");

  // if a file is selected
  if (!path.isNull()) _load_scene(path);

  // adjust _time_line
  //  audiofileiotools::get_file_duration();
}

/** Sets master volume.
 * @param volume new master volume in dB
 */
void ssr::QUserInterface::_set_master_volume(float volume)
{
  // limit range
  volume = std::min(volume, MAXVOLUME);
  volume = std::max(volume, MINVOLUME);

  // convert to linear scale
  _controller.set_master_volume(apf::math::dB2linear(volume));
}

/** Changes selected sources' volume.
 * @param d_volume volume change in dB
 */
void ssr::QUserInterface::_change_volume_of_selected_sources(float d_volume)
{

  for (selected_sources_map_t::iterator i = _selected_sources_map.begin(); i != _selected_sources_map.end(); i++)
  {
    float current_gain = _scene.get_source_gain(i->second)*dB2linear(d_volume);

    // limit range (linear scale)
    current_gain = std::min(current_gain, dB2linear(MAXVOLUME));
    current_gain = std::max(current_gain, dB2linear(MINVOLUME));

    _controller.set_source_gain(i->second, current_gain);
   
  } // for
}

/// Opens a save-file-as dialog.
void ssr::QUserInterface::_save_file_as()
{
  QString file_name = QFileDialog::getSaveFileName(this, 
                        "Save scene in ASDF", ".", "ASDF files (*.asd)");

  // if aborted
  if ( file_name.isEmpty() ) 
  { 
    VERBOSE("Scene not saved.");
  } 
  else 
  {
    // convert to std::string
    std::string file_name_std = file_name.toStdString();

    // check file extension
    if ( posixpathtools::get_file_extension(file_name_std) != "asd" )
    {
      file_name_std.append(".asd");
    }

    _controller.save_scene_as_XML(file_name_std);
    VERBOSE("Scene saved in '" << file_name_std << "'.");
  }

}

/** Loads a new _scene whose asdf description resides in
 * @param path_to_scene path to _scene description file (absolute or relative).
 */
void ssr::QUserInterface::_load_scene(const QString& path_to_scene)
{

  // tell user to wait
  setCursor(Qt::WaitCursor);

  // restore standard orientation
  _rotation_x = 0.0f;

  _deselect_all_sources();

  // Check who sent the signal (if anybody)
  QSceneButton* sending_object = static_cast<QSceneButton *>(sender());

  // Check if function was called by a signal
  if (sending_object != nullptr)
  {
    // Uncheck the other buttons
    for (scene_button_list_t::iterator i = _scene_button_list.begin();
         i != _scene_button_list.end(); i++)
    {
      // Set all buttons up that have not been clicked
      if ( (*i) != sending_object )
        (*i)->setChecked(false);
    }
  }

  _controller.load_scene(std::string(path_to_scene.toUtf8()));

  // clear mouse cursor
  setCursor(Qt::ArrowCursor);
}


/** Updates all widgets on the screen including OpenGL stuff.*/
void ssr::QUserInterface::_update_screen()
{
    // update time line
    _time_line->set_progress(static_cast<float>(_scene.get_transport_position())/
                            static_cast<float>(_scene.get_sample_rate()));

    // update CPU label
    _cpu_label->set_load(_scene.get_cpu_load());

    // update volume slider (incl. master level)
    _volume_slider->update_displays(linear2dB(_scene.get_master_signal_level() + 0.00001f),
                                    linear2dB(_scene.get_master_volume() + 0.00001f)); // min -100dB

    // update transport tools
    if (_scene.is_playing() ){
      _pause_button->setChecked(false);
      _play_button->setChecked(true);
    }
    else {
      _pause_button->setChecked(true);
      _play_button->setChecked(false);
    }

    if(_scene.get_processing_state())
      _processing_button->setChecked(true);
    else
      _processing_button->setChecked(false);

    // update source properties dialog
    if (_source_properties->isVisible())
    {
      // move the dialog to the desired position
      _update_source_properties_position();
      
      // update displays of source properties dialog
      _source_properties->update_displays(_scene.get_source(_id_of_last_clicked_source), 
					  _scene.get_reference());
    } // if

    // refresh screen
    update();
}

#ifndef ENABLE_FLOATING_CONTROL_PANEL
/** Checks if a mouse event occurred outside of the visible OpenGL window.
 * @param event incoming Qt event.
 * @return @b true if event occurred outside.
 */
bool ssr::QUserInterface::_mouse_event_out_of_scope(QMouseEvent *event)
{
  if (event->x() < DEFAULTFRAMELEFT + 2
      || event->x() > width() - DEFAULTFRAMERIGHT
      || event->y() < DEFAULTFRAMETOP + 2
      || event->y() > height() - DEFAULTFRAMEBOTTOM)
  {
    return true;
  }

  return false;
}

/** Catches mouse events which occurred outside of the visible OpenGL widget.
 * @param event incoming Qt event.
 * @return @b true if event was caught.
 */
bool ssr::QUserInterface::event(QEvent *e)
{
  // check if mouse action starts outside of scope
  if (e->type() == QEvent::MouseButtonPress
      || e->type() == QEvent::MouseButtonDblClick)
  {

    // close text edit in time line if visible
    //_time_line->reset_appearence();

    QMouseEvent *mouse_event = (QMouseEvent *)e;
    if (_mouse_event_out_of_scope(mouse_event))
    {
      _ignore_mouse_events = true;
      return true;
    }
    else _ignore_mouse_events = false;
  }

  else if (e->type() == QEvent::MouseMove && _ignore_mouse_events)
  {
    return true;
  }

  return QOpenGLPlotter::event(e);
}
#endif

/** Handles Qt resize events.
 * @param event Qt resize event.
 */
void ssr::QUserInterface::resizeEvent(QResizeEvent *event)
{
  QOpenGLPlotter::resizeEvent(event);

#ifndef ENABLE_FLOATING_CONTROL_PANEL
  // resize frame
  _frame->resize(DEFAULTFRAMETOP, DEFAULTFRAMEBOTTOM, DEFAULTFRAMELEFT, DEFAULTFRAMERIGHT);

  // horizontal arrangement of buttons in pixels:
  // frame            file menu                        proc. but.
  // DEFAULTFRAMELEFT FILEMENUWIDTH BETWEENBUTTONSPACE BUTTONWIDTH BETWEENBUTTONSPACE
  // skip button                    pause                          play
  // BUTTONWIDTH BETWEENBUTTONSPACE BUTTONWIDTH BETWEENBUTTONSPACE BUTTONWIDTH
  //
  // 3*BETWEENBUTTONSPACE

  _resizeControls(width());
#endif
}

void ssr::QUserInterface::_resizeControls(int newWidth)
{
  const int _time_line_position_x = DEFAULTFRAMELEFT+FILEMENUWIDTH+
  4*(BETWEENBUTTONSPACE+BUTTONWIDTH)+
  BETWEENLABELSPACE - 12; // the 12 is due to LEFTMARGIN in qssrtimeline.cpp
  
  const int _zoom_label_position_x = newWidth - DEFAULTFRAMERIGHT + 3 -
  FILEMENUWIDTH - 2*BUTTONWIDTH - 2*BETWEENLABELSPACE;
  
  // if there is space then show the _time_line
  if (width()-DEFAULTFRAMERIGHT-FILEMENUWIDTH-150 > _time_line_position_x + 30)
  {
    _time_line->setGeometry(_time_line_position_x, 23,
                            _zoom_label_position_x - _time_line_position_x - BETWEENBUTTONSPACE, 36);
    _time_line->show();
  }
  else _time_line->hide();
  
  // if there is space then show the _zoom_label
  if (_zoom_label_position_x > _time_line_position_x - BETWEENLABELSPACE)
  {
    _zoom_label->setGeometry(_zoom_label_position_x, 30, BUTTONWIDTH, 15);
    _zoom_label_text_tag->setGeometry(_zoom_label_position_x, 15, BUTTONWIDTH, 15);
    
    _zoom_label->show();
    _zoom_label_text_tag->show();
  }
  else
  {
    _zoom_label->hide();
    _zoom_label_text_tag->hide();
  }
  
  // if there is space then show the _cpu_label
  if (_zoom_label_position_x > _time_line_position_x - 2*BETWEENLABELSPACE - BUTTONWIDTH)
  {
    _cpu_label->setGeometry(_zoom_label_position_x+BUTTONWIDTH+BETWEENLABELSPACE,
                            30, BUTTONWIDTH, 15);
    _cpu_label_text_tag->setGeometry(_zoom_label_position_x+BUTTONWIDTH+
                                     BETWEENLABELSPACE, 15, BUTTONWIDTH, 15 );
    
    _cpu_label->show();
    _cpu_label_text_tag->show();
    
  }
  else
  {
    _cpu_label->hide();
    _cpu_label_text_tag->hide();
  }
  
  _volume_slider->setGeometry(newWidth - DEFAULTFRAMERIGHT - FILEMENUWIDTH + 3, 30, FILEMENUWIDTH, 25);
  _volume_slider_text_tag->setGeometry(newWidth - DEFAULTFRAMERIGHT - FILEMENUWIDTH + 3, 15, FILEMENUWIDTH, 15 );
  
  int no_of_scenes = 0;
  
  // check which scene buttons are visible
  if (!_scene_button_list.empty())
  {
    for (scene_button_list_t::iterator i = _scene_button_list.begin();
         i != _scene_button_list.end(); i++)
    {
      if (DEFAULTFRAMELEFT + (SCENEBUTTONWIDTH+BETWEENSCENEBUTTONSPACE)
          * no_of_scenes + SCENEBUTTONWIDTH < newWidth - DEFAULTFRAMERIGHT)
      {
        (*i)->show();
      }
      else (*i)->hide();
      
      no_of_scenes++;
    }
  }
}  

/** Handles Qt mouse press events.
 * @param event Qt mouse event.
 */
void ssr::QUserInterface::mousePressEvent(QMouseEvent *event)
{
  ssr::id_t _id_of_lastlast_clicked_source = _id_of_last_clicked_source;
  
  event->accept();

  _volume_slider_selected = false;
  _reference_selected = false;
  _direction_handle_selected = false;

  _previous_mouse_event = *event;

  uint selected_object = _find_selected_object(event->pos());

  // WARNING("Object " << selected_object << " selected.");

  // get position of mouse event
  GLdouble pos_x, pos_y, pos_z;
  _get_openGL_pos(event->x(),event->y(),&pos_x,&pos_y,&pos_z);

  // initialize rubber band
  if (event->button() == Qt::RightButton)
  {
    _rubber_band_starting_point.x = _rubber_band_ending_point.x = pos_x;
    _rubber_band_starting_point.y = _rubber_band_ending_point.y = pos_y;
  }

  // if click on listener
  if (selected_object == REFERENCEINDEX1)
  {
    if (!_ctrl_pressed) _select_source(0);

     _x_offset = _scene.get_reference().position.x - pos_x;
     _y_offset = _scene.get_reference().position.y - pos_y;
  }

  // click on SSR icon
  else if (selected_object == SSRLOGOINDEX) _show_about_window();

  // check if no source was clicked
  if (selected_object <= NAMESTACKOFFSET)
  {
    // hide source properties dialog
    _source_properties->hide();
    
    // no source was clicked
    if (event->button() == Qt::LeftButton) _deselect_all_sources();
    return;
  }
  // subtract NAMESTACKOFFSET
  else // a source was clicked
  {
    selected_object -= NAMESTACKOFFSET;
  }

  // check if left click on source volume slider
  if (event->button() == Qt::LeftButton && selected_object % NAMESTACKSTEP == 2)
  {
    _volume_slider_selected = true;
  }
  // check if click on source direction handle
  else if (event->button() == Qt::LeftButton && selected_object % NAMESTACKSTEP == 3)
  {
    _direction_handle_selected = true;
  }

  // check if loudspeaker was clicked
  if (selected_object % NAMESTACKSTEP == 4 && !_ctrl_pressed)
  {
    _source_properties->hide();

    // make sure that no sound source is selected
    _deselect_all_sources();
    return;
  }
  // if not, select the respective sound source
  else  _select_source(static_cast<int>(selected_object/NAMESTACKSTEP), _ctrl_pressed);

  /////////////////////////////////////////////////////////////////
  // from here on we can be sure that a source has been selected //
  /////////////////////////////////////////////////////////////////

  std::unique_ptr<Position> source_position =
    _scene.get_source_position(_id_of_last_clicked_source);

  // save mouse pointer offset to source position
  _x_offset = source_position->x - pos_x;
  _y_offset = source_position->y - pos_y;

  // right click on source
  if (event->button() == Qt::RightButton && selected_object % NAMESTACKSTEP == 1)
  {
    if (_source_properties->isVisible() && _id_of_last_clicked_source == _id_of_lastlast_clicked_source)
      _source_properties->hide();
    else
    {
      _update_source_properties_position();
      _source_properties->show();
    }
  }
}

/** Handles Qt mouse move events.
 * @param event Qt mouse event.
 */
void ssr::QUserInterface::mouseMoveEvent(QMouseEvent *event)
{
  // move source
  if (event->buttons() == Qt::LeftButton &&
      !_selected_sources_map.empty()     &&
      !_volume_slider_selected           &&
      !_direction_handle_selected        &&
      !_selected_sources_map.empty())
  {
    GLdouble pos_x, pos_y, pos_z;

    _get_openGL_pos(event->x(), event->y(), &pos_x, &pos_y, &pos_z);

    Position position(static_cast<float>(pos_x + _x_offset),
                      static_cast<float>(pos_y + _y_offset));

    Position d_position = position -
      *_scene.get_source_position(_id_of_last_clicked_source);

    // move all selected sources
    for (selected_sources_map_t::iterator i = _selected_sources_map.begin(); i != _selected_sources_map.end(); i++)
    {
      // rotate complex sources by the appropriate angle 
      if (_scene.get_source_model(i->second) == Source::directional ||
               _scene.get_source_model(i->second) == Source::extended)
      {
        // position delta expressed as angle
        Orientation d_orientation = (*_scene.get_source_position(i->second) + d_position).orientation() -
            (*_scene.get_source_position(i->second)).orientation();

        // set the new orientation
        _controller.set_source_orientation(i->second,
                (*_scene.get_source_orientation(i->second)) + d_orientation);
      } // if

      // finally set the source's position
      // plane waves and point sources will automatically face the reference
      _controller.set_source_position(i->second,
                *_scene.get_source_position(i->second) + d_position);

    } // for

  } // if

  // rotate all selected sources
  else if ((event->buttons() == Qt::LeftButton) && _direction_handle_selected)
  {
    GLdouble pos_x,pos_y,pos_z;

    _get_openGL_pos(event->x(),event->y(),&pos_x,&pos_y,&pos_z);

    // absolut mouse position in OpenGL coordinates
    Position mouse_pos(static_cast<float>(pos_x), static_cast<float>(pos_y));

    // position relative to source position
    mouse_pos -= *_scene.get_source_position(_id_of_last_clicked_source);

    _get_openGL_pos(_previous_mouse_event.x(),
                    _previous_mouse_event.y(),
                    &pos_x,&pos_y,&pos_z);

    // previous absolute position in OpenGL coordinates
    Position prev_mouse_pos(static_cast<float>(pos_x), static_cast<float>(pos_y));

    // previous position relative to source position
    prev_mouse_pos -= *_scene.get_source_position(_id_of_last_clicked_source);

    // rotate all selected sources that can be rotated
    for (selected_sources_map_t::iterator i = _selected_sources_map.begin();
         i != _selected_sources_map.end(); i++)
    {
      if (_scene.get_source_model(i->second) == Source::directional ||
          _scene.get_source_model(i->second) == Source::extended)
      {
        _controller.set_source_orientation(i->second,
                (*_scene.get_source_orientation(i->second)) +
                (mouse_pos.orientation() - prev_mouse_pos.orientation()));
      }
    } // for

  }

  // change source volume
  else if ((event->buttons() == Qt::LeftButton) && _volume_slider_selected)
  {
    float gain;

    // change gain of all selected sources
    for (selected_sources_map_t::iterator i = _selected_sources_map.begin();
         i != _selected_sources_map.end(); i++)
    {
      // get current source gain
      gain = _scene.get_source_gain(i->second);
      // if source is not found, gain = 0.

      // convert to dB
      gain = linear2dB(gain);

      // calculate gain according to mouse position
      gain += (event->globalX() - _previous_mouse_event.globalX())
        * VOLUMEACCELERATION;

      // change gain
      if (gain > MINVOLUME && gain <= MAXVOLUME)
      {
        _controller.set_source_gain(i->second, dB2linear(gain));
      }
    } // for

  } // else if

  // rotate listener
  else if ((event->buttons() == Qt::LeftButton) && _reference_selected)
  {
    GLdouble pos_x,pos_y,pos_z;

    _get_openGL_pos(event->x(),event->y(),&pos_x,&pos_y,&pos_z);

    // absolut mouse position in OpenGL coordinates
    Position mouse_pos(static_cast<float>(pos_x), static_cast<float>(pos_y));

    // position relative to reference position
    mouse_pos -= _scene.get_reference().position;

    _get_openGL_pos(_previous_mouse_event.x(),
                    _previous_mouse_event.y(),
                    &pos_x,&pos_y,&pos_z);

    // previous absolut position in OpenGL coordinates
    Position prev_mouse_pos(static_cast<float>(pos_x), static_cast<float>(pos_y));

    // previous position relative to relative position
    prev_mouse_pos -= _scene.get_reference().position;

    _controller.set_reference_orientation(_scene.get_reference().orientation +
                                          (mouse_pos.orientation() - prev_mouse_pos.orientation()));

  } // else if

  // translate listener
  else if (event->buttons() == Qt::RightButton && _reference_selected)
  {
    GLdouble pos_x,pos_y,pos_z;
    Position position;

    _get_openGL_pos(event->x(),event->y(),&pos_x,&pos_y,&pos_z);

    position.x = static_cast<float>(pos_x + _x_offset);
    position.y = static_cast<float>(pos_y + _y_offset);

    _controller.set_reference_position(position);
  } // else if

  // right click on background
  else if ((event->buttons() == Qt::RightButton) && !_reference_selected)
  {
      GLdouble pos_x, pos_y, pos_z;
      _get_openGL_pos(event->x(),event->y(),&pos_x,&pos_y,&pos_z);

      _rubber_band_ending_point.x = pos_x;
      _rubber_band_ending_point.y = pos_y;
  } // else if

  // translate whole plot
  else if ((event->buttons() == Qt::LeftButton) && !_reference_selected)
  {
    _window_x_offset += (event->globalX() - _previous_mouse_event.globalX())
      / _zoom_factor*2.0f;
    _window_y_offset -= (event->globalY() - _previous_mouse_event.globalY())
      / _zoom_factor*2.0f;

  } // else if

  // store mouse position
  _previous_mouse_event = *event;

}


void ssr::QUserInterface::_update_source_properties_position()
{
  if (!_id_of_last_clicked_source) return;

  std::unique_ptr<Position> source_position =
    _scene.get_source_position(_id_of_last_clicked_source);


  int x, y;
  _get_pixel_pos(source_position->x, source_position->y,
		 0,&x, &y);

#ifdef __APPLE__
  _source_properties->move(QPoint(this->x() + x + 100, this->y() + y));
#else
  _source_properties->move(QPoint(x + 100, y));
#endif

}

/** Handles Qt mouse double click events.
 * @param event Qt mouse event.
 */
void ssr::QUserInterface::mouseDoubleClickEvent(QMouseEvent *event)
{
  event->accept();

  // double click on background or listener
  if (_selected_sources_map.empty())
  {
    // hide source properties dialog
    _source_properties->hide();

    // return to reference position
    _window_x_offset = 0.0f;
    _window_y_offset = STDWINDOWYOFFSET;

    // restore zoom
    _set_zoom(100);
  }
  else 
  { 
    // double click on source
    if (_source_properties->isVisible())
    {
      _source_properties->hide();
    }
    else
    {
      ;//_source_properties->show();
    }
  }

}

/** Handles Qt mouse release events.
 * @param event Qt mouse event.
 */
void ssr::QUserInterface::mouseReleaseEvent (QMouseEvent *event)
{
  event->accept();

  if (event->button() == Qt::RightButton)
  {
    // clear rubber band
    _rubber_band_starting_point = _rubber_band_ending_point;
  }
}

#ifdef ENABLE_FLOATING_CONTROL_PANEL
/** Catches Qt key press events for floating control panel.
 * @param sender Qt object that sent the event.
 * @param event The event itself.
 */
bool ssr::QUserInterface::eventFilter(QObject *sender, QEvent *event)
{
  (void)sender;
  if (event->type() == QEvent::KeyPress)
  {
    QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
    this->keyPressEvent(keyEvent);
    return true;
  }
  return false;
}
#endif

/** Handles Qt key press events.
 * @param event Qt key event.
 */
void ssr::QUserInterface::keyPressEvent(QKeyEvent *event)
{
  event->accept();

  switch ( event->key() )
  {
    // actions directly related to OpenGL window
//  case Qt::Key_Plus: _set_zoom(static_cast<int>(_zoom_factor/STDZOOMFACTOR * 110.0f + 0.5f)); break;
//  case Qt::Key_Minus: _set_zoom(static_cast<int>(_zoom_factor/STDZOOMFACTOR * 90.0f + 0.5f)); break;

  case Qt::Key_Up: _window_y_offset -= 0.1f; update(); break;
  case Qt::Key_Down: _window_y_offset += 0.1f; update(); break;
  case Qt::Key_Left: _window_x_offset += 0.1f; update(); break;
  case Qt::Key_Right: _window_x_offset -= 0.1f; update(); break;
  case Qt::Key_Space: if (_scene.is_playing()) _controller.transport_stop();
                      else _controller.transport_start();
                      break;
  case Qt::Key_Backspace: _skip_back(); break;

    // actions related to sources
  case Qt::Key_A: if (event->modifiers() == Qt::ControlModifier) _select_all_sources(); break;
  case Qt::Key_F: _toggle_fixation_state_of_selected_sources(); break;
  case Qt::Key_M: _toggle_mute_state_of_selected_sources(); break;
  case Qt::Key_P: _toggle_source_models(); break;  
  case Qt::Key_R: _controller.set_auto_rotation(!_scene.get_auto_rotation()); break;

  case Qt::Key_S: if ( event->modifiers() == Qt::ControlModifier ) 
                  {
                    _save_file_as();
                  }
                  else if (_selected_sources_map.empty()) 
                  {
                    _unsolo_all_sources();
                  }
		  else _solo_selected_sources(); 
                  break;

  case Qt::Key_T: if ( event->modifiers() == Qt::ControlModifier ) 
                  {
		    _time_line->show_time_edit(); 
                  }
                  break;

  case Qt::Key_0: _deselect_all_sources(); break;
  case Qt::Key_1: _select_source(1, _ctrl_pressed); break;
  case Qt::Key_2: _select_source(2, _ctrl_pressed); break;
  case Qt::Key_3: _select_source(3, _ctrl_pressed); break;
  case Qt::Key_4: _select_source(4, _ctrl_pressed); break;
  case Qt::Key_5: _select_source(5, _ctrl_pressed); break;
  case Qt::Key_6: _select_source(6, _ctrl_pressed); break;
  case Qt::Key_7: _select_source(7, _ctrl_pressed); break;
  case Qt::Key_8: _select_source(8, _ctrl_pressed); break;
  case Qt::Key_9: _select_source(9, _ctrl_pressed); break;

    // miscellaneous actions
  case Qt::Key_Plus: if (_selected_sources_map.empty())
		     { 
		       // change master level
		       _set_master_volume(linear2dB(_scene.get_master_volume() + 0.00001f) + 1.0f); // min -100dB
		     }
                     // change selected sources level
		     else _change_volume_of_selected_sources(1.0f);
                     break;   
  case Qt::Key_Minus: if (_selected_sources_map.empty())
		     { 
		       // change master level
		       _set_master_volume(linear2dB(_scene.get_master_volume() + 0.00001f) - 1.0f); // min -100dB
		     }
                     // change selected sources level
		     else _change_volume_of_selected_sources(-1.0f);
                     break; 
  case Qt::Key_Return: {_controller.calibrate_client(); break; }
  case Qt::Key_Control: {_ctrl_pressed = true; break; }
  case Qt::Key_Alt: {_alt_pressed = true; break; }
  case Qt::Key_F11: {if (!isFullScreen()) setWindowState(Qt::WindowFullScreen);
      else setWindowState( Qt::WindowNoState );} break;
  case Qt::Key_C: {if (event->modifiers() == Qt::ControlModifier)
        close();
      else
        break;}
  case Qt::Key_Escape: close();
  } // switch
}

/** Handles Qt key release events.
 * @param event Qt key event.
 */
void ssr::QUserInterface::keyReleaseEvent(QKeyEvent *event)
{
    switch (event->key()){
    case Qt::Key_Control: {_ctrl_pressed = false; break; }
    case Qt::Key_Alt: {_alt_pressed = false; break; }
    default: ;
    }
}

/** Handles Qt mouse wheel events.
 * @param event Qt mouse wheel event.
 */
void ssr::QUserInterface::wheelEvent(QWheelEvent *event)
{
  event->accept();

  // update zoom
  _set_zoom(static_cast<int>(_zoom_factor/STDZOOMFACTOR *
                             (100.0f+event->delta()/100.f*5.0f) + 0.5f));
}

/// Displays the about window.
void ssr::QUserInterface::_show_about_window()
{
  const std::string about_string =
    "<B>" PACKAGE_NAME "</B><BR>"
    "&nbsp;version " PACKAGE_VERSION
    "<BR><BR>"
    SSR_AUTHORS_QT
    "<BR><BR>"
    "<EM>Website</EM>:&nbsp;" PACKAGE_URL "<BR>"
    "<EM>e-Mail</EM>:&nbsp;&nbsp;" PACKAGE_BUGREPORT "<BR>"
    "<BR>"
    "Copyright &copy; 2012-2014 Institut f&uuml;r Nachrichtentechnik<BR>"
    "&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;"
    "&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;"
    "&nbsp;&nbsp;&nbsp;Universit&auml;t Rostock<BR>"
    "Copyright &copy; 2006-2012 Quality &amp; Usability Lab<BR>"
    "&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;"
    "&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;"
    "&nbsp;&nbsp;&nbsp;Telekom Innovation Laboratories<BR>"
    "&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;"
    "&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;"
    "&nbsp;&nbsp;&nbsp;TU Berlin<BR>"
    "<BR>"
    "This program comes with ABSOLUTELY NO WARRANTY;<BR>"
    "this is free software, and you are welcome<BR>"
    "to redistribute it under certain conditions;<BR>"
    "for details, see the enclosed file COPYING.<BR>";

  QDialog about_window(this, Qt::Popup);

  about_window.setAutoFillBackground(true);
  about_window.setPalette(QPalette(QColor(244, 244, 244)));

  QClickTextLabel ssr_logo;
  QClickTextLabel text_label;

  ssr_logo.setGeometry(20, 30, 310, 211); // the SSR Logo has 310x211 pixels
  ssr_logo.setScaledContents(true);

  QString path_to_image( _path_to_gui_images.c_str() );
  path_to_image.append("/ssr_logo_large.png");

  ssr_logo.setPixmap( QPixmap( path_to_image ) );

  // text_label.setGeometry(0, 261, 350, 370);
  text_label.setText(about_string.c_str());
  text_label.setIndent(20); 
  text_label.setAlignment(Qt::AlignTop);
  text_label.adjustSize();

  // arrange widgets in vertical layout
  QVBoxLayout *layout = new QVBoxLayout;

  layout->addWidget(&ssr_logo);
  layout->addWidget(&text_label);

  about_window.setLayout(layout);

  //about_window.adjustSize();

  connect(&ssr_logo, SIGNAL(clicked()), &about_window, SLOT(close()));
  connect(&text_label, SIGNAL(clicked()), &about_window, SLOT(close()));
 
  about_window.exec();
}

/** Sets the mute state of the currently active mouse.
 * @param flag \a true or \a false
 */
void ssr::QUserInterface::_set_source_mute(const bool flag)
{
  _controller.set_source_mute(_id_of_last_clicked_source, flag);
}

/// Toggles the mute state of all selected sound sources
void ssr::QUserInterface::_toggle_mute_state_of_selected_sources()
{
  // iterate over selected sources
  for (  selected_sources_map_t::iterator i = _selected_sources_map.begin();
         i != _selected_sources_map.end(); i++)
  {
    _controller.set_source_mute(i->second,
                                !_scene.get_source_mute_state(i->second));
  }
}

void ssr::QUserInterface::_toggle_source_models()
{
  // toggle all source types between "plane" and "point"
  for (selected_sources_map_t::iterator i = _selected_sources_map.begin(); i != _selected_sources_map.end(); i++)
  {
    // if source is plane then make sure that it faces the reference
    if (_scene.get_source_model(i->second) == Source::plane)
    {
      _controller.set_source_model(i->second, Source::point);
    }
    else if (_scene.get_source_model(i->second) == Source::point)
    {
      _controller.set_source_model(i->second, Source::plane);      
    } // if
  } // for
}

/** Sets the position fixed state of the currently active mouse.  
 * @param flag \a true or \a false
 */
void ssr::QUserInterface::_set_source_position_fixed(const bool flag)
{
  _controller.set_source_position_fixed(_id_of_last_clicked_source, flag);
}

/** Sets the position fixed state of the currently active mouse.  
 * @param index \a 0="plane wave" \a 1="point source" 
 */
void ssr::QUserInterface::_set_source_model(const int index)
{
  Source::model_t model = Source::unknown;
  
  switch(index){
    case 0:
      model = Source::plane;
      break;
    case 1:
      model = Source::point;
      break;
  }
  VERBOSE("index: " << index);

  ssr::id_t id = _id_of_last_clicked_source;
  _controller.set_source_model(id, model);
  // make sure that the plane wave is oriented towards the reference point
  _controller.set_source_orientation(id, (_scene.get_reference().position
        - *_scene.get_source_position(id)).orientation());
}

void ssr::QUserInterface::_toggle_fixation_state_of_selected_sources()
{
  // iterate over selected sources
  for (  selected_sources_map_t::iterator i = _selected_sources_map.begin();
         i != _selected_sources_map.end(); i++)
  {
    _controller.set_source_position_fixed(i->second,
                                !_scene.get_source_position_fixed(i->second));
  }
}

// Settings for Vim (http://www.vim.org/), please do not remove:
// vim:softtabstop=2:shiftwidth=2:expandtab:textwidth=80:cindent
// vim:fdm=expr:foldexpr=getline(v\:lnum)=~'/\\*\\*'&&getline(v\:lnum)!~'\\*\\*/'?'a1'\:getline(v\:lnum)=~'\\*\\*/'&&getline(v\:lnum)!~'/\\*\\*'?'s1'\:'='
