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
/// Helper functions for filename and path manipulation.

#ifndef SSR_PATHTOOLS_H
#define SSR_PATHTOOLS_H

#include <filesystem>

namespace fs = std::filesystem;

/** helper functions for filename and path manipulation.
 **/
namespace pathtools
{

/** Remove common directory part and prepend "../" if necessary.
 * If @p path is in the directory of @p filename or below, remove directory
 * from @p path. For each further sub-directories of @p filename, prepend
 * "../" to @p path. If they have no common directories, prepend an
 * appropriate number of parent directories and "../"s.
 * @param path A path given relative to the current directory.
 *   If @p path is absolute or empty, it is not changed. If @p path is
 *   relative, the result is also relative.
 * @param filename This can also be a directory, but then it has to end with a
 *   trailing slash.
 *   If @p filename doesn't have a directory part, @p path is not changed.
 **/
inline fs::path make_path_relative_to_file(const std::string& path
    , const std::string& filename)
{
  auto p = fs::path{path};
  if (p.is_absolute() || p == "")
  {
    return p;
  }
  p = fs::absolute(p);
  return fs::relative(p, fs::absolute(filename).parent_path());
}

/** Prepend directory part of one path to another path.
 * @param path A path given relative to @p filename.
 *   If @p path is absolute or empty, it is not changed.
 *   If @p path is relative, the result is also relative.
 * @param filename The directory part of @p filename is prepended to @p path.
 * @return @p path prepended with the directory name of @p filename,
 *   relative to the current directory.
 **/
inline std::string make_path_relative_to_current_dir(const std::string& path
    , const std::string& filename)
{
  auto p = fs::path{path};
  if (p.is_absolute() || p == "")
  {
    return p.string();
  }
  return fs::relative(fs::absolute(filename).parent_path() / p).string();
}

/** Insert escape characters (\) before whitespace characters.
 * @param filename the file name
 * @return the file name with escaped whitespace characters
 **/
inline std::string get_escaped_filename(const std::string& filename)
{
  std::string escaped_filename;

  for (const auto ch: filename)
  {
    if (isspace(ch))
    {
      escaped_filename.append(1, '\\');
    }
    escaped_filename.append(1, ch);
  }
  return escaped_filename;
}

}  // namespace pathtools

#endif
