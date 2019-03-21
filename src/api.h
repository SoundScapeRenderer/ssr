/******************************************************************************
 * Copyright © 2019 SSR Contributors                                          *
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
/// API definition for all functions of the SSR.

#ifndef SSR_API_H
#define SSR_API_H

#include <cstdint>  // for uint32_t
#include <memory>  // for std::unique_ptr
#include <string>
#include <utility>  // for std::pair
#include <vector>

namespace ssr
{

using id_t = const std::string&;

/// 3D vector type for positions and translations.  @see vec3, Rot
struct Pos { float x{}, y{}, z{}; };
/// Quaternion type for rotations.  @see quat, Pos
struct Rot { float x{}, y{}, z{}, w{1}; };

/// Information about a single loudspeaker.
struct Loudspeaker
{
  Pos position;
  Rot rotation;

  /// Type of loudspeaker.  Normal loudspeakers use the empty string @c "",
  /// subwoofers use @c "subwoofer".
  /// Other types might be supported by some renderers.
  std::string model;
};

/// Interface for controlling scene and renderer.
/// None of the classes/functions here is thread-safe, i.e. it is not allowed to
/// access/call them from different threads at the same time.
/// The Publisher::take_control() and Publisher::update_follower() functions
/// take exclusive control over the SSR instance, by acquiring a mutex.
namespace api
{

/// Bundle start/stop events.  Bundles typically contain one or more messages
/// that are supposed to happen at the same time.
/// @see SubscribeHelper::bundle()
struct BundleEvents
{
  virtual ~BundleEvents() = default;

  /// A bundle has been started.  For example by Publisher::take_control() or
  /// Publisher::update_follower().
  virtual void bundle_start() = 0;

  /// A bundle has been finished.
  virtual void bundle_stop() = 0;
};


/// Subscribable scene properties that can be changed by clients.
/// @see SubscribeHelper::scene_control()
struct SceneControlEvents
{
  virtual ~SceneControlEvents() = default;

  /// Orient sources towards the reference point ... or not.
  /// @todo additional per-source setting?
  virtual void auto_rotate_sources(bool auto_rotate) = 0;

  /// Remove a source from the Scene.
  /// @param id ID of the source
  virtual void delete_source(id_t id) = 0;

  /// Set source position.
  /// @param id ID of the source
  /// @param position new position
  virtual void source_position(id_t id, const Pos& position) = 0;

  /// Set source rotation/orientation.
  /// @param id ID of the source
  /// @param rotation new rotation
  virtual void source_rotation(id_t id, const Rot& rotation) = 0;

  /// Set source volume.
  /// @param id ID of the source
  /// @param volume new volume (linear)
  virtual void source_volume(id_t id, float volume) = 0;

  /// Mute/unmute source.
  /// @param id ID of the source
  /// @param mute mute if @b true, unmute if @b false
  virtual void source_mute(id_t id, bool mute) = 0;

  /// Set source name.
  /// @param id ID of the source
  /// @param name new name
  virtual void source_name(id_t id, const std::string& name) = 0;

  /// Set source model (=type).  This is typically @c "point" or @c "plane",
  /// but certain renderers might support more models.
  /// @param id ID of the source
  /// @param model new model
  virtual void source_model(id_t id, const std::string& model) = 0;

  /// Set whether source position/rotation is fixed.
  /// @param id ID of the source
  /// @param fixed whether position/rotation is fixed or not
  virtual void source_fixed(id_t id, bool fixed) = 0;

  /// Set reference position.
  /// @param position new position
  virtual void reference_position(const Pos& position) = 0;

  /// Set reference rotation/orientation.
  /// @param rotation new rotation
  virtual void reference_rotation(const Rot& rotation) = 0;

  /// Set master volume.
  /// @param volume new volume (linear)
  virtual void master_volume(float volume) = 0;

  /// Set amplitude decay exponent.
  /// @param exponent amplitude decay exponent
  /// @todo per source setting?
  virtual void decay_exponent(float exponent) = 0;

  /// Set amplitude reference distance.
  /// @param distance amplitude reference distance
  virtual void amplitude_reference_distance(float distance) = 0;
};


/// Subscribable scene information that cannot be directly changed by clients.
/// @see SubscribeHelper::scene_information()
struct SceneInformationEvents
{
  virtual ~SceneInformationEvents() = default;

  /// Sampling rate of the scene/transport in Hertz.
  /// This is needed to convert api::TransportFrameEvents::transport_frame()
  /// and audio file durations to seconds.
  virtual void sample_rate(int rate) = 0;

  /// Publish creation of a new source.
  /// @param id ID of the new source
  virtual void new_source(id_t id) = 0;

  /// Set immutable properties (string-to-string mapping) of a (new) source,
  /// e.g. @c audio-file, @c audio-file-channel, @c audio-file-length,
  /// @c port-name, @c properties-file.
  virtual void source_property(id_t id, const std::string& key
                                      , const std::string& value) = 0;

  /// Whether the JACK transport is rolling or not.
  virtual void transport_rolling(bool rolling) = 0;
};


/// Subscribable renderer properties that can be changed by clients.
/// @see SubscribeHelper::renderer_control()
struct RendererControlEvents
{
  virtual ~RendererControlEvents() = default;

  /// Switch processing on/off
  virtual void processing(bool processing_state) = 0;

  /// Renderer-specific position offset.
  /// Relative to SceneControlEvents::reference_position().
  virtual void reference_position_offset(const Pos& position) = 0;

  /// Renderer-specific rotation offset.
  /// Relative to SceneControlEvents::reference_rotation().
  /// This is typically controlled by a head tracker.
  virtual void reference_rotation_offset(const Rot& rotation) = 0;
};


/// Subscribable renderer properties that cannot be changed by clients.
/// @see SubscribeHelper::renderer_information()
struct RendererInformationEvents
{
  virtual ~RendererInformationEvents() = default;

  /// Name of the renderer.
  virtual void renderer_name(const std::string& name) = 0;

  /// List of loudspeakers.  Doesn't change during the lifetime of a renderer.
  virtual void loudspeakers(const std::vector<Loudspeaker>& loudspeakers) = 0;
};


/// Continuous updates about the current JACK transport position.
/// This is part of the scene information but because of the high frequency of
/// messages this can be subscribed separately.
/// @see SubscribeHelper::transport_frame()
struct TransportFrameEvents
{
  virtual ~TransportFrameEvents() = default;

  /// Update transport frame.
  /// Whether or not the transport is "rolling" can be found out with
  /// api::SceneInformationEvents::transport_rolling().
  /// @param frame current transport time in frames.
  ///   Use api::SceneInformationEvents::sample_rate() to convert to seconds.
  /// @see http://www.jackaudio.org/files/docs/html/group__TransportControl.html
  virtual void transport_frame(uint32_t frame) = 0;
};


/// Continuous updates about current source signal levels.
/// This is part of the scene information but because of the high frequency of
/// messages this can be subscribed separately.
/// @see SubscribeHelper::source_metering()
struct SourceMetering
{
  virtual ~SourceMetering() = default;

  /// Instantaneous level of source signal.
  /// @param id ID of the source
  /// @param level level (linear scale)
  virtual void source_level(id_t id, float level) = 0;
};


/// Continuous updates about the current master signal level.
/// This is part of the renderer information but because of the high frequency
/// of messages this can be subscribed separately.
/// @see SubscribeHelper::master_metering()
struct MasterMetering
{
  virtual ~MasterMetering() = default;

  /// Combined (= maximum) instantaneous level of all outputs.
  /// @param level level (linear scale)
  virtual void master_level(float level) = 0;
};


/// Continuous updates about output activity, per source.
/// This is part of the renderer information but because of the high frequency
/// of messages this can be subscribed separately.
/// @see SubscribeHelper::output_activity()
struct OutputActivity
{
  virtual ~OutputActivity() = default;

  /// Influence of a given source on each output (as a linear scaling factor).
  /// @param id ID of the source
  /// @param begin Pointer to first activity
  /// @param end Past-the-end pointer
  virtual void output_activity(id_t id, float* begin, float* end) = 0;
};


/// Continuous updates about the renderer's CPU load (as reported by JACK).
/// This is part of the renderer information but because of the high frequency
/// of messages this can be subscribed separately.
/// @see SubscribeHelper::cpu_load()
struct CpuLoad
{
  virtual ~CpuLoad() = default;

  /// CPU load.
  /// @param load CPU load in percent
  virtual void cpu_load(float load) = 0;
};


/// Interface for controlling an SSR instance.
/// Its renderer can be controlled with RendererControlEvents.
/// If the instance is itself a "follower", the SceneControlEvents are
/// forwarded to the connected "leader" instance.
/// Control over an SSR instance can be obtained with Publisher::take_control().
struct Controller : virtual SceneControlEvents
                  , RendererControlEvents
{
  /// Load a scene from a file.
  virtual void load_scene(const std::string& filename) = 0;

  /// Save a scene to a file.
  virtual void save_scene(const std::string& filename) const = 0;

  /// Create a new source.
  /// @param id Unique identifier of the source.  If a source with that ID
  ///   already exists, the source will not be created.  If an empty string is
  ///   given, a unique ID will be auto-generated.
  /// @param name Name of the new source.  This doesn't have to be unique and it
  ///   can change during the existence of the source.
  /// @param model Model (=type) of the new source.  Default is @c "point".
  /// @param file_or_port_name Name of audio file or JACK port.
  ///   If a JACK port is given, @a channel has to be zero!
  /// @param channel number of channel in audio file.
  ///   If zero, a JACK port is given instead of an audio file.
  /// @param position Initial position of the new source.
  /// @param rotation Initial rotation of the new source.
  /// @param fixed Whether the source can be moved or not.
  /// @param volume Initial (linear) volume of the new source.
  /// @param mute Initial mute state.
  /// @param properties_file Renderer-specific additional file, e.g. containing
  ///   impulse responses.  Use empty string if not needed.
  virtual void new_source(const std::string& id, const std::string& name
      , const std::string& model, const std::string& file_name_or_port_number
      , int channel, const Pos& position, const Rot& rotation, bool fixed
      , float volume, bool mute, const std::string& properties_file) = 0;

  /// Delete all sources.
  virtual void delete_all_sources() = 0;

  /// Start JACK transport.  @see TransportFrameEvents
  virtual void transport_start() = 0;

  /// Stop JACK transport.  @see TransportFrameEvents
  virtual void transport_stop() = 0;

  /// Skip the scene to a specified instant of time.
  /// @param time instant of time in frames.
  ///   Use api::SceneInformationEvents::sample_rate() to convert from seconds.
  /// @see TransportFrameEvents
  virtual void transport_locate_frames(uint32_t time) = 0;

  /// Skip the scene to a specified instant of time.
  /// @param time instant of time in seconds.
  ///   Use api::SceneInformationEvents::sample_rate() to convert from frames.
  /// @see TransportFrameEvents
  virtual void transport_locate_seconds(float time) = 0;

  /// Reset the tracker (if one is connected).
  /// @todo There should probably be a more fancy tracker interface ...?
  virtual void reset_tracker() = 0;

  /// Get string ID given one-based source number.
  /// If the source number is @c 0 or higher than the number of sources, an
  /// empty string is returned.
  ///
  /// This is meant to help implementing clients that don't track source IDs.
  /// If they (or their human operator) have a-priori knowledge about the
  /// current number of sources, this function can be used to get the
  /// corresponding IDs.
  /// @note The source numbers change when sources are removed!
  /// @see Publisher::get_source_number()
  virtual std::string get_source_id(unsigned int source_number) const = 0;
};


/// Messages sent from the "leader" instance to a "follower".
/// Control over a "follower" can be obtained with Publisher::update_follower().
struct Follower : virtual SceneControlEvents
                , SceneInformationEvents
                , TransportFrameEvents
                , SourceMetering
{};


/// A subscription to a group of events.
/// Nothing interesting can be done with a Subscription object, but when it is
/// destroyed, the corresponding subscription is automatically cancelled.
/// @see Publisher
struct Subscription
{
  virtual ~Subscription() = default;
};


/// Return type of Publisher::subscribe().
/// An object of this class has exclusive control over the SSR instance until
/// the object is destroyed.  Use it like this for a single subscription:
///                                                                        @code
/// mysubscription = mypublisher.subscribe()->cpu_load(mysubscriber);
///                                                                     @endcode
/// ... or in its own scope for multiple subscriptions:
///                                                                        @code
/// {
///   auto subscribe = mypublisher.subscribe();
///   mysubscription1 = subscribe->transport_frame(mysubscriber);
///   mysubscription2 = subscribe->master_metering(mysubscriber);
/// }
///                                                                     @endcode
/// A given subscription is automatically cancelled when the corresponding
/// Subscription object is destroyed.
/// @note A subscriber must outlive its subscription.  This can be easily ensured
///   by binding the returned Subscription to the scope of the subscriber,
///   because the subscriber is automatically unsubscribed when the subscription
///   object goes out of scope.
/// @warning Subscribing again to an already subscribed class of events is not
///   allowed and an exception will be raised if it is attempted.
struct SubscribeHelper
{
  virtual ~SubscribeHelper() = default;

  /// Subscribe to BundleEvents.
  virtual std::unique_ptr<Subscription> bundle(
      BundleEvents* subscriber) = 0;
  /// Subscribe to SceneControlEvents.
  /// Additionally, this sends the current state to the @a subscriber.
  virtual std::unique_ptr<Subscription> scene_control(
      SceneControlEvents* subscriber) = 0;
  /// Subscribe to SceneInformationEvents.
  /// Additionally, this sends the current state to the @a subscriber.
  virtual std::unique_ptr<Subscription> scene_information(
      SceneInformationEvents* subscriber) = 0;
  /// Subscribe to RendererControlEvents.
  /// Additionally, this sends the current state to the @a subscriber.
  virtual std::unique_ptr<Subscription> renderer_control(
      RendererControlEvents* subscriber) = 0;
  /// Subscribe to RendererInformationEvents.
  /// Additionally, this sends the current state to the @a subscriber.
  virtual std::unique_ptr<Subscription> renderer_information(
      RendererInformationEvents* subscriber) = 0;
  /// Subscribe to TransportFrameEvents.
  virtual std::unique_ptr<Subscription> transport_frame(
      TransportFrameEvents* subscriber) = 0;
  /// Subscribe to SourceMetering.
  virtual std::unique_ptr<Subscription> source_metering(
      SourceMetering* subscriber) = 0;
  /// Subscribe to MasterMetering.
  virtual std::unique_ptr<Subscription> master_metering(
      MasterMetering* subscriber) = 0;
  /// Subscribe to OutputActivity.
  virtual std::unique_ptr<Subscription> output_activity(
      OutputActivity* subscriber) = 0;
  /// Subscribe to CpuLoad.
  virtual std::unique_ptr<Subscription> cpu_load(
      CpuLoad* subscriber) = 0;
};


/// Handle subscriptions, allow controlling the scene/renderer.
struct Publisher
{
  virtual ~Publisher() = default;

  /// Prepare to make subscriptions.  The returned SubscribeHelper object can be
  /// used to make the actual subscriptions.  The SSR instance will remain
  /// locked until the returned object is destroyed.
  /// @warning While the SSR instance is locked, calls to attach_leader(),
  ///   take_control() and update_follower() will not complete, neither will any
  ///   attempts to unsubscribe previous subscriptions.
  virtual std::unique_ptr<SubscribeHelper> subscribe() = 0;

  /// Connect to "leader" (if this Publisher is a "follower").
  /// The "leader" is automatically un-attached when the returned Subscription
  /// object is destroyed.
  virtual std::unique_ptr<Subscription> attach_leader(Controller* leader) = 0;

  /// Obtain exclusive control over this Publisher's SSR instance.
  /// This automatically starts a "bundle", see BundleEvents.
  /// When the returned object is destroyed, the "bundle" is closed and
  /// exclusive control is automatically released.
  /// Use it like this for a single event:
  ///                                                                      @code
  /// mypublisher.take_control()->transport_locate(0.0f);
  ///                                                                   @endcode
  /// ... or in its own scope for multiple events:
  ///                                                                      @code
  /// {
  ///   auto control = mypublisher.take_control();
  ///   control->master_volume(0.8);
  ///   control->processing(true);
  /// }
  ///                                                                   @endcode
  /// @param suppress_own The generated SceneControlEvents are not forwarded to
  ///   the given subscriber.
  virtual std::unique_ptr<Controller> take_control(
      SceneControlEvents* suppress_own) = 0;
  /// @overload
  virtual std::unique_ptr<Controller> take_control() = 0;

  /// Obtain exclusive control over this Publisher's SSR instance, which is
  /// supposed to be a "follower".
  /// This automatically starts a "bundle", see BundleEvents.
  /// When the returned object is destroyed, the "bundle" is closed and
  /// exclusive control is automatically released.
  virtual std::unique_ptr<Follower> update_follower() = 0;

  /// Get the one-based source number given a string ID.
  /// If the given ID doesn't exist, @c 0 is returned.
  /// @note The source numbers change when sources are removed!
  /// @see Controller::get_source_id()
  virtual unsigned int get_source_number(id_t source_id) const = 0;
};

}  // namespace api

}  // namespace ssr

#endif
