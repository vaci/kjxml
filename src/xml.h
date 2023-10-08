#pragma once

// Copyright (c) 2023 Vaci Koblizek.
// Licensed under the Apache 2.0 license found in the LICENSE file or at:
//     https://opensource.org/licenses/Apache-2.0

#include <kj/common.h>
#include <kj/memory.h>
#include <kj/string.h>

namespace xml {

  struct Attribute;
  struct Document;
  struct Node;

  enum class NodeType {
    UNKNOWN,
    DOCUMENT,
    ELEMENT,
    DATA,
    CDATA,
    COMMENT,
    DECLARATION,
    DOCTYPE,
    PI
  };

  struct Node {

    Node(NodeType type)
      : type_{type} {
    }

    kj::Maybe<const Node&> parent() const {
      return parent_;
    }

    kj::ArrayPtr<const char> name() const { return name_; }
    kj::ArrayPtr<const char> value() const { return value_; }
    
    kj::Maybe<const Document&> document() const;
 
    NodeType type() const { return type_; }

    NodeType type_;
    kj::Array<kj::Own<Attribute>> attrs_;
    kj::ArrayPtr<const char> name_;
    kj::ArrayPtr<const char> value_;
    kj::Array<kj::Own<Node>> children_;
    kj::Maybe<const Node&> parent_;
  };

  struct Document
    : Node {
  };

  struct Attribute {
    kj::ArrayPtr<const char> name_{};
    kj::ArrayPtr<const char> value_{};
  };


  kj::Own<Document> parse(kj::StringPtr txt);
  kj::Own<Node> parse_node(kj::StringPtr txt);
}
