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
/// XML parser class (implementation).

#include <iostream>
#include <cassert>

#include "xmlparser.h"

// TODO: avoid error messages in this kind of "library"?
/// print error message to standard error
#define ERR(msg)   std::cerr << msg << std::endl
//#define PRINT(msg) std::cout << msg << std::endl

XMLParser::~XMLParser() { } //xmlCleanupParser(); }

void XMLParser::Init() { xmlInitParser(); }

XMLParser::doc_t XMLParser::load_file(const std::string& file_name) const
{
  doc_t temp; // temp = NULL
  try
  {
    temp.reset(new Document(file_name, true));
  }
  catch(Document::document_error& e)
  {
    ERR(e.what());
  }
  return temp;
}

XMLParser::doc_t XMLParser::load_string(const std::string& xml_string) const
{
  doc_t temp; // temp = NULL
  try
  {
    temp.reset(new Document(xml_string, false));
  }
  catch(Document::document_error& e)
  {
    ERR(e.what());
  }
  return temp;
}

std::string XMLParser::replace_entities(const std::string& input) const
{
  xmlChar* xml_string;
  xml_string = xmlEncodeEntitiesReentrant(nullptr, BAD_CAST input.c_str());
  if (!xml_string) return "";
  std::string temp((char*)xml_string);
  xmlFree(xml_string);
  return temp;
}

XMLParser::Document::~Document()
{
  if (_xpath_context) xmlXPathFreeContext(_xpath_context);
  if (_doc)           xmlFreeDoc(_doc);
}

bool XMLParser::Document::validate(const std::string& schema_file_name) const
{
  if (!_doc)
  {
    ERR("Document not loaded!");
    return false;
  }
  xmlSchemaParserCtxtPtr schema_ctxt;
  schema_ctxt = xmlSchemaNewParserCtxt(schema_file_name.c_str());
  if (schema_ctxt == nullptr)
  {
    ERR("error in xmlSchemaNewParserCtxt!");
    return false;
  }

  xmlSchemaPtr schema;
  schema = xmlSchemaParse(schema_ctxt);
  if (schema == nullptr)
  {
    ERR("error in xmlSchemaParse!");
    return false;
  }
  xmlSchemaFreeParserCtxt(schema_ctxt);

  xmlSchemaValidCtxtPtr valid_schema;
  valid_schema = xmlSchemaNewValidCtxt(schema);
  if (valid_schema == nullptr)
  {
    ERR("error in xmlSchemaNewValidCtxt!");
    return false;
  }

  int schema_result;
  // Returns 1 if valid so far, 0 if errors were detected,
  // and -1 in case of internal error.
  schema_result = xmlSchemaIsValid(valid_schema);
  switch (schema_result)
  {
    case 1:
      //PRINT("Valid schema!");
      break;
    case 0:
      ERR("Invalid schema!");
      return false;
    case -1:
      ERR("Internal error in xmlSchemaIsValid()!");
      return false;
    default:
      ERR("Unknown return value of xmlSchemaIsValid()!");
      return false;
  }

  int doc_result;
  // Returns 0 if the document is schemas valid, a positive error code
  // otherwise and -1 in case of internal or API error.
  doc_result = xmlSchemaValidateDoc(valid_schema, _doc);

  ////////////////////////////////////////////////////////////////////
  // WARNING // WARNING // WARNING // WARNING // WARNING // WARNING //
  //
  // the return values of xmlSchemaIsValid() and xmlSchemaValidateDoc()
  // have a completely different meaning!
  /////////////////////////////////////////////////////////////////////

  if (doc_result > 0)
  {
    ERR("Invalid doc! (error code: " << doc_result << ")");
    return false;
  }
  else if (doc_result < 0)
  {
    ERR("Internal error in xmlSchemaValidateDoc()! "
        "(error code: " << doc_result << ")");
    return false;
  }
  xmlSchemaFreeValidCtxt(valid_schema);
  return true;
}

//int xmlSchemaValidateOneElement(valid_schema, xmlNodePtr elem);

