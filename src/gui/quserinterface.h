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
/// QUserInterface class (definition).

#ifndef SSR_QUSERINTERFACE_H
#define SSR_QUSERINTERFACE_H

#ifdef HAVE_CONFIG_H
#include <config.h>  // for ENABLE_FLOATING_CONTROL_PANEL
#endif

#include <QtCore/QObject>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>

#include "qopenglplotter.h"
#include "qclicktextlabel.h"
#include "qguiframe.h"
#include "qfilemenulabel.h"
#include "qcpulabel.h"
#include "qzoomlabel.h"
#include "qvolumeslider.h"
#include "qscenebutton.h"
#include "qssrtimeline.h"
#include "qsourceproperties.h"

namespace ssr
{

/** This class completes the OpenGL window and makes it a proper interface
 * including handling of mouse and keyboard actions and other interaction tools.
 */
class QUserInterface : public QOpenGLPlotter
{
  Q_OBJECT

  public:
    QUserInterface(Publisher& controller, const Scene& scene
        , const std::string& path_to_gui_images
        , const std::string& path_to_scene_menu
        , unsigned int update_frequency = 30u, QWidget *parent = 0);
    ~QUserInterface();

    // list with scene labels
    typedef std::list<QSceneButton *> scene_button_list_t;

  private slots:
    void _update_screen();
    virtual void _create_scene_menu(const std::string& path_to_scene_menu);
    virtual void _open_file();
    virtual void _new_scene();
    virtual void _set_master_volume( float volume );
    virtual void _change_volume_of_selected_sources(float d_volume);
    virtual void _save_file_as();
    virtual void _load_scene(const QString& scene_name = QString());
    virtual void _skip_back();
    virtual void _show_file_menu();
    virtual void _processing_button_pressed();
    virtual void _pause_button_pressed();
    virtual void _transport_locate(float time);
    virtual void _play_button_pressed();
    virtual void _set_source_mute(const bool flag);
    virtual void _set_source_position_fixed(const bool flag);
    virtual void _set_source_model(const int index);
    virtual void _resizeControls(int newWidth);

  protected:
    std::string scene_description_file; ///< path to current scene descriptinon file (obsolete???)
    uint _active_scene;       ///< number of the quick access tab of the current scene
#ifndef ENABLE_FLOATING_CONTROL_PANEL
    bool _mouse_event_out_of_scope(QMouseEvent *event);
    virtual bool event(QEvent *event);
#else
    bool eventFilter(QObject *sender, QEvent *event);
#endif
    virtual void resizeEvent(QResizeEvent *event);
    virtual void keyPressEvent(QKeyEvent *event);
    virtual void keyReleaseEvent(QKeyEvent *event);
    virtual void mousePressEvent(QMouseEvent *event);
    virtual void mouseMoveEvent(QMouseEvent *event);
    virtual void mouseDoubleClickEvent(QMouseEvent *event);
    virtual void mouseReleaseEvent (QMouseEvent *event);
    virtual void wheelEvent(QWheelEvent * event);

  private:
    scene_button_list_t _scene_button_list; ///< list which holds all quiack access scene tabs

    // Fancy stuff
    QGUIFrame* _frame;                ///< frame around the OpenGL window
    QFileMenuLabel* _file_menu_label; ///< label that holds the file menu
    QCPULabel* _cpu_label;            ///< label that displays the current CPU load
    QZoomLabel* _zoom_label;          ///< zoom action and display label
    QVolumeSlider* _volume_slider;    ///< master volume and master audio level widget
    QSSRTimeLine* _time_line;

    // Text labels
    QLabel* _cpu_label_text_tag;      ///< text tag above \a cpu_label
    QLabel* _zoom_label_text_tag;     ///< text tag above \a zoom_label
    QLabel* _volume_slider_text_tag;  ///< text tag above \a volume_slider

    // Buttons
    QPushButton* _processing_button;  ///< button to enable/disable audio processing
    QPushButton* _skip_back_button;   ///< button to skip to the beginning of a scene
    QPushButton* _pause_button;       ///< button to pause replaying
    QPushButton* _play_button;        ///< button to start replaying

    bool _ignore_mouse_events;

    QSourceProperties* _source_properties;  ///< source properties dialog

    void _show_about_window();
    void _update_source_properties_position();
    void _toggle_mute_state_of_selected_sources();
    void _toggle_solo_state_of_selected_sources();
    void _toggle_source_models();
    void _toggle_fixation_state_of_selected_sources();
    void _solo_selected_sources();
    void _unsolo_selected_sources();
    void _unsolo_all_sources();
  
    QWidget *_controlsParent;  ///< parent widget of buttons and labels
};

}  // namespace ssr

#endif

// Settings for Vim (http://www.vim.org/), please do not remove:
// vim:softtabstop=2:shiftwidth=2:expandtab:textwidth=80:cindent
// vim:fdm=expr:foldexpr=getline(v\:lnum)=~'/\\*\\*'&&getline(v\:lnum)!~'\\*\\*/'?'a1'\:getline(v\:lnum)=~'\\*\\*/'&&getline(v\:lnum)!~'/\\*\\*'?'s1'\:'='
