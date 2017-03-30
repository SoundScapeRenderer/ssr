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
/// Helper functions for UNIX-style filename and path manipulation.

#ifndef SSR_POSIXPATHTOOLS_H
#define SSR_POSIXPATHTOOLS_H

#include <string>
#include <list>
#include <sstream>  // for istringstream
#include <iterator> // for ostream_iterator
#include <cstring>  // for strcmp()
#include <fcntl.h>  // for O_RDONLY, dev_t, ino_t
#include <dirent.h> // for DIR, opendir()
#include <cassert>  // for assert()
#include <sys/stat.h> // for stat and mkdir (on Mac OSX)
#include <unistd.h>  // for chdir(), fchdir(), close()

/** helper functions for filename and path manipulation.
 *
 * @warning Code in this file is highly platform dependent!
 *
 * @todo Maybe in the future other solutions should be considered, e.g. using
 * boost.filesystem or using a totally different language (like Python) for such
 * things.
 **/
namespace posixpathtools
{

/** Get current working directory.
 * @param[out] path the cwd
 * @return @b true on success
 *
 * This is stolen from
 * http://insanecoding.blogspot.com/2007/11/pathmax-simply-isnt.html
 * @author http://insanecoding.blogspot.com/
 *
 * Contrary to the original, this version doesn't add a slash at the end.
 *
 * @warning This function may not work on all platforms!
 **/
inline bool getcwd(std::string& path)
{
  using file_id = std::pair<dev_t, ino_t>;

  bool success = false;
  // Keep track of start directory, so can jump back to it later
  auto start_fd = open(".", O_RDONLY);
  if (start_fd != -1)
  {
    struct stat sb;
    if (!fstat(start_fd, &sb))
    {
      auto current_id = file_id(sb.st_dev, sb.st_ino);
      // Get info for root directory, so we can determine when we hit it
      if (!stat("/", &sb))
      {
        auto path_components = std::list<std::string>();
        file_id root_id(sb.st_dev, sb.st_ino);

        // If they're equal, we've obtained enough info to build the path
        while (current_id != root_id)
        {
          bool pushed = false;

          if (!chdir("..")) //Keep recursing towards root each iteration
          {
            auto dir = opendir(".");
            if (dir)
            {
              dirent* entry;
              // We loop through each entry trying to find where we came from
              while ((entry = readdir(dir)))
              {
                if (strcmp(entry->d_name, ".")
                    && strcmp(entry->d_name, "..")
                    && !lstat(entry->d_name, &sb))
                {
                  auto child_id = file_id(sb.st_dev, sb.st_ino);
                  // We found where we came from, add its name to the list
                  if (child_id == current_id)
                  {
                    path_components.push_back(entry->d_name);
                    pushed = true;
                    break;
                  }
                }
              }
              closedir(dir);

              // If we have a reason to contiue, we update the current dir id
              if (pushed && !stat(".", &sb))
              {
                current_id = file_id(sb.st_dev, sb.st_ino);
              }
            } // Else, Uh oh, can't read information at this level
          }
          // If we didn't obtain any info this pass, no reason to continue
          if (!pushed) break;
        }

        if (current_id == root_id) // Unless they're equal, we failed above
        {
          path = "";
          for (std::list<std::string>::reverse_iterator i
              = path_components.rbegin(); i != path_components.rend(); ++i)
          {
            path += "/" + *i;
          }
          if (path == "") path = "/";
          success = true;
        }
        fchdir(start_fd);
      }
    }
    close(start_fd);
  }
  return success;
}

/** Turn a path into a sequence of path components.
 * If @p path is absolute (i.e. it starts with a slash), the first element of
 * @p tokens will be an empty string.
 * If @p path has a trailing slash, the last element of @p tokens will be an
 * empty string. A trailing slash can be used to distinguish between files and
 * directories.
 **/
template<typename T>
void tokenize(const std::string& path, T& tokens)
{
  // add another slash to get an empty string if there is a trailing slash
  std::istringstream ss(path + "/");
  std::string s;
  while (std::getline(ss, s, '/')) tokens.push_back(s);
}

/** Turn a sequence of path components into a path.
 * @param tokens a list of path components.
 * @return The string containing the path. If @p tokens is an empty list or if
 * @p tokens consists of one empty string (and nothing else), "." is returned.
 * @see tokenize()
 **/
template<typename T>
std::string untokenize(const T& tokens)
{
  std::ostringstream oss;

  std::copy(tokens.begin(), tokens.end()
      , std::ostream_iterator<std::string>(oss, "/"));
  // this always adds a trailing slash, which we don't want

  auto result = oss.str();

  if (!result.empty())
  {
    // remove trailing slash
    result.erase(result.end()-1);
  }

  // if it's empty now, this means we are dealing with the current directory
  if (result.empty())
  {
    result = ".";
  }

  return result;
}

/** Remove "." and ".." from a list of path components.
 * @note If there are ".."s at the beginning of a relative path, they are not
 *   removed.
 **/
template<typename T>
void clean_path(T& tokens)
{
  if (tokens.empty()) return; // nothing to do.

  // check if path is absolute, remove the first empty element temporarily
  bool absolute = false;
  if (!tokens.empty() && tokens.front() == "")
  {
    absolute = true;
    tokens.pop_front();
  }
  // check for trailing slash, remove the last empty element temporarily
  bool directory = false;
  if (!tokens.empty() && tokens.back() == "")
  {
    directory = true;
    tokens.pop_back();
  }

  auto it = tokens.rbegin();

  int levels = 0;

  while (it != tokens.rend())
  {
    // erasing reverse iterators is tricky,
    // see http://www.drdobbs.com/cpp/184401406
    auto fwd_it = it.base();
    --fwd_it;

    if (*it == ".")
    {
      tokens.erase(fwd_it);
    }
    else if (*it == "..")
    {
      ++levels;
      tokens.erase(fwd_it);
    }
    else if (*it == "") // this should normally not happen, but who knows?
    {
      tokens.erase(fwd_it);
    }
    else if (levels > 0)
    {
      tokens.erase(fwd_it);
      --levels;
    }
    else
    {
      // do nothing, except
      ++it;
    }
  }

  // if there are still "levels" left, we have to prepend ".." for each:
  while (levels > 0)
  {
    tokens.push_front("..");
    --levels;
  }

  // re-attach empty element for absolute path
  if (absolute) tokens.push_front("");
  // if it is still empty now, it can only be the current directory:
  if (tokens.empty()) tokens.push_back(".");
  // re-attach empty element for trailing slash
  if (directory) tokens.push_back("");
  // root directory
  if (tokens.size() == 1 && tokens.front() == "") tokens.push_back("");
}

/** Remove the last item of a list of path components.
 * This is normally a file name or an empty string (in case of a trailing
 * slash in the original path). If the last element in the list is ".." or
 * ".", it is not removed.
 **/
template<typename T>
void remove_last_component(T& tokens)
{
  if (!tokens.empty() && tokens.back() != ".." && tokens.back() != ".")
  {
    tokens.pop_back();
  }
}

/** Turn a list of path components into an absolute path.
 * @param[in,out] path A list of path components. If it's relative, the
 *   current working directory is prepended.
 **/
template<typename T>
void make_absolute(T& path)
{
  // if path is relative, prepend cwd
  if (!path.empty() && path.front() != "")
  {
    // get current working directory
    std::string cwd;
    if (!getcwd(cwd))
    {
      // this should never happen.
      assert(false);
      path = T();
      return;
    }
    T cwd_tokens;
    tokenize(cwd, cwd_tokens);
    path.splice(path.begin(), cwd_tokens);
  }
}

/** Make one list of path components relative to another.
 * @param[in,out] path path to be made relative to @p directory.
 * @param directory base directory. If empty, the current directory is used.
 * @note Initially, both @p path and @p directory can be relative or absolute.
 **/
template<typename T>
void make_relative(T& path, const T& directory = T())
{
  T dir(directory);
  if (dir.empty()) dir.push_back(".");

  // ironically, first we make both paths absolute:
  make_absolute(path);
  make_absolute(dir);

  // remove "." and "..":
  clean_path(path);
  clean_path(dir);

  // remove common parent directories
  while (!path.empty() && !dir.empty() && path.front() == dir.front())
  {
    path.pop_front();
    dir .pop_front();
  }

  while (!dir.empty())
  {
    if (dir.back() != "")
    {
      path.push_front("..");
    }
    // TODO: what if dir.back() == ".." (or ".")?
    dir.pop_back();
  }

  // if path consists of a single empty string
  // => it's the current directory with trailing slash:
  if (path.size() == 1 && path.front() == "")
  {
    path.push_front(".");
  }
}

/** Remove common directory part and prepend "../" if necessary.
 * If @p path is in the directory of @p filename or below, remove directory
 * from @p path. For each further sub-directories of @p filename, prepend
 * "../" to @p path. If they have no common directories, prepend an
 * appropriate number of parent directories and "../"s.
 * @param path A path given relative to the current directory.
 *   If @p path is absolute or empty, it is not changed. If @p path is
 *   relative, the result is also relative. If @p path has a trailing slash,
 *   the result will also have a trailing slash.
 * @param filename This can also be a directory, but then it has to end with a
 *   trailing slash.
 *   If @p filename doesn't have a directory part, @p path is not changed.
 * @warning This function expects UNIX-style paths!
 **/
inline std::string make_path_relative_to_file(const std::string& path
    , const std::string& filename)
{
  if (path != "" && path[0] != '/')
  {
    // path components will be stored in lists p1 and p2:
    std::list<std::string> p1, p2;

    tokenize(path, p1);
    tokenize(filename, p2);

    remove_last_component(p2);

    // if "filename" didn't have any path component, p2 is now empty and
    // "path" could be returned unchanged. However, we will clean it first.
    // Same thing if "filename" was ".".
    if (p2.empty() || (p2.size() == 1 && p2.front() == "."))
    {
      clean_path(p1);
      return untokenize(p1);
    }

    make_relative(p1, p2);

    return untokenize(p1);
  }
  return path; // path is absolute or empty; return it unchanged
}

/** Add directory part of one path to another path.
 * @param path A path given relative to @p filename.
 *   If @p path is absolute or empty, it is not changed. If @p path has a
 *   trailing slash, the result will also have a trailing slash.
 *   If @p path is relative, the result is also relative. 
 * @param filename The directory part of @p filename is prepended to @p path.
 * @return @p path prepended with the directory name of @p filename.
 * @warning This function expects UNIX-style paths!
 **/
inline std::string make_path_relative_to_current_dir(const std::string& path
    , const std::string& filename)
{
  if (path != "" && path[0] != '/')
  {
    auto result = std::list<std::string>();

    tokenize(filename, result);

    remove_last_component(result);

    // add the components of "path" to the end of the list
    tokenize(path, result);

    make_relative(result);

    return untokenize(result);
  }
  return path; // path is absolute or empty; return it unchanged
}

/** Get the extension of a file name.
 * If an extension contains a dot (e.g. tar.gz), only the last part is
 * returned (e.g. gz). If you want more, you have to run the function several
 * times.
 * @param filename the filename
 * @return the extension, excluding the dot; empty string if there is no
 *   extension
 * @warning This function expects UNIX-style paths!
 **/
inline std::string get_file_extension(const std::string& filename)
{
  auto last_slash = filename.rfind('/');
  auto last_dot   = filename.rfind('.');

  if (// if there is a dot and it is neither the first nor the last character
      last_dot > 0 && last_dot < filename.length() - 1
      // and if there is either no slash or it is before the last dot and at
      // least one other character is inbetween them
      && (last_slash == std::string::npos
        || (last_dot > last_slash && last_dot - last_slash > 1)))
  {
    return filename.substr(last_dot + 1);
  }
  return "";
}

/** Insert escape characters (\) before white spaces.
 * @param filename the file name
 * @return the file name with escaped white spaces
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

}  // namespace posixpathtools

#endif

// Settings for Vim (http://www.vim.org/), please do not remove:
// vim:softtabstop=2:shiftwidth=2:expandtab:textwidth=80:cindent
