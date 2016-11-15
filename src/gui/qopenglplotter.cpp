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

#include <stdio.h>
#include <stdarg.h>
#include <cmath>

#include <QtCore/QPoint>
#include <QtCore/QRect>
#include <QtCore/QSize>
#include <QtGui/QCloseEvent>
#include <QtGui/QImage>
#include <QtGui/QMouseEvent>
#include <QtWidgets/QAction>
#include <QtWidgets/QDialog>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QMenu>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSlider>

#include "qopenglplotter.h"
#include "qclicktextlabel.h"
//#include "mathtools.h"

#define BACKGROUNDCOLOR 0.9294f,0.9294f,0.9020f
// Define how detailedly circles are plotted
#define LEVELOFDETAIL 40

////////////////////////////////////////////////////////////////////////////////
// Implementation of the nested class QGUI::SourceCopy
////////////////////////////////////////////////////////////////////////////////

/** Type conversion constructor.
 * @param other reference to an element of the source map
 **/
ssr::QOpenGLPlotter::SourceCopy::SourceCopy(
    const Scene::source_map_t::value_type& other) :
  //const std::pair<Source::id_t, Source*>& other) :
  DirectionalPoint(other.second),
  id(other.first),
  model(other.second.model),
  mute(other.second.mute),
  gain(other.second.gain),
  signal_level(other.second.signal_level),
  output_levels(other.second.output_levels),
  name(other.second.name),
  fixed_position(other.second.fixed_position)
{}

////////////////////////////////////////////////////////////////////////////////

ssr::QOpenGLPlotter::QOpenGLPlotter(Publisher& controller, const Scene& scene
        , const std::string& path_to_gui_images
        , QWidget *parent)
  : QGLWidget(parent),
    _controller(controller),
    _scene(scene), 
    _active_source(-1),
    _path_to_gui_images(path_to_gui_images),
    _id_of_last_clicked_source(0),
    _zoom_factor(STDZOOMFACTOR),
    _previous_mouse_event(QMouseEvent(QEvent::MouseButtonPress,
          QPoint(),
          Qt::LeftButton,
          Qt::LeftButton,
          Qt::NoModifier)), // dummy event
    _ctrl_pressed(false),
    _alt_pressed(false),
    _rubber_band_starting_point(Position()),
    _rubber_band_ending_point(Position()),
    _window_x_offset(0.0f),
    _window_y_offset(STDWINDOWYOFFSET),
    _window_z_offset(0.0f),
    _x_offset(0.0f),
    _y_offset(0.0f),
    _z_offset(0.0f),
    _rotation_x(0.0f),
    _rotation_y(0.0f),
    _rotation_z(0.0f),
    _reference_selected(false),
    _volume_slider_selected(false),
    _direction_handle_selected(false),
    _allow_displaying_text(true),
    _glu_quadric(gluNewQuadric()),
    _plot_listener(false)
{
  _set_zoom(100); // 100%

  _soloed_sources.clear();

  // define possible colors for sources
  _color_vector.push_back(QColor(163, 95, 35));
  _color_vector.push_back(QColor( 43,174,247));
  _color_vector.push_back(QColor( 75,135, 35));
  _color_vector.push_back(QColor( 97, 31,160));
  _color_vector.push_back(QColor(173, 54, 35));
  //_color_vector.push_back(QColor(242,226, 22));  // yellow is too hard to read

}

ssr::QOpenGLPlotter::~QOpenGLPlotter()
{
  // delete source_properties;
}

void ssr::QOpenGLPlotter::_load_background_textures()
{
  QImage image_buffer;

  image_buffer = QImage(); 

  // load SSR logo
  QString path_to_image( _path_to_gui_images.c_str() );
  path_to_image.append("/ssr_logo.png");

  image_buffer.load(path_to_image, "PNG");

  if (!image_buffer.isNull()) _ssr_logo_texture = bindTexture(image_buffer);
  else 
    ERROR("Texture \"" << path_to_image.toUtf8().data() << "\" not loaded.");

  image_buffer = QImage(); 

  // load source shadow texture
  path_to_image = 
        QString( _path_to_gui_images.c_str() ).append("/source_shadow.png");

  image_buffer.load(path_to_image, "PNG");

  if (!image_buffer.isNull()) 
  {
    _source_shadow_texture = bindTexture(image_buffer);
  }
  else 
    ERROR("Texture \"" << path_to_image.toUtf8().data() << "\" not loaded.");

  if (_controller.show_head())
  {
    _plot_listener = true;

    // load listener texture
    image_buffer = QImage();

    path_to_image = 
        QString( _path_to_gui_images.c_str() ).append("/listener.png");

    image_buffer.load(path_to_image, "PNG");

    if (!image_buffer.isNull()) _listener_texture = bindTexture(image_buffer);
    else 
     ERROR("Texture \"" << path_to_image.toUtf8().data() << "\" not loaded.");

    // load listener shadow texture
    image_buffer = QImage();

    path_to_image = 
        QString( _path_to_gui_images.c_str() ).append("/listener_shadow.png");

    image_buffer.load(path_to_image, "PNG");

    if (!image_buffer.isNull())
    { 
      _listener_shadow_texture = bindTexture(image_buffer);
    }
    else 
     ERROR("Texture \"" << path_to_image.toUtf8().data() << "\" not loaded.");

    // load listener background texture
    image_buffer = QImage();

    path_to_image = 
     QString( _path_to_gui_images.c_str() ).append("/listener_background.png");

    image_buffer.load(path_to_image, "PNG");

    if (!image_buffer.isNull()) 
    {
      _listener_background_texture = bindTexture(image_buffer);
    }
    else 
     ERROR("Texture \"" << path_to_image.toUtf8().data() << "\" not loaded.");

  }


}

