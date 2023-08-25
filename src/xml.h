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
    DOCUMENT,
      ELEMENT,
      DATA,
      CDATA,
      COMMENT,
      DECLARATION,
      DOCTYPE,
      PI
  };

  struct NodeBase {
    virtual ~NodeBase() {}

    virtual kj::StringPtr name() const = 0;
    virtual kj::StringPtr value() const = 0;
    virtual kj::Maybe<const Node&> parent() const = 0;
    virtual kj::Maybe<const Document&> document() const = 0;
  };

  struct Node
    : NodeBase {

    virtual NodeType type() const  = 0;
    virtual kj::Maybe<const Node&> prev_sibling(kj::StringPtr = {});
    virtual kj::Maybe<const Node&> next_sibling(kj::StringPtr = {});
  };

  struct Document
    : Node {
  };

  struct Attribute
    : NodeBase {

    virtual kj::Maybe<const Document&> document() const = 0;
    virtual kj::Maybe<const Attribute&> prev(kj::StringPtr name) const = 0;
    virtual kj::Maybe<const Attribute&> next(kj::StringPtr name) const = 0;
  };


  kj::Own<Document> parse(kj::StringPtr txt);
}
