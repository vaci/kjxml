#include "xml.h"


#include <gtest/gtest.h>


#include <kj/main.h>

struct XmlTest
  : testing::Test {
  
  XmlTest() {  
  }

  ~XmlTest() noexcept {
  }
};

TEST_F(XmlTest, Declaration) {
  auto txt = "?xml ><hello/>"_kj;
  auto node = xml::parse_node(txt);
  EXPECT_EQ(node->type(), xml::NodeType::DECLARATION);
}

TEST_F(XmlTest, Docttpe) {
  auto txt = "!DOCTYPE foo>";
  auto node = xml::parse_node(txt);
  EXPECT_EQ(node->type(), xml::NodeType::DOCTYPE);
  EXPECT_EQ(kj::str(node->name()), kj::str(""));
  EXPECT_EQ(kj::str(node->value()), kj::str("foo"));
}

TEST_F(XmlTest, Cdata) {
  auto txt = "![CDATA[foo]]>";
  auto node = xml::parse_node(txt);
  EXPECT_EQ(node->type(), xml::NodeType::CDATA);
  EXPECT_EQ(kj::str(node->name()), kj::str(""));
  EXPECT_EQ(kj::str(node->value()), kj::str("foo"));
}

TEST_F(XmlTest, Element) {
  auto txt = "hello/>";
  auto node = xml::parse_node(txt);
  EXPECT_EQ(node->type(), xml::NodeType::ELEMENT);
  EXPECT_EQ(kj::str(node->name()), kj::str("hello"));
  EXPECT_EQ(kj::str(node->value()), kj::str(""));
  EXPECT_FALSE(node->children_.size());
}

TEST_F(XmlTest, ElementWithAttr) {
  auto txt = R"(hello name="foo"/>)";
  auto node = xml::parse_node(txt);
  EXPECT_EQ(node->type(), xml::NodeType::ELEMENT);
  EXPECT_EQ(kj::str(node->name()), kj::str("hello"));
  EXPECT_EQ(kj::str(node->value()), kj::str(""));
  EXPECT_FALSE(node->children_.size());

  EXPECT_EQ(node->attrs_.size(), 1);
  EXPECT_EQ(kj::str(node->attrs_[0]->name_), "name");
  EXPECT_EQ(kj::str(node->attrs_[0]->value_), "foo");
}

TEST_F(XmlTest, ElementWithAttrs) {
  auto txt = R"(hello name="foo" bar="baz" />)";
  auto node = xml::parse_node(txt);
  EXPECT_EQ(node->type(), xml::NodeType::ELEMENT);
  EXPECT_EQ(kj::str(node->name()), kj::str("hello"));
  EXPECT_EQ(kj::str(node->value()), kj::str(""));
  EXPECT_FALSE(node->children_.size());

  EXPECT_EQ(node->attrs_.size(), 2);
  EXPECT_EQ(kj::str(node->attrs_[0]->name_), "name");
  EXPECT_EQ(kj::str(node->attrs_[0]->value_), "foo");
  EXPECT_EQ(kj::str(node->attrs_[1]->name_), "bar");
  EXPECT_EQ(kj::str(node->attrs_[1]->value_), "baz");
}

TEST_F(XmlTest, ElementWithSpacedAttrs) {
  auto txt = R"(hello   name="foo"   bar="baz"  />)";
  auto node = xml::parse_node(txt);
  EXPECT_EQ(node->type(), xml::NodeType::ELEMENT);
  EXPECT_EQ(kj::str(node->name()), kj::str("hello"));
  EXPECT_EQ(kj::str(node->value()), kj::str(""));
  EXPECT_FALSE(node->children_.size());

  EXPECT_EQ(node->attrs_.size(), 2);
  EXPECT_EQ(kj::str(node->attrs_[0]->name_), "name");
  EXPECT_EQ(kj::str(node->attrs_[0]->value_), "foo");
  EXPECT_EQ(kj::str(node->attrs_[1]->name_), "bar");
  EXPECT_EQ(kj::str(node->attrs_[1]->value_), "baz");
}

TEST_F(XmlTest, ElementWithData) {
  auto txt = R"(hello>world</hello>)";
  auto node = xml::parse_node(txt);
  EXPECT_EQ(node->type(), xml::NodeType::ELEMENT);
  EXPECT_EQ(kj::str(node->name()), kj::str("hello"));
  EXPECT_EQ(kj::str(node->value()), "world");
  EXPECT_FALSE(node->children_.size());
}
TEST_F(XmlTest, ElementWithChild) {
  auto txt = R"(hello><world/></hello>)";
  auto node = xml::parse_node(txt);
  EXPECT_EQ(node->type(), xml::NodeType::ELEMENT);
  EXPECT_EQ(kj::str(node->name()), kj::str("hello"));
  EXPECT_EQ(kj::str(node->value()), "");
  EXPECT_EQ(node->children_.size(), 1);

  EXPECT_EQ(kj::str(node->children_[0]->name()), kj::str("world"));
  EXPECT_EQ(kj::str(node->children_[0]->value()), ""); 
}
TEST_F(XmlTest, ElementWithChildren) {
  auto txt = R"(foo><bar/><baz/></foo>)";
  auto node = xml::parse_node(txt);
  EXPECT_EQ(node->type(), xml::NodeType::ELEMENT);
  EXPECT_EQ(kj::str(node->name()), kj::str("foo"));
  EXPECT_EQ(kj::str(node->value()), "");
  EXPECT_EQ(node->children_.size(), 2);

  EXPECT_EQ(kj::str(node->children_[0]->name()), kj::str("bar"));
  EXPECT_EQ(kj::str(node->children_[0]->value()), ""); 
  EXPECT_EQ(kj::str(node->children_[1]->name()), kj::str("baz"));
  EXPECT_EQ(kj::str(node->children_[1]->value()), ""); 
}

int main(int argc, char* argv[]) {
  kj::TopLevelProcessContext processCtx{argv[0]};
  processCtx.increaseLoggingVerbosity();

  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