void 
ssr::QOpenGLPlotter::initializeGL()
{
  glClearColor(1.0,1.0,1.0,1.0);

  // done using Qt now:
  // glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH | GLUT_MULTISAMPLE);
  
  glEnable(GL_CULL_FACE);
  glHint(GL_PERSPECTIVE_CORRECTION_HINT,GL_FASTEST);

  glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

  // for blending
  //glColor4f(.0f,.0f,.0f,1.0f);
  //glBlendFunc(GL_SRC_ALPHA,GL_ONE);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  //glPolygonOffset(-0.2, -1.0); // TODO: necessary ???

  glTexParameteri(GL_TEXTURE_2D, 
		  GL_TEXTURE_MAG_FILTER, 
		  GL_LINEAR_MIPMAP_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, 
		  GL_TEXTURE_MIN_FILTER, 
		  GL_LINEAR_MIPMAP_NEAREST);

  // load TLabs logo and SSR logo
  _load_background_textures();

  update();
}

void 
ssr::QOpenGLPlotter::resizeGL(int width, int height)
{
  glViewport(0, 0, width, height);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(-(float)width/_zoom_factor, (float)width/_zoom_factor,
          -(float)height/_zoom_factor, (float)height/_zoom_factor, 1.0, 15.0);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
	
  _glu_quadric = gluNewQuadric();
}

void 
ssr::QOpenGLPlotter::paintGL()
{
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  gluLookAt(0.0f, 0.0f, 10.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f);

  // translate to center of plot
  glTranslatef(_window_x_offset, _window_y_offset, _window_z_offset);

  _draw_background();

  // rotate plot (not implemented yet)
  glRotatef(_rotation_x, 0.0f, 0.0f, 1.0f);

  _draw_reference();

  _draw_objects();

  _draw_rubber_band();
}

void 
ssr::QOpenGLPlotter::_draw_background()
{
  // background color
  glColor3f(BACKGROUNDCOLOR);
  // ID of background
  glLoadName(BACKGROUNDINDEX);

  // plot background
  glBegin(GL_QUADS);
    glVertex3f( (-(float)width() /_zoom_factor - _window_x_offset),
                (-(float)height()/_zoom_factor - _window_y_offset),
                0.0f);
    glVertex3f( ( (float)width() /_zoom_factor - _window_x_offset),
                (-(float)height()/_zoom_factor - _window_y_offset),
                0.0f);
    glVertex3f( ( (float)width() /_zoom_factor - _window_x_offset),
                ( (float)height()/_zoom_factor - _window_y_offset),
                0.0f);
    glVertex3f( (-(float)width() /_zoom_factor - _window_x_offset),
                ( (float)height()/_zoom_factor - _window_y_offset),
                0.0f);
  glEnd();

  // Draw textured stuff
  glColor3f(1.0f,1.0f,1.0f);
  glEnable(GL_TEXTURE_2D);

  // draw SSR logo
  glPushMatrix();

  // TODO: Incorporate the following line more elegantly
  glTranslatef(-_window_x_offset, -_window_y_offset, -_window_z_offset);

  // translate to respective position // TODO: Include it in verteces
  // fixed position
  // glTranslatef( -4.1f*STDZOOMFACTOR/_zoom_factor, -3.3f*STDZOOMFACTOR/_zoom_factor, 0.0f);

  // relative position
  glTranslatef( (-(float)width() /_zoom_factor + 0.4f*STDZOOMFACTOR/_zoom_factor),
                (-(float)height()/_zoom_factor + 0.3f*STDZOOMFACTOR/_zoom_factor), 0.0f);

  glBindTexture(GL_TEXTURE_2D, _ssr_logo_texture);

  glLoadName(SSRLOGOINDEX); // ID of logo
  
  glColor3f(1.0f,1.0f,1.0f);

  glBegin(GL_QUADS);
  glTexCoord2f(0.0f, 0.0f); glVertex3f(0.0f, 0.0f, 0.0f);
  glTexCoord2f(1.0f, 0.0f); glVertex3f(0.5f*STDZOOMFACTOR/_zoom_factor, 0.0f, 0.0f);
  glTexCoord2f(1.0f, 1.0f); glVertex3f(0.5f*STDZOOMFACTOR/_zoom_factor, 0.5f*STDZOOMFACTOR/_zoom_factor, 0.0f);
  glTexCoord2f(0.0f, 1.0f); glVertex3f(0.0f, 0.5f*STDZOOMFACTOR/_zoom_factor, 0.0f);
  glEnd();
  
  glDisable(GL_TEXTURE_2D);

  glPopMatrix();
}

