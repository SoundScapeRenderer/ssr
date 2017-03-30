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
/// XML parser class (definition).

#ifndef SSR_XMLPARSER_H
#define SSR_XMLPARSER_H

#include <string>
#include <memory> // for std::unique_ptr
#include <stdexcept>
#include <libxml/parser.h>
#include <libxml/xmlschemas.h>
#include <libxml/xpath.h>

#include "apf/misc.h"  // for NonCopyable

/// Wrapper for libxml2
struct XMLParser
{
  // using automatically generated ctor.

  ~XMLParser();

  static void Init();
  class Document;    // forward declaration
  class XPathResult; // forward declaration
  class Node;        // forward declaration

  typedef std::unique_ptr<Document>    doc_t;
  typedef std::unique_ptr<XPathResult> xpath_t;

  doc_t load_file(const std::string& file_name) const;
  doc_t load_string(const std::string& xml_string) const;

  // we should put that outside of the class in the surrounding namespace, but
  // we don't have one ...
  std::string replace_entities(const std::string& input) const;

  Node new_node(const std::string& name);
};

////////////////////////////////////////////////////////////////////////////////
// Definition of the nested class Document
////////////////////////////////////////////////////////////////////////////////

/// XML document.
class XMLParser::Document
{
  public:
    /// exception to be thrown by ctor.
    struct document_error : public std::runtime_error
    {
      document_error(const std::string& s): std::runtime_error(s) {}
    };

    explicit Document(const std::string& input, bool file = true)
      throw (document_error);

    ~Document();

    bool validate(const std::string& schema_file_name) const;

    xpath_t eval_xpath(const std::string& expression) const;

  private:
    bool _xpath_available() const;

    xmlDocPtr _doc;
    mutable xmlXPathContextPtr _xpath_context;
};

////////////////////////////////////////////////////////////////////////////////
// Definition of the nested class XPathResult
////////////////////////////////////////////////////////////////////////////////

/** The result of an XPath query.
 * Holds an xmlXPathObjectPtr and allows to iterate through the results.
 **/
class XMLParser::XPathResult : apf::NonCopyable
{
  public:
    explicit XPathResult(const xmlXPathObjectPtr xpath_object); ///< ctor.
    ~XPathResult();                                             ///< dtor.
    xmlNodePtr node() const;   ///< return the current node of the nodeset
    int size() const;          ///< return the number of results in the nodeset
    XPathResult& operator++(); ///< prefix operator (++xpath)

    // not implemented to avoid implementing the copy ctor:
    // XPathResult operator++(int); // postfix operator (xpath++)

  private:
    xmlXPathObjectPtr _xpath_object; ///< pointer to the original result
    int _current; ///< number of the current node in the nodeset
};

////////////////////////////////////////////////////////////////////////////////
// Definition of the nested class Node
////////////////////////////////////////////////////////////////////////////////

/// node of a DOM tree
class XMLParser::Node
{
  public:
    // ctor is not explicit, can be used for implicit conversions!
    Node(const xmlNodePtr node = nullptr); ///< ctor

    //bool add_child(const Node& child_node);
    xmlNodePtr new_child(const std::string& name,
        const std::string& content = "");
    void new_attribute(const std::string& name, const std::string& value);
    /// get a named attribute from the current node.
    std::string get_attribute(const std::string& attr_name) const;

    void descend() throw (std::underflow_error);     ///< go to the first child
    Node child() const throw (std::underflow_error); ///< get the first child
    Node child(const std::string& name) const; ///< get element named @a name
    xmlNodePtr get() const;

    std::string to_string();

    Node&      operator++() throw (std::overflow_error);
    xmlNodePtr operator= (const xmlNodePtr);
    bool       operator! () const;

    /// identity operator (==) between a node and a node name
    friend bool operator==(const Node& node, const std::string& str);
    /// identity operator (==) between a node name and a node
    friend bool operator==(const std::string& str, const Node& node);
    /// output stream operator (<<)
    friend std::ostream& operator<<(std::ostream& stream, const Node& node);

  private:
    xmlNodePtr _node;
};

std::string get_content(const xmlNodePtr& node);      ///< get node content
std::string get_content(const XMLParser::Node& node); ///< get node content

#endif

// Settings for Vim (http://www.vim.org/), please do not remove:
// vim:softtabstop=2:shiftwidth=2:expandtab:textwidth=80:cindent