XMLParser::XPathResult::XPathResult(const xmlXPathObjectPtr xpath_object) :
  _xpath_object(xpath_object),
  _current(0)
{}

XMLParser::XPathResult::~XPathResult()
{
  if (_xpath_object) xmlXPathFreeObject(_xpath_object);
}

xmlNodePtr XMLParser::XPathResult::node() const
{
  if (!_xpath_object) return nullptr;
  if (_current >= _xpath_object->nodesetval->nodeNr) return nullptr;
  return _xpath_object->nodesetval->nodeTab[_current];
}

int XMLParser::XPathResult::size() const
{
  if (!_xpath_object) return 0;
  return _xpath_object->nodesetval->nodeNr;
}

/** _.
 * The corresponding postfix operator (xpath++) is not implemented because it
 * would also need a copy constructor to be implemented. For now, the
 * preincrement operator is sufficient.
 **/
XMLParser::XPathResult& XMLParser::XPathResult::operator++()
{
  _current++;
  return *this;
}

// not implemented to avoid implementing the copy ctor:
/*
XPathResult XMLParser::XPathResult::operator++(int)
{
  XPathResult old(*this); // copy ctor needed!
  operator++();
  return old;
}
*/

XMLParser::xpath_t XMLParser::Document::eval_xpath(
    const std::string& expression) const
{
  if (!_xpath_available())
  {
    ERR("Couldn't create XPath context!");
    return xpath_t(nullptr);
  }
  xmlXPathObjectPtr result;
  result = xmlXPathEvalExpression(BAD_CAST expression.c_str(),
      _xpath_context);
  if (result == nullptr)
  {
    ERR("Error in xmlXPathEvalExpression!");
    return xpath_t(nullptr);
  }
  if(xmlXPathNodeSetIsEmpty(result->nodesetval))
  {
    xmlXPathFreeObject(result);
    //ERR("No result!");
    return xpath_t(nullptr);
  }
  return xpath_t(new XPathResult(result));
}

XMLParser::Document::Document(const std::string& input, bool file)
  throw (document_error) :
  _doc(nullptr),
  _xpath_context(nullptr)
{
  if (file)
  {
    _doc = xmlParseFile(input.c_str());
    if (_doc == nullptr)
    {
      throw document_error("Error in xmlParseFile (" + input + ")!");
    }
  }
  else
  {
    _doc = xmlParseDoc(BAD_CAST input.c_str());
    if (_doc == nullptr)
    {
      throw document_error("error in xmlParseDoc (" + input + ")!");
    }
  }
}

bool XMLParser::Document::_xpath_available() const
{
  if (_xpath_context == nullptr)
  {
    _xpath_context = xmlXPathNewContext(_doc);
    if (_xpath_context == nullptr)
    {
      ERR("Error in xmlXPathNewContext");
      return false;
    }
  }
  return true;
}

XMLParser::Node::Node(const xmlNodePtr node) :
  _node(node)
{}

XMLParser::Node XMLParser::new_node(const std::string& name)
{
  return Node(xmlNewNode(nullptr, BAD_CAST name.c_str()));
}

/*
bool XMLParser::Node::add_child(const Node& child_node)
{
  return xmlAddChild(_node, child_node.get());
}
*/

xmlNodePtr XMLParser::Node::new_child(const std::string& name,
    const std::string& content)
{
  if (!_node) return nullptr;
  return xmlNewChild(_node, nullptr, BAD_CAST name.c_str(),
    content == "" ? nullptr : BAD_CAST content.c_str());
}

void XMLParser::Node::new_attribute(const std::string& name,
    const std::string& value)
{
  xmlNewProp(_node, BAD_CAST name.c_str(), BAD_CAST value.c_str());
}

std::string XMLParser::Node::get_attribute(const std::string& attr_name) const
{
  if (!_node) return "";
  xmlChar* xml_string;
  xml_string = xmlGetProp(_node, BAD_CAST attr_name.c_str());
  if (!xml_string) return "";
  std::string temp((char*)xml_string);
  xmlFree(xml_string);
  return temp;
}