void ssr::QOpenGLPlotter::_draw_reference()
{
  glEnable(GL_MULTISAMPLE);
  glEnable(GL_POLYGON_OFFSET_LINE);

  glPushMatrix();

  // translate according to reference position
  glTranslatef(_scene.get_reference().position.x, 
                                 _scene.get_reference().position.y, 0.0f);

  glPushMatrix();
  
  glLoadName(REFERENCEINDEX1); // ID of handle to rotate listener

  float scale = 1.0f;

  if (_plot_listener)
  {
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    
    glBindTexture(GL_TEXTURE_2D, _listener_background_texture);
    gluQuadricTexture(_glu_quadric, GL_TRUE );
    gluDisk(_glu_quadric, 0.0f, 0.7f, LEVELOFDETAIL,1);
    
    glTranslatef(0.03f, -0.03f, 0.0f);
   
    // rotate according to reference position
    glRotatef(_scene.get_reference().orientation.azimuth, 0.0f, 0.0f, 1.0f);

    glBindTexture(GL_TEXTURE_2D, _listener_shadow_texture);

    glBegin(GL_QUADS);
      glTexCoord2f(0.0f, 0.0f); glVertex3f(-0.35f, -0.35f, 0.0f);
      glTexCoord2f(1.0f, 0.0f); glVertex3f( 0.35f, -0.35f, 0.0f);
      glTexCoord2f(1.0f, 1.0f); glVertex3f( 0.35f,  0.35f, 0.0f);
      glTexCoord2f(0.0f, 1.0f); glVertex3f(-0.35f,  0.35f, 0.0f);
    glEnd();
    
    glPopMatrix();

    // rotate according to reference position
    glRotatef(_scene.get_reference().orientation.azimuth, 0.0f, 0.0f, 1.0f);
    
    glBindTexture(GL_TEXTURE_2D, _listener_texture);

    glBegin(GL_QUADS);
      glTexCoord2f(0.0f, 0.0f); glVertex3f(-0.3f, -0.3f, 0.0f);
      glTexCoord2f(1.0f, 0.0f); glVertex3f( 0.3f, -0.3f, 0.0f);
      glTexCoord2f(1.0f, 1.0f); glVertex3f( 0.3f,  0.3f, 0.0f);
      glTexCoord2f(0.0f, 1.0f); glVertex3f(-0.3f,  0.3f, 0.0f);
    glEnd();
    
    glDisable(GL_BLEND);
    glDisable(GL_TEXTURE_2D);
  }
  else
  {
    // rotate according to reference position
    glRotatef(_scene.get_reference().orientation.azimuth, 0.0f, 0.0f, 1.0f);

    // background color
    glColor3f(BACKGROUNDCOLOR);

    // define handle to interact with reference
    glBegin(GL_QUADS);
      glVertex3f( 0.15f * scale, -0.05f * scale, 0.0f);
      glVertex3f( 0.15f * scale,  0.05f * scale, 0.0f);
      glVertex3f(-0.05f * scale,  0.05f * scale, 0.0f);
      glVertex3f(-0.05f * scale, -0.05f * scale, 0.0f);
    glEnd();

    // gray
    glColor3f(0.4f, 0.4f, 0.4f);

    // draw rhomb
    glBegin(GL_LINES);
      glVertex3f( 0.0f, -0.05f * scale, 0.0f);
      glVertex3f( 0.15f * scale, 0.0f,  0.0f);
      glVertex3f( 0.15f * scale, 0.0f,  0.0f);
      glVertex3f( 0.0f , 0.05f * scale, 0.0f);
      glVertex3f( 0.0f , 0.05f * scale, 0.0f);
      glVertex3f(-0.05f * scale, 0.0f,  0.0f);
      glVertex3f(-0.05f * scale, 0.0f,  0.0f);
      glVertex3f( 0.0f, -0.05f * scale, 0.0f);
    glEnd();

    // somehow this 90 degree rotation is needed
    glRotatef(-90, 0.0f, 0.0f, 1.0f);

    // rotate/translate according to reference offset
    glTranslatef(_scene.get_reference_offset().position.x
        , _scene.get_reference_offset().position.y, 0.0f);
    glRotatef(_scene.get_reference_offset().orientation.azimuth
        , 0.0f, 0.0f, 1.0f);

    // draw cross (showing the reference offset)
    glBegin(GL_LINES);
      glVertex3f( 0.02f * scale, 0.0f,  0.0f);
      glVertex3f(-0.02f * scale, 0.0f,  0.0f);
      glVertex3f( 0.0f, -0.02f * scale, 0.0f);
      glVertex3f( 0.0f,  0.03f * scale, 0.0f);
    glEnd();
  } 

  glPopMatrix();
  glPopMatrix();

  // mark origin of coordinate system
  glColor3f(0.4f, 0.4f, 0.4f);

  // draw origin of coordinate system
  glBegin(GL_LINES);
    glVertex3f( 0.02f, 0.0f,  0.0f);
    glVertex3f(-0.02f, 0.0f,  0.0f);
    glVertex3f( 0.0f, -0.02f, 0.0f);
    glVertex3f( 0.0f,  0.02f, 0.0f);
  glEnd();

  glDisable(GL_POLYGON_OFFSET_LINE);
  glDisable(GL_MULTISAMPLE);
}

