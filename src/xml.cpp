// Copyright (c) 2023 Vaci Koblizek.
// Licensed under the Apache 2.0 license found in the LICENSE file or at:
//     https://opensource.org/licenses/Apache-2.0

#include "xml.h"

#include <kj/common.h>
#include <kj/debug.h>
#include <kj/vector.h>

namespace xml {

  
kj::Maybe<const Document&> Node::document() const {
  const Node* node = this;
  while (true) {
    KJ_IF_MAYBE(p, node->parent()) {
      node = p;
    }
    else {
      break;
    }
  }

  if (node->type() == NodeType::DOCUMENT) {
    return static_cast<const Document&>(*node);
  }
  else {
    return nullptr;
  }
}

namespace {

  struct AttributeImpl;

  bool isNodeName(char ch) {
    return
      ch != ' ' &&
      ch != '\n' &&
      ch != '\r' &&
      ch != '\t' &&
      ch != '/' &&
      ch != '>' &&
      ch != '?' &&
      ch != '\0';
  }

  bool isAttrName(char ch) {
    return
      ch != ' ' &&
      ch != '\n' &&
      ch != '\r' &&
      ch != '\t' &&
      ch != '/' &&
      ch != '<' &&
      ch != '>' &&
      ch != '=' &&
      ch != '?' &&
      ch != '!' &&
      ch != '\0';
  }

  bool isWhitespace(char ch) {
    return
      ch == '\t' ||
      ch == '\n' ||
      ch == '\r' ||
      ch == ' ';
  }

  bool isPcdata(char ch) {
    return ch != '<' && ch != '&';
  }
}

static inline const char* skipSpace(const char* p) {
  for (;;) {
    switch (*p) {
      case '\t':
      case '\n':
      case '\r':
      case ' ':
        ++p;
        break;
      default:
        return p;
    }
  }
}

struct Parser {
  kj::StringPtr str;

  Parser(kj::StringPtr str)
    : str(str) {
  }

  void consume(size_t count = 1) {
    str = str.slice(count);
  }

  void skipSpace() {
    while (str.size() && isWhitespace(str[0])) {
      consume();
    }
  }

  void parseBom() {
    constexpr auto bom = "\xEF\xBB\xBF"_kj;
    if (str.startsWith(bom)) {
      consume(bom.size());
    }
  }
    
  kj::Own<Node> parseNode();

  kj::Array<kj::Own<Attribute>> parse_node_attrs() {
    kj::Vector<kj::Own<Attribute>> attrs;
    
    while (str.size() && isAttrName(str[0])) {
      auto name = str;
      consume();
      while (str.size() && isAttrName(str[0])) {
	consume();
      }
      KJ_REQUIRE(name != str);

      auto attr = kj::heap<Attribute>();
      attr->name_ = name.slice(0, str.begin() - name.begin());
      skipSpace();
      KJ_REQUIRE(str.size() && str[0] == '=');
      consume();
      skipSpace();

      KJ_REQUIRE(str.size());
      char quote = str[0];
      KJ_REQUIRE(quote == '\'' || quote == '\"');
      consume();

      auto len = KJ_REQUIRE_NONNULL(str.findFirst(quote));
      attr->value_ = str.slice(0, len);
      attrs.add(kj::mv(attr));

      consume(len);
      consume();
      skipSpace();
    }
    return attrs.releaseAsArray();
  }

 kj::Array<kj::Own<Node>> parse_node_contents(Node& node) {
   kj::Vector<kj::Own<Node>> children;
   
  while (true) {
    auto value = str;
    skipSpace();
    KJ_REQUIRE(str.size());
    if (str[0] == '<') {
      consume();
      KJ_REQUIRE(str.size() > 1);
      if (str[0] == '/') {
	consume();
	while (str.size() && isNodeName(str[0])) {
	  consume();
	}
	skipSpace();
	KJ_REQUIRE(str.size() && str[0] == '>');
	consume();
	return children.releaseAsArray();
      }
      else {
	children.add(parseNode());
      }
    }
    else {
      // data node
      while (str.size() && isPcdata(str[0])) {
	consume();
      }
      node.value_ = value.slice(0, str.begin() - value.begin());
    }
  }
}
  
kj::Own<Node> parse_doctype() {
  auto value = str;
  while (str.size()) {
    switch (str[0]) {
      case '>': {
	auto node = kj::heap<Node>(NodeType::DOCTYPE);
	//node->value_ = kj::arrayPtr(value, txt-value);
	node->value_ = value.slice(0, str.begin() - value.begin());
	consume();
        return node;
      }
      case'[': {
	int depth = 1;
	while (depth) {
	  KJ_REQUIRE(str.size());
	  switch (str[0]) {
	  case '[': ++depth; break;
	  case ']': --depth; break;
	  default: break;
	  }
	  consume();
	}
	break;
      }

      default:
	consume();
	break;
    }
  }
  KJ_FAIL_REQUIRE("Expected >");
}