std::string get_content(const xmlNodePtr& node)
{
  if (!node) return "";
  xmlChar* xml_string;
  xml_string = xmlNodeGetContent(node);
  std::string result((char*)xml_string);
  xmlFree(xml_string);
  return result;
}

std::string get_content(const XMLParser::Node& node)
{
  return get_content(node.get());
}

/* ._
 * @todo return pointer to the old node? or to the new node? or nothing?
 **/
void XMLParser::Node::descend() throw (std::underflow_error)
{
  if (!_node) throw std::underflow_error(
      "XMLParser::Node::descend(): empty node!");
  _node = _node->xmlChildrenNode;
}

XMLParser::Node XMLParser::Node::child() const throw (std::underflow_error)
{
  if (!_node) throw std::underflow_error(
      "XMLParser::Node::child(): empty node!");
  return(Node(_node->xmlChildrenNode));
}

XMLParser::Node XMLParser::Node::child(const std::string& name) const
{
  for (Node i = this->child(); !!i; ++i)
  {
    if (i == name) return i;
  }
  return Node(nullptr);
}

xmlNodePtr XMLParser::Node::get() const
{
  return _node;
}

std::string XMLParser::Node::to_string()
{
  //xmlDocPtr doc = xmlNewDoc(BAD_CAST "1.0");
  //xmlDocSetRootElement(doc, _node);
  //int size;
  //xmlChar * temp;
  //xmlDocDumpFormatMemory(doc, &temp, &size, 1);
  //xmlDocDumpMemory(doc, &temp, &size);
  xmlBufferPtr buffer = xmlBufferCreate();
  //if (xmlNodeDump(buffer, doc, _node, 0, 1) == -1)

  if ( !_node )
  {
    std::cout << "Empty _node!\n";
    return "";
  }

  // last arg: 1 = format, 0 = everything in one line
  if (xmlNodeDump(buffer, _node->doc, _node, 0, 0) == -1)
  //if (xmlNodeBufGetContent(buffer, _node) != 0)
  {
    std::cout << "couldn't convert XML document to string!\n";
    return "";
  }
  std::string result = (char *)xmlBufferContent(buffer);
  //std::string result = (char *)temp;
  xmlBufferFree(buffer);

  //xmlFree(temp);
  //xmlFreeDoc(doc);
  // when doc is freed, all nodes are deleted, too
  //_node = nullptr;

  return result;
}

/*
std::string XMLParser::Node::to_string()
{
  xmlDocPtr doc = xmlNewDoc(BAD_CAST "1.0");
  xmlNodePtr node = xmlCopyNode(_node, 1);
  xmlDocSetRootElement(doc, node);
  int size;
  xmlChar * temp;
  xmlDocDumpFormatMemory(doc, &temp, &size, 1);
  //xmlChar * temp = xmlNodeListGetString(_node->doc, _node, 1);
  std::string result = (char *)temp;
  xmlFree(temp);
  xmlFreeDoc(doc);
  return result;
}
*/

// preincrement
XMLParser::Node& XMLParser::Node::operator++() throw (std::overflow_error)
{
  if (!_node) throw std::overflow_error(
      "XMLParser::Node::operator++: empty node!");
  _node = _node->next;
  return *this;
}

xmlNodePtr XMLParser::Node::operator=(const xmlNodePtr node)
{
  _node = node;
  return node;
}

bool XMLParser::Node::operator!() const
{
  return !_node;
}

bool operator==(const XMLParser::Node& node, const std::string& str)
{
  return !xmlStrcmp(node.get()->name, BAD_CAST str.c_str());
}

bool operator==(const std::string& str, const XMLParser::Node& node)
{
  return node == str;
}

std::ostream& operator<<(std::ostream& stream, const XMLParser::Node& node)
{
  stream << (char*)node.get()->name;
  return stream;
}

// Settings for Vim (http://www.vim.org/), please do not remove:
// vim:softtabstop=2:shiftwidth=2:expandtab:textwidth=80:cindent