void ssr::QOpenGLPlotter::_draw_objects()
{
  // enable anti-aliasing
  glEnable(GL_MULTISAMPLE);

  // get source info
  source_buffer_list_t source_buffer_list;
  _scene.get_sources(source_buffer_list);

  // plot loudspeakers ////////////////////////////////////////////////////////

  std::vector<float> output_levels;

  if (_id_of_last_clicked_source > 0)
  {
    source_buffer_list_t::const_iterator temp = source_buffer_list.begin();
    std::advance(temp, _id_of_last_clicked_source - 1);
    output_levels = temp->output_levels;
  }

  _loudspeakers.clear();
  _scene.get_loudspeakers(_loudspeakers);

  glLoadName(NAMESTACKOFFSET + source_buffer_list.size() * NAMESTACKSTEP + 4);

  for (Loudspeaker::container_t::size_type i = 0; i < _loudspeakers.size(); ++i)
  {
    glPushMatrix();

    // TODO: z position???
    glTranslatef(_loudspeakers[i].position.x, _loudspeakers[i].position.y, 0.0f);
    glRotatef(_loudspeakers[i].orientation.azimuth, 0.0f, 0.0f, 1.0f);

    // TODO: display actual level instead of on/off value
    bool active = false;
    if (output_levels.size() > i) active = output_levels[i];

    _draw_loudspeaker(_loudspeakers[i].mute, active);

    glPopMatrix();
  } // for

  // end plot loudspeakers ////////////////////////////////////////////////////

  // plot sound sources ///////////////////////////////////////////////////////
  int n = 1;

  for (source_buffer_list_t::const_iterator i = source_buffer_list.begin(); i != source_buffer_list.end(); i++)
  {
   
    glPushMatrix();

    // go to source position
    glTranslatef(i->position.x,i->position.y,0.0);

    // draw it
    _draw_source(i, n);

    glPopMatrix();

    // increment counter
    n++;

  } // for
  // end plot sources /////////////////////////////////////////////////////////

  // disable anti-aliasing
  glDisable(GL_MULTISAMPLE);
}

