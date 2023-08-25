// Copyright (c) 2023 Vaci Koblizek.
// Licensed under the Apache 2.0 license found in the LICENSE file or at:
//     https://opensource.org/licenses/Apache-2.0

#include "xml.h"

namespace xml {

namespace {

  struct AttributeImpl;
    
  constexpr kj::byte whitespace[256] = {
 // 0   1   2   3   4   5   6   7   8   9   A   B   C   D   E   F
    0,  0,  0,  0,  0,  0,  0,  0,  0,  1,  1,  0,  0,  1,  0,  0,  // 0
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  // 1
    1,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  // 2
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  // 3
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  // 4
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  // 5
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  // 6
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  // 7
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  // 8
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  // 9
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  // A
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  // B
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  // C
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  // D
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  // E
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0   // F

  };

  constexpr kj::byte nodeName[256] = {
 // 0   1   2   3   4   5   6   7   8   9   A   B   C   D   E   F
    0,  1,  1,  1,  1,  1,  1,  1,  1,  0,  0,  1,  1,  0,  1,  1,  // 0
    1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // 1
    0,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  0,  // 2
    1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  0,  0,  // 3
    1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // 4
    1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // 5
    1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // 6
    1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // 7
    1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // 8
    1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // 9
    1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // A
    1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // B
    1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // C
    1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // D
    1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // E
    1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1   // F
  };

  constexpr kj::byte digits[256] = {
 // 0   1   2   3   4   5   6   7   8   9   A   B   C   D   E   F
    255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,  // 0
    255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,  // 1
    255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,  // 2
      0,  1,  2,  3,  4,  5,  6,  7,  8,  9,255,255,255,255,255,255,  // 3
    255, 10, 11, 12, 13, 14, 15,255,255,255,255,255,255,255,255,255,  // 4
    255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,  // 5
    255, 10, 11, 12, 13, 14, 15,255,255,255,255,255,255,255,255,255,  // 6
    255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,  // 7
    255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,  // 8
    255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,  // 9
    255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,  // A
    255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,  // B
    255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,  // C
    255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,  // D
    255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,  // E
    255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255   // F
  };

  kj::StringPtr skip_ws(kj::StringPtr txt) {
    auto* tmp = txt.begin();
    auto count = 0u;
    while (tmp != txt.end() || *tmp == ' ' || *tmp == '\t') {
      count++;
      tmp++;
    }
    return kj::StringPtr{tmp, txt.size()-count};
  }

kj::StringPtr parse_bom(kj::StringPtr txt) {
  if (static_cast<unsigned char>(txt[0]) == 0xEF && 
      static_cast<unsigned char>(txt[1]) == 0xBB && 
      static_cast<unsigned char>(txt[2]) == 0xBF) {
    return kj::StringPtr{txt.begin()+3, txt.size()-3};
  }
}


struct NodeBaseImpl {

    kj::StringPtr name_{""_kj};
    kj::StringPtr value_{""_kj};
    kj::Maybe<Node&> parent_;
  };

