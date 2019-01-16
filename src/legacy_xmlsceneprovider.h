// For compatibility with old network interface

#ifndef SSR_LEGACY_XMLSCENEPROVIDER_H
#define SSR_LEGACY_XMLSCENEPROVIDER_H

namespace ssr
{

struct LegacyXmlSceneProvider
{
  virtual ~LegacyXmlSceneProvider() = default;

  virtual std::string get_scene_as_XML() const = 0;
};

}  // namespace ssr

#endif