void 
ssr::QOpenGLPlotter::_draw_source(source_buffer_list_t::const_iterator& source,
			     unsigned int index)
{
  float scale = 1.0f;

  bool selected = false;
  bool soloed = false;

  if (_selected_sources_map.find(index) != _selected_sources_map.end()) 
  {
    selected = true;
  }

  if (_soloed_sources.find(source->id) != _soloed_sources.end())
  {
    soloed = true;
  }
  
  // check if source is selected
  if (selected) scale = 2.0f;

  // dummy id to prevend mouse action
  glLoadName(DUMMYINDEX);

  // draw shadow texture
  glEnable(GL_TEXTURE_2D);
  //glEnable(GL_ALPHA_TEST);
  glEnable(GL_BLEND);

  glBindTexture(GL_TEXTURE_2D, _source_shadow_texture);

  glPushMatrix();
  glTranslatef(0.015f, -0.015f, 0.01f);
  gluQuadricTexture(_glu_quadric, GL_TRUE );
  gluDisk(_glu_quadric, 0.0f, 0.163f * scale,LEVELOFDETAIL,1);
  glPopMatrix();

  glDisable(GL_TEXTURE_2D);
  //glDisable(GL_ALPHA_TEST);
  glDisable(GL_BLEND);

  // make sure that source is drawn on top
  //if (selected) glTranslatef(0.0f, 0.0f, 0.2f);
  //else          
  glTranslatef(0.0f, 0.0f, 0.1f);
  // TODO: make it more elegant

  // source id
  glLoadName(NAMESTACKOFFSET + index * NAMESTACKSTEP + 1);

  // draw source
  glColor3f(1.0f, 1.0f, 1.0f); // white

  gluDisk(_glu_quadric, 0.0f, 0.15f * scale, LEVELOFDETAIL, 1);

  // fill source
  qglColor(_color_vector[source->id%_color_vector.size()]);

  gluPartialDisk(_glu_quadric, 0.0f, 0.125f * scale,
                 LEVELOFDETAIL, 1, 0.0f, 360.0f);

  // draw solo indication
  if (soloed)
  {
    gluPartialDisk(_glu_quadric, 0.2f * scale, 0.22f * scale,
		   LEVELOFDETAIL, 1, 55.0f, 70.0f);  
    gluPartialDisk(_glu_quadric, 0.2f * scale, 0.22f * scale,
		   LEVELOFDETAIL, 1, 235.0f, 70.0f);
  }

  // choose color
  if (source->mute) glColor3f(0.5f, 0.5f, 0.5f);
  else qglColor(_color_vector[source->id%_color_vector.size()]);
    
  // draw ring around source
  gluDisk(_glu_quadric, 0.14f * scale, 0.15f * scale, LEVELOFDETAIL, 1);

  // volume slider id; It is accessible only when source selected
  if (selected) glLoadName(NAMESTACKOFFSET + index * NAMESTACKSTEP + 2);

  // plot level meter frame in black
  glBegin(GL_TRIANGLE_FAN);
    glVertex3f( -0.1f * scale, -0.2f * scale, 0.0f);
    glVertex3f( -0.1f * scale, -0.25f * scale, 0.0f);
    glVertex3f(  0.1f * scale, -0.25f * scale, 0.0f);
    glVertex3f(  0.1f * scale, -0.2f * scale, 0.0f);
  glEnd();

  if (selected)
  {
    // plot volume slider
    glBegin(GL_TRIANGLES);
      glVertex3f((-0.09f * scale + (20.0f*log10(source->gain)+50.0f)/62.0f
                  * 0.18f * scale ) - 0.015f * scale, -0.29f * scale, 0.0f);
      glVertex3f((-0.09f * scale + (20.0f*log10(source->gain)+50.0f)/62.0f
                  * 0.18f * scale ) + 0.015f * scale, -0.29f * scale, 0.0f);
      glVertex3f((-0.09f * scale + (20.0f*log10(source->gain)+50.0f)/62.0f
                  * 0.18f * scale ), -0.26f * scale, 0.0f);
    glEnd();
  }

  if (source->fixed_position)
  {
    // draw cross in gray
    glColor3f(0.4f, 0.4f, 0.4f);

    glBegin(GL_TRIANGLES);
      glVertex3f( -0.02f * scale,  0.005f * scale, 0.0f);
      glVertex3f( -0.02f * scale, -0.005f * scale, 0.0f);
      glVertex3f(  0.02f * scale,  0.005f * scale, 0.0f);

      glVertex3f(  0.02f * scale,  0.005f * scale, 0.0f);
      glVertex3f( -0.02f * scale, -0.005f * scale, 0.0f);
      glVertex3f(  0.02f * scale, -0.005f * scale, 0.0f);

      glVertex3f( -0.005f * scale, -0.02f * scale, 0.0f);
      glVertex3f(  0.005f * scale, -0.02f * scale, 0.0f);
      glVertex3f(  0.005f * scale,  0.02f * scale, 0.0f);

      glVertex3f(  0.005f * scale,  0.02f * scale, 0.0f);
      glVertex3f( -0.005f * scale,  0.02f * scale, 0.0f);
      glVertex3f( -0.005f * scale, -0.02f * scale, 0.0f);
    glEnd();
  }

  // plot level meter background white
  glColor3f(1.0f, 1.0f, 1.0f);

  glBegin(GL_TRIANGLE_FAN);
    glVertex3f( -0.09f * scale, -0.21f * scale, 0.0f);
    glVertex3f( -0.09f * scale, -0.24f * scale, 0.0f);
    glVertex3f(  0.09f * scale, -0.24f * scale, 0.0f);
    glVertex3f(  0.09f * scale, -0.21f * scale, 0.0f);
  glEnd();

  // plot audio level in green
  glColor3f(0.2275f, 0.9373f, 0.2275f);

  // signal_level in dB
  float signal_level = 20.0f
    * log10(source->signal_level + 0.00001f); // min -100 dB

  // only values up to 0 dB can be shown
  signal_level = std::min(signal_level, 0.0f);

  // TODO: color
  glBegin(GL_TRIANGLE_FAN);
    glVertex3f( -0.09f * scale, -0.21f * scale, 0.0f);
    glVertex3f( -0.09f * scale, -0.24f * scale, 0.0f);
    glVertex3f( -0.09f * scale + (signal_level + 50.0f)/50.0f
                * 0.18f * scale, -0.24f * scale, 0.0f);
    glVertex3f( -0.09f * scale + (signal_level + 50.0f)/50.0f
                * 0.18f * scale, -0.21f * scale, 0.0f);
  glEnd();

  // display source name
  if (_allow_displaying_text)
  {
    qglColor(_color_vector[source->id%_color_vector.size()]);
    QFont f = font();
    f.setPointSize(static_cast<int>(_zoom_factor/STDZOOMFACTOR*font().pointSize() + 0.5f));
    renderText(0.18f * scale, 0.13f * scale, 0.0f, source->name.c_str(), f);
  }

  // dummy id to prevend mouse action
  glLoadName(DUMMYINDEX);

  // plot orientation of plane wave
  if (source->model == Source::plane)
  {
    // rotate
    glRotatef(static_cast<GLfloat>(source->orientation.azimuth), 0.0f, 0.0f, 1.0f);

    if (source->mute) glColor3f(0.5f, 0.5f, 0.5f);
      else qglColor(_color_vector[source->id%_color_vector.size()]);
      
    // plot ring segments
    gluPartialDisk(_glu_quadric, 0.18f * scale, 0.19f * scale, LEVELOFDETAIL, 1, 105.0f, 75.0f);
    gluPartialDisk(_glu_quadric, 0.18f * scale, 0.19f * scale, LEVELOFDETAIL, 1,   0.0f, 90.0f);

    // plot bars
    glBegin(GL_TRIANGLE_FAN);
      glVertex3f(-0.005f * scale, -0.36f * scale, 0.0f); glVertex3f( 0.005f * scale, -0.36f * scale, 0.0f);
      glVertex3f( 0.005f * scale, -0.18f * scale, 0.0f); glVertex3f(-0.005f * scale, -0.18f * scale,  0.0f);
    glEnd();
    glBegin(GL_TRIANGLE_FAN);
      glVertex3f( 0.005f * scale, 0.36f * scale, 0.0f); glVertex3f(-0.005f * scale, 0.36f * scale, 0.0f);
      glVertex3f(-0.005f * scale, 0.18f * scale, 0.0f); glVertex3f( 0.005f * scale, 0.18f * scale, 0.0f);
    glEnd();

    // plot arrow
    glBegin(GL_TRIANGLE_FAN); // lower branch
      glVertex3f( 0.277f * scale, 0.0f, 0.0f); glVertex3f( 0.25f * scale, -0.028f * scale, 0.0f);
      glVertex3f( 0.25f * scale, -0.04f * scale, 0.0f); glVertex3f( 0.29f * scale,  0.0f, 0.0f);
    glEnd();

    glBegin(GL_TRIANGLE_FAN); // upper branch
      glVertex3f( 0.29f * scale, 0.0f, 0.0f); glVertex3f( 0.25f * scale,  0.04f * scale, 0.0f);
      glVertex3f( 0.25f * scale, 0.028f * scale, 0.0f); glVertex3f( 0.277f * scale, 0.0f, 0.0f);
    glEnd();

    glBegin(GL_TRIANGLE_FAN); // root
      glVertex3f( 0.18f * scale,  0.005f * scale, 0.0f); glVertex3f( 0.18f * scale, -0.005f * scale, 0.0f);
      glVertex3f( 0.285f * scale, -0.005f * scale, 0.0f); glVertex3f( 0.285f * scale,  0.005f * scale, 0.0f);
    glEnd();
  }
  else if (source->model == Source::directional)
  {
    // rotate
    glRotatef((GLfloat)(source->orientation.azimuth), 0.0f, 0.0f, 1.0f);

    qglColor(_color_vector[source->id%_color_vector.size()]);

    // plot ring
    gluPartialDisk(_glu_quadric, 0.18f * scale, 0.19f * scale, LEVELOFDETAIL, 1, 105.0f, 346.0f);

    // id of direction handle
    glLoadName(NAMESTACKOFFSET + index * NAMESTACKSTEP + 3);

    // plot arrow
    glBegin(GL_TRIANGLE_FAN); // lower branch
      glVertex3f( 0.277f * scale, 0.0f, 0.0f); glVertex3f( 0.25f * scale, -0.028f * scale, 0.0f);
      glVertex3f( 0.25f * scale, -0.04f * scale, 0.0f); glVertex3f( 0.29f * scale,  0.0f, 0.0f);
    glEnd();

    glBegin(GL_TRIANGLE_FAN); // upper branch
      glVertex3f( 0.29f * scale, 0.0f, 0.0f); glVertex3f( 0.25f * scale,  0.04f * scale, 0.0f);
      glVertex3f( 0.25f * scale, 0.028f * scale, 0.0f); glVertex3f( 0.277f * scale, 0.0f, 0.0f);
    glEnd();

    glBegin(GL_TRIANGLE_FAN); // root
      glVertex3f( 0.18f * scale,  0.005f * scale, 0.0f); glVertex3f( 0.18f * scale, -0.005f * scale, 0.0f);
      glVertex3f( 0.285f * scale, -0.005f * scale, 0.0f); glVertex3f( 0.285f * scale,  0.005f * scale, 0.0f);
    glEnd();
  }

}