  struct NodeImpl 
    : Node, NodeBaseImpl {

    NodeImpl(NodeType type)
      : type_{type} {
    }
 
    NodeType type() const override { return type_; }

    kj::StringPtr name() const override {
      return name_;
    }

    kj::StringPtr value() const override {
      return value_;
    }

    kj::Maybe<const Node&> parent() const override {
      return parent_;
    }
    
    kj::Maybe<const Document&> document() const override {
      const Node* node = this;
      while (true) {
	KJ_IF_MAYBE(p, node->parent()) {
	  node = p;
	}
	else {
	  break;
	}
      }

      if (node->type() == NodeType::document) {
	return static_cast<const Document&>(*node);
      }
      else {
	return nullptr;
      }
    }

    kj::Maybe<const Node&> first_node(kj::StringPtr name) const {
      if (name.size()) {
	for (auto child = first_node_; child; child = child->next_sibling_) {
	  if (child->name_ == name) {
	    return *child;
	  }
	  else {
	    return nullptr;
	  }
	}
      }
      else {
	return *first_node_;
      }
    }

    kj::Maybe<const Node&> last_node(kj::StringPtr name) const {
      if (name.size()) {
	for (auto child = last_node_; child; child = child->prev_sibling_) {
	  if (child->name_ == name) {
	    return *child;
	  }
	  else {
	    return nullptr;
	  }
	}
      }
      else {
	return *last_node_;
      }
    }

    kj::Maybe<const Node&> prev_sibling(kj::StringPtr name = {}) const {
      if (name.size()) {
	for (auto sibling = prev_sibling_; sibling; sibling = sibling->prev_sibling_) {
	  if (sibling->name() == name) {
	    return *sibling;
	  }
	}
	return nullptr;
      }
      else {
	return *prev_sibling_;
      }
    }

    kj::Maybe<const Node&> next_sibling(kj::StringPtr name = {}) const {
      if (name.size()) {
	for (auto sibling = next_sibling_; sibling; sibling = sibling->next_sibling_) {
	  if (sibling->name_ == name) {
	    return *sibling;
	  }
	}
	return nullptr;
      }
      else {
	return *next_sibling_;
      }
    }

    kj::Maybe<const Attribute&> first_attr(kj::StringPtr name = {}) const;
    kj::Maybe<const Attribute&> last_attr(kj::StringPtr name = {}) const;
    
    NodeType type_;
    NodeImpl* first_node_;
    NodeImpl* last_node_;
    AttributeImpl* first_attr_;
    AttributeImpl* last_attr_;
    
    NodeImpl* prev_sibling_;
    NodeImpl* next_sibling_;
  };

  struct AttributeImpl
    : virtual Attribute, NodeBaseImpl  {

    kj::StringPtr name() const override {
      return name_;
    }

    kj::StringPtr value() const override {
      return value_;
    }

    kj::Maybe<const Node&> parent() const override {
      return parent_;
    }

    kj::Maybe<const Document&> document() const override {
      auto node = &KJ_UNWRAP_OR_RETURN(parent(), nullptr);
      while (true) {
	KJ_IF_MAYBE(p, node->parent()) {
	  node = p;
	  continue;
	}
	else {
	  if (node->type() == NodeType::document) {
	    return static_cast<const Document&>(*node);
	  }
	  else {
	    return nullptr;
	  }
	}
      }
    }

    kj::Maybe<const Attribute&> prev(kj::StringPtr name) const override {
      if (name.size()) {
	for (auto attr = prev_attr_; attr; attr = attr->prev_attr_) {
	  if (attr->name_ == name) {
	    return *attr;
	  }
	}
	return nullptr;
      }
      else {
	return parent_ != nullptr ? prev_attr_ : nullptr;
      }
    }

    kj::Maybe<const Attribute&> next(kj::StringPtr name) const override {
      if (name.size()) {
	for (auto attr = next_attr_; attr; attr = attr->next_attr_) {
	  if (attr->name_ == name) {
	    return *attr;
	  }
	}
	return nullptr;
      }
      else {
	return parent_ != nullptr ? next_attr_ : nullptr;
      }
    }
    
    AttributeImpl* prev_attr_;
    AttributeImpl* next_attr_;
  };

  kj::Maybe<const Attribute&> NodeImpl::first_attr(kj::StringPtr name) const {
    if (name.size()) {
      for (auto attr = first_attr_; attr; attr = attr->next_attr_) {
	if (attr->name_ == name) {
	  return *attr;
	}
      }
      return nullptr;
    }
    else {
      return *first_attr_;
    }
  }

  kj::Maybe<const Attribute&> NodeImpl::last_attr(kj::StringPtr name) const {
    if (name.size()) {
      for (auto attr = last_attr_; attr; attr = attr->prev_attr_) {
	if (attr->name_ == name) {
	  return *attr;
	}
      }
      return nullptr;
    }
    else {
      return *last_attr_;
    }
  }

kj::Own<NodeImpl> parse_xml_declaration(kj::StringPtr txt) {
  auto node = kj::heap<NodeImpl>();
  txt = skip_ws(txt);
  return node;
}

}

  
kj::Own<Document> parse(kj::StringPtr txt) {
  return {};
}

}