  kj::Own<Node> parsePI() {
    while (!str.startsWith("?>"_kj)) {
      KJ_REQUIRE(str.size());
      consume();
    }

    consume(2ul);
    return kj::heap<Node>(NodeType::PI);
  }

  kj::Own<Node> parseCdata() {
    auto value = str;
    while (!str.startsWith("]]>"_kj)) {
      KJ_REQUIRE(str.size());
      consume();
    }

    auto node = kj::heap<Node>(NodeType::CDATA);
    node->value_ = value.slice(0, str.begin() - value.begin());
    consume(3ul);
    return node;
  }

  kj::Own<Node> parseElement() {
    auto node = kj::heap<Node>(NodeType::ELEMENT);
    auto name = str;
    while (str.size() && isNodeName(str[0])) {
      consume();
    }
    KJ_REQUIRE(str != name);
    node->name_ = name.slice(0, str.begin() - name.begin());
    skipSpace();
    node->attrs_ = parse_node_attrs();

    if (str.startsWith(">"_kj)) {
     consume();
     node->children_ = parse_node_contents(*node);
    }
    else if (str.startsWith("/>"_kj)) {
      consume(2ul);
    }
    else {
      KJ_FAIL_REQUIRE("Expected >");
    }
    
    return node;
  }

  kj::Own<Node> parseComment() {
    return kj::heap<Node>(NodeType::COMMENT);
  }

  kj::Own<Node> parseXmlDeclaration() {
    auto node = kj::heap<Node>(NodeType::DECLARATION);
    skipSpace();
    node->attrs_ = parse_node_attrs();
      return node;
  }
    

  kj::Own<Document> parse() {
    auto doc = kj::Own<Document>();
    kj::Vector<kj::Own<Node>> nodes;
    parseBom();

    while (true) {
      skipSpace();

      if (str.size() == 0) {
	break;
      }

      KJ_REQUIRE(str[0] == '<');
      consume();
      nodes.add(parseNode());
    }

    doc->children_ = nodes.releaseAsArray();
    return doc;
  }
};

kj::Own<Node> Parser::parseNode() {
  KJ_REQUIRE(str.size());

  switch (str[0]) {
  case '?':
    consume();
    if (str.size() >= 4 &&
	(str[0] == 'x' || str[0] == 'X') &&
	(str[1] == 'm' || str[1] == 'M') && 
	(str[2] == 'l' || str[2] == 'L') &&
	isWhitespace(str[3])) {
      // '<?xml ' - xml declaration
      consume(4ul);
      return parseXmlDeclaration();
    }
    else {
      // Parse PI
      return parsePI();
    }
  case '!': {
    consume();
    if (str.startsWith("--"_kj)) {
      // '<!--' - xml comment
      consume(2ul);
      return parseComment();
    }
    else  if (str.startsWith("[CDATA["_kj)) {
      consume(7ul);
      return parseCdata();
    }
    else if (str.startsWith("DOCTYPE") && isWhitespace(str[7])) {
      // '<!DOCTYPE ' - doctype
      consume(8ul);
      return parse_doctype();
    }
    else {
      auto count = KJ_REQUIRE_NONNULL(str.findFirst('>'));
      consume(count);
      return kj::heap<Node>(NodeType::UNKNOWN);
    }
  }
  default:
    return parseElement();
  }
}

  
kj::Own<Document> parse(kj::StringPtr txt) {
  Parser parser{txt};
  return parser.parse();
}
  
kj::Own<Node> parse_node(kj::StringPtr txt) {
  Parser parser{txt};
  return parser.parseNode();
}

}