void ssr::QOpenGLPlotter::_draw_loudspeaker(bool muted, bool active)
{
  glColor3f(0.4f, 0.4f, 0.4f);

  if (muted)
  {
    // plot muted loudspeaker
    // TODO: Make lines wider?
    glBegin(GL_LINE_STRIP);
      glVertex3f( -0.02f,  0.017f, 0.0f); glVertex3f( -0.02f, -0.017f, 0.0f);
      glVertex3f( 0.002f, -0.017f, 0.0f);
      glVertex3f( 0.024f, -0.040f, 0.0f); glVertex3f(  0.024f,  0.040f, 0.0f);
      glVertex3f( 0.002f,  0.017f, 0.0f); glVertex3f( -0.02f, 0.017f, 0.0f);
    glEnd();
  }
  else
  {
    // plot loudspeaker

    glBegin(GL_TRIANGLE_FAN); // magnet
      glVertex3f( -0.02f,  0.017f, 0.0f); glVertex3f( -0.02f, -0.017f, 0.0f);
      glVertex3f( -0.002f, -0.017f, 0.0f); glVertex3f( -0.002f,  0.017f, 0.0f);
    glEnd();

    glBegin(GL_TRIANGLE_FAN); // cone
      glVertex3f( 0.002f,  0.017f, 0.0f); glVertex3f( 0.002f, -0.017f, 0.0f);
      glVertex3f( 0.024f, -0.040f, 0.0f); glVertex3f( 0.024f,  0.040f, 0.0f);
    glEnd();

    glColor3f(1.0f, 1.0f, 1.0f);

    glBegin(GL_TRIANGLE_FAN); // ring
      glVertex3f( -0.002f,  0.017f, 0.0f); glVertex3f( -0.002f, -0.017f, 0.0f);
      glVertex3f(  0.002f, -0.017f, 0.0f); glVertex3f(  0.002f,  0.017f, 0.0f);
    glEnd();

    if (active)
    {
      // plot sound waves

      // TODO: more elegant
      if (_active_source == -1) _active_source = 0;

      qglColor(_color_vector[_id_of_last_clicked_source%_color_vector.size()]);

      gluPartialDisk(_glu_quadric, 0.05f ,0.055f, LEVELOFDETAIL, 1, 50.0f, 80.0f);
      gluPartialDisk(_glu_quadric, 0.1f , 0.105f, LEVELOFDETAIL, 1, 50.0f, 80.0f);
      gluPartialDisk(_glu_quadric, 0.15f, 0.155f, LEVELOFDETAIL, 1, 50.0f, 80.0f);
    }
  }
}

