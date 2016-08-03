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

#ifndef SSR_QOPENGLPLOTTER_H
#define SSR_QOPENGLPLOTTER_H

#ifdef __APPLE__
#include <OpenGL/glu.h>
#else
#include <GL/glu.h>
#endif

#include <QtOpenGL/QGLWidget>
#include <QtWidgets/QAction>
#include <QtCore/QString>
#include <QtWidgets/QWidget>
#include <QtGui/QImage>
#include <QtGui/QMouseEvent>
#include <QtGui/QKeyEvent>
#include <QtGui/QCloseEvent>
#include <QtWidgets/QLabel>
#include <vector>
#include <list>
#include <map>
#include <set>

#include "publisher.h"
#include "scene.h"
#include "qsourceproperties.h"

#define STDZOOMFACTOR 280.0f
#define STDWINDOWYOFFSET -1.0f
#define VOLUMEACCELERATION 0.3f

#define DUMMYINDEX 0
#define BACKGROUNDINDEX 1
#define REFERENCEINDEX1 2
#define REFERENCEINDEX2 3
#define SSRLOGOINDEX 4

#define NAMESTACKOFFSET 6
#define NAMESTACKSTEP 5

/* Info about name stack:
 *
 * dummy (to prevent mouse action):  0
 * background:                       1
 * reference handle for translation: 2
 * reference handle for rotation:    3
 * SSR logo:                         4
 * 
 * source:                          NAMESTACKOFFSET + index_of_source      * NAMESTACKSTEP + 1
 * source volume slider:            NAMESTACKOFFSET + index_of_source      * NAMESTACKSTEP + 2 
 * source direction handle:         NAMESTACKOFFSET + index_of_source      * NAMESTACKSTEP + 3
 * loudspeaker:                     NAMESTACKOFFSET + index_of_loudspeaker * NAMESTACKSTEP + 4
 * 
 */

namespace ssr
{

/// open GL plotter
class QOpenGLPlotter : public QGLWidget
{
  Q_OBJECT

    // TODO: Discriminate between GLfloat and float etc.

  protected:
    struct SourceCopy;      // nested class, defined later

////////////////////////////////////////////////////////////////////////////////
// Declaration of the nested class SourceCopy
////////////////////////////////////////////////////////////////////////////////

    /** Temporary buffer for source information.
     * This class is used to extract certain information for each source from
     * the Scene. There is no use in copying data which are not used afterwards.
     **/
    struct SourceCopy : DirectionalPoint
    {
      /// SourceCopies want to be stored in such a list
      typedef std::list<SourceCopy> list_t;
      /// type conversion constructor
      SourceCopy(const Scene::source_map_t::value_type& other);

      ssr::id_t id;       ///< identifier
      Source::model_t model; ///< source model
      bool mute;           ///< mute state 
      float gain;            ///< source gain
      float signal_level;   ///< level of audio stream (linear, between 0 and 1)
      std::vector<float> output_levels;
      std::string name;       ///<source name
      bool fixed_position;
    };

  public:
    QOpenGLPlotter(Publisher& controller, const Scene& scene
        , const std::string& path_to_gui_images
        , QWidget *parent = 0);
    virtual ~QOpenGLPlotter();

    typedef QOpenGLPlotter::SourceCopy::list_t source_buffer_list_t;

    // type for vector with possible source colors
    typedef std::vector<QColor> color_vector_t;

  protected:
    Publisher& _controller;
    const Scene& _scene;
    int _active_source;
    const std::string _path_to_gui_images;

    ssr::id_t _id_of_last_clicked_source;

    typedef std::map<int, ssr::id_t> selected_sources_map_t;
    selected_sources_map_t _selected_sources_map;

    float _zoom_factor;

    std::set<ssr::id_t> _soloed_sources;

    //  void  mousePressEvent(QMouseEvent *event);
    QMouseEvent _previous_mouse_event;
    bool   _ctrl_pressed;
    bool   _alt_pressed;

    int    _find_selected_object(const QPoint &pos);
    void   _get_openGL_pos(int x, int y, 
			   GLdouble* posX,
			   GLdouble* posY,
			   GLdouble* posZ);
    void   _get_pixel_pos(GLdouble pos_x, 
			  GLdouble pos_y, 
			  GLdouble pos_z, 
			  int* x, int* y);

    Position _rubber_band_starting_point;
    Position _rubber_band_ending_point;

    GLfloat _window_x_offset, _window_y_offset, _window_z_offset;
    GLfloat _x_offset, _y_offset, _z_offset;
    GLfloat _angle_offset;
    //Orientation orientation_offset;
    GLfloat _rotation_x;
    GLfloat _rotation_y;
    GLfloat _rotation_z;
    bool    _reference_selected;
    bool    _volume_slider_selected;
    bool    _direction_handle_selected;

    void _select_source(int source, bool add_to_selection = false);
    void _select_all_sources();
    void _deselect_source(int source);
    void _deselect_all_sources();

  private:
    GLuint _ssr_logo_texture;
    GLuint _source_shadow_texture;
    GLuint _listener_texture;
    GLuint _listener_shadow_texture;
    GLuint _listener_background_texture;

    Loudspeaker::container_t _loudspeakers;

    color_vector_t _color_vector;

    bool _allow_displaying_text;
    /// Quadric necessary to plot spheres and disks 
    GLUquadricObj *_glu_quadric;

    bool _plot_listener;

    // OpenGL functions
    void  initializeGL();
    void  resizeGL(int width, int height);
    void  paintGL();

    // drawing functions
    void _draw_background();
    void _draw_objects();
    void _draw_source(source_buffer_list_t::const_iterator& source, 
		      unsigned int index);
    void _draw_loudspeaker(bool muted = false, bool active = false);
    void _draw_rubber_band();
    void _draw_reference();

    void _load_background_textures();

    protected slots:
      void _set_zoom(int zoom);

  signals:
    void signal_zoom_set(int zoom);
};

}  // namespace ssr

#endif

// Settings for Vim (http://www.vim.org/), please do not remove:
// vim:softtabstop=2:shiftwidth=2:expandtab:textwidth=80:cindent
// vim:fdm=expr:foldexpr=getline(v\:lnum)=~'/\\*\\*'&&getline(v\:lnum)!~'\\*\\*/'?'a1'\:getline(v\:lnum)=~'\\*\\*/'&&getline(v\:lnum)!~'/\\*\\*'?'s1'\:'='