void ssr::QOpenGLPlotter::_draw_rubber_band()
{
  // check if rubber band has to be drawn
  if (_rubber_band_starting_point == _rubber_band_ending_point)
  {
    return;
  }

  glEnable(GL_COLOR_LOGIC_OP);
  glLogicOp(GL_XOR);

  glColor4f(0.4f, 0.4f, 0.4f, 0.5f);

  // draw rubber band
  glBegin(GL_LINES);
    glVertex3f(_rubber_band_starting_point.x, _rubber_band_starting_point.y, 0.0f);
    glVertex3f(_rubber_band_starting_point.x, _rubber_band_ending_point.y,   0.0f);

    glVertex3f(_rubber_band_starting_point.x, _rubber_band_ending_point.y, 0.0f);
    glVertex3f(_rubber_band_ending_point.x,   _rubber_band_ending_point.y, 0.0f);

    glVertex3f(_rubber_band_ending_point.x, _rubber_band_ending_point.y,  0.0f);
    glVertex3f(_rubber_band_ending_point.x, _rubber_band_starting_point.y,0.0f);

    glVertex3f(_rubber_band_ending_point.x,   _rubber_band_starting_point.y, 0.0f);
    glVertex3f(_rubber_band_starting_point.x, _rubber_band_starting_point.y, 0.0f);
  glEnd();

  glDisable(GL_COLOR_LOGIC_OP);

  // determine limits of rubber band
  const float upper_x = std::max(_rubber_band_starting_point.x,
                           _rubber_band_ending_point.x);
  const float lower_x = std::min(_rubber_band_starting_point.x,
                           _rubber_band_ending_point.x);

  const float upper_y = std::max(_rubber_band_starting_point.y,
                           _rubber_band_ending_point.y);
  const float lower_y = std::min(_rubber_band_starting_point.y,
                           _rubber_band_ending_point.y);

  source_buffer_list_t source_buffer_list;
  _scene.get_sources(source_buffer_list);

  int n = 1;

  // determine which sources are under the rubber band
  for (source_buffer_list_t::const_iterator i = source_buffer_list.begin();
       i != source_buffer_list.end(); i++)
  {
    // de-/select sources underneath the rubber band
    if (i->position.x > lower_x && i->position.x < upper_x &&
        i->position.y > lower_y && i->position.y < upper_y )
    {
      if (_ctrl_pressed && _alt_pressed) _deselect_source(n);
      else _select_source(n, true);
    }
    else if (!_ctrl_pressed) _deselect_source(n);

    n++;
  }
}

int ssr::QOpenGLPlotter::_find_selected_object(const QPoint &pos)
{
    const int MaxSize = 512;
    GLuint buffer[MaxSize];
    GLint viewport[4];

    // avoid error message from X server
    _allow_displaying_text = false;

    glGetIntegerv(GL_VIEWPORT, viewport);
    glSelectBuffer(MaxSize, buffer);
    glRenderMode(GL_SELECT);

    glInitNames();
    glPushName(DUMMYINDEX);

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluPickMatrix((GLdouble)pos.x(),(GLdouble)(viewport[3] - pos.y()),5.0, 5.0, viewport);
    glOrtho(-(float)width()/_zoom_factor, (float)width()/_zoom_factor,
            -(float)height()/_zoom_factor, (float)height()/_zoom_factor, 1.0f, 15.0f);

    // redraw stuff in projection mode
    paintGL();

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();

    // glPushName,glPopName ?

    // danger of error message from X server is over
    _allow_displaying_text = true;

    GLint no_of_hits = glRenderMode(GL_RENDER);

    if (no_of_hits > 0){
      // TODO: Make sure that closest object gets selected.
      return buffer[(no_of_hits-1)*4+3];
    }
    else return 0;
}


void ssr::QOpenGLPlotter::_get_openGL_pos(int x, int y, 
				     GLdouble* pos_x,
                                     GLdouble* pos_y, 
				     GLdouble* pos_z)
{
  GLint viewport[4];
  GLdouble modelview[16];
  GLdouble projection[16];
  GLfloat win_x, win_y, win_z;

  glGetDoublev(GL_MODELVIEW_MATRIX, modelview);
  glGetDoublev(GL_PROJECTION_MATRIX, projection);
  glGetIntegerv(GL_VIEWPORT, viewport );

  win_x = static_cast<GLfloat>(x);
  win_y = static_cast<GLfloat>(viewport[3]) - static_cast<GLfloat>(y);

  glReadPixels(x, static_cast<int>(win_y), 1,
               1, GL_DEPTH_COMPONENT, GL_FLOAT, &win_z);

  gluUnProject(win_x, win_y, win_z, modelview,
               projection, viewport, pos_x, pos_y, pos_z);
}

void ssr::QOpenGLPlotter::_get_pixel_pos(GLdouble pos_x, 
				    GLdouble pos_y, 
				    GLdouble pos_z,
				    int* x, int* y)
{
  GLint viewport[4];
  GLdouble modelview[16];
  GLdouble projection[16];
  GLdouble win_x, win_y, win_z;

  glGetDoublev(GL_MODELVIEW_MATRIX, modelview);
  glGetDoublev(GL_PROJECTION_MATRIX, projection);
  glGetIntegerv(GL_VIEWPORT, viewport);

  gluProject(pos_x, pos_y, pos_z, modelview,
               projection, viewport, &win_x, &win_y, &win_z);

  *x = static_cast<int>(win_x + 0.5);
  *y = static_cast<int>(viewport[3] - win_y + 0.5);

}

void ssr::QOpenGLPlotter::_set_zoom(int zoom)
{
  // limit possible zoom range
  zoom = std::max( 30, zoom);
  zoom = std::min(300, zoom);

  // update zoom factor
  _zoom_factor = STDZOOMFACTOR*zoom/100.0f;

  // update projection matrix
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();

  // TODO: Define clipping planes such that nothing is rendered behind overlays
  glOrtho(-(float)width()/_zoom_factor, (float)width()/_zoom_factor,
          -(float)height()/_zoom_factor, (float)height()/_zoom_factor, 1.0f, 15.0f);

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  emit signal_zoom_set(zoom);
}

void ssr::QOpenGLPlotter::_select_source(int source, bool add_to_selection)
{

  if (!add_to_selection) _deselect_all_sources();

  // check if listener is selected
  if (source == 0) _reference_selected = true;
  else _reference_selected = false;

  // if listener or background selected clear map
  if (source <= 0) _selected_sources_map.clear();

  // if source is not yet selected
  if (source > 0 &&
      _selected_sources_map.find(source) == _selected_sources_map.end())
  {
    source_buffer_list_t source_buffer_list;

    // get selected sources ID
    _scene.get_sources(source_buffer_list);

    // if source does not exist
    if (source > static_cast<int>(source_buffer_list.size())) return;

    source_buffer_list_t::iterator i = source_buffer_list.begin();

    // iterate to source
    for (int n = 1; n < source; n++) i++;

    // store source and its id
    _selected_sources_map[source] = i->id;

    // make its id directly available
    _id_of_last_clicked_source = i->id;
  }
  else if (!_alt_pressed)
  {
    // make valid id available
    _id_of_last_clicked_source = _selected_sources_map.find(source)->second;
  }
  // if source is already selected then deselect it
  else if (_alt_pressed) _deselect_source(source);
}

void ssr::QOpenGLPlotter::_select_all_sources()
{
  // clear variables;
  _selected_sources_map.clear();
  _id_of_last_clicked_source = 0;

  int n = 1;
  source_buffer_list_t source_buffer_list;

  // get selected sources ID
  _scene.get_sources(source_buffer_list);

  for (source_buffer_list_t::const_iterator i = source_buffer_list.begin();
       i != source_buffer_list.end(); i++)
  {
    // store source and its id
    _selected_sources_map[n] = i->id;

    // make valid id available
    _id_of_last_clicked_source = i->id;

    n++;
  }
}

void ssr::QOpenGLPlotter::_deselect_source(int source)
{
  // make sure that at least one sources stays selected
  // and that id_of_last_clicked_source has a valid value
  if (_selected_sources_map.size() > 1)
  {
    _selected_sources_map.erase(source);
    _id_of_last_clicked_source = _selected_sources_map.begin()->second;
  }
}

void ssr::QOpenGLPlotter::_deselect_all_sources()
{
  _selected_sources_map.clear();
  _id_of_last_clicked_source = 0;
}

// Settings for Vim (http://www.vim.org/), please do not remove:
// vim:softtabstop=2:shiftwidth=2:expandtab:textwidth=80:cindent
// vim:fdm=expr:foldexpr=getline(v\:lnum)=~'/\\*\\*'&&getline(v\:lnum)!~'\\*\\*/'?'a1'\:getline(v\:lnum)=~'\\*\\*/'&&getline(v\:lnum)!~'/\\*\\*'?'s1'\:'='
