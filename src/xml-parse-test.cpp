#include <gtest/gtest.h>

#include <kj/debug.h>
#include <kj/main.h>
#include <kj/parse/char.h>
#include <kj/parse/common.h>

struct ParseTest
  : testing::Test {
  
  ParseTest() {  
  }

  ~ParseTest() noexcept {
  }
};

TEST_F(ParseTest, Bom) {

  constexpr auto whitespaceChar = kj::parse::anyOfChars(" \n\r\t");
  constexpr auto whitespace = kj::parse::many(kj::parse::anyOfChars(" \n\r\t"));
  constexpr auto discardWhitespace = kj::parse::discard(kj::parse::many(kj::parse::discard(kj::parse::anyOfChars(" \n\r\t"))));
  
  using Input = kj::parse::IteratorInput<char, const char*>;

  auto amp = kj::parse::transform(kj::parse::exactString("amp"), []{ return '&'; });
  auto apos = kj::parse::transform(kj::parse::exactString("apos"), []{ return '\''; });
  auto quot = kj::parse::transform(kj::parse::exactString("quot"), []{ return '\"'; });
  auto lt = kj::parse::transform(kj::parse::exactString("lt"), []{ return '<'; });
  auto gt = kj::parse::transform(kj::parse::exactString("gt"), []{ return '>'; });

  auto escape =
    kj::parse::sequence(
      kj::parse::discard(kj::parse::exactChar<'&'>()),
      kj::parse::oneOf(
        amp, apos, quot, lt, gt
      ),
      kj::parse::discard(kj::parse::exactChar<';'>())
    );

  auto attrName =
    kj::parse::many(kj::parse::anyOfChars(" \n\r\t/<>=?!").invert());

  auto attrValue =
    kj::parse::oneOf(
      kj::parse::sequence(
        kj::parse::exactChar<'\''>(),
        kj::parse::many(
          kj::parse::oneOf(
	    escape,
	    kj::parse::anyOfChars("\'").invert()
	  )
        ),
        kj::parse::exactChar<'\''>()
      ),
      kj::parse::sequence(
        kj::parse::exactChar<'\"'>(),
        kj::parse::many(
          kj::parse::oneOf(
	    escape,
	    kj::parse::anyOfChars("\"").invert()
	  )
        ),
        kj::parse::exactChar<'\"'>()
      )
    );

  auto attr =
    kj::parse::sequence(
      attrName,
      discardWhitespace,
      kj::parse::exactChar<'='>(),
      discardWhitespace,
      attrValue
    );

  auto attrSet = kj::parse::many(
    kj::parse::sequence(
      kj::parse::discard(whitespaceChar),
      discardWhitespace,
      attr
    )
  );

  auto bom =
    kj::parse::discard(
      kj::parse::exactString("\xEF\xBB\xBF")
    );

  auto decl =
    kj::parse::sequence(
      kj::parse::exactString("<?"),
      kj::parse::oneOf(
	kj::parse::exactString("XML"),	
	kj::parse::exactString("xml")
      ),
      attrSet,
      discardWhitespace,
      kj::parse::exactString("?>")
    );
			
  auto doctypeSubsection =
    kj::parse::many(
      kj::parse::anyOfChars("[]").invert()
    );
  
  auto doctype =
    kj::parse::sequence(
      kj::parse::exactString("<!DOCTYPE"),
      whitespace,
      kj::parse::exactChar<'>'>()
    );

  auto comment =
    kj::parse::sequence(
      kj::parse::exactString("<!--"),
      kj::parse::many(kj::parse::anyOfChars("").invert()),
      kj::parse::exactString("-->")
    );

  auto parser =
    kj::parse::sequence(
      kj::parse::optional(bom),
      discardWhitespace,
      kj::parse::optional(decl),
      kj::parse::optional(doctype)
    );

  {
    constexpr auto txt = "\xEF\xBB\xBF"_kj;
    Input iter(txt.begin(), txt.end());
    parser(iter);
    EXPECT_TRUE(iter.atEnd());
  }

  {
    constexpr auto txt = ""_kj;
    Input iter(txt.begin(), txt.end());
    parser(iter);
    EXPECT_TRUE(iter.atEnd());
  }
  {
    constexpr auto txt = "<?xml ?>"_kj;
    Input iter(txt.begin(), txt.end());
    parser(iter);
    EXPECT_TRUE(iter.atEnd());
  }

  {
    constexpr auto txt = "foo='bar'"_kj;
    Input iter(txt.begin(), txt.end());
    auto tokens = KJ_REQUIRE_NONNULL(attr(iter));
    EXPECT_TRUE(iter.atEnd());
    KJ_LOG(INFO, kj::get<0>(tokens));
    KJ_LOG(INFO, kj::get<1>(tokens));
  }

  {
    constexpr auto txt = " zero='aaa' one=\"bbb\""_kj;
    Input iter(txt.begin(), txt.end());
    auto attrs = KJ_REQUIRE_NONNULL(attrSet(iter));
    EXPECT_TRUE(iter.atEnd());
    EXPECT_EQ(kj::get<0>(attrs[0]), "zero"_kj);
    EXPECT_EQ(kj::get<1>(attrs[0]), "aaa"_kj);
    EXPECT_EQ(kj::get<0>(attrs[1]), "one"_kj);
    EXPECT_EQ(kj::get<1>(attrs[1]), "bbb"_kj);
  }

  {
    constexpr auto txt = "&quot;"_kj;
    Input iter(txt.begin(), txt.end());
    auto tokens = KJ_REQUIRE_NONNULL(escape(iter));
    EXPECT_EQ(kj::get<0>(tokens), '\"');
    EXPECT_TRUE(iter.atEnd());
  }

  {
    constexpr auto txt = "\'foo&lt;&gt;&quot;&\'"_kj;
    Input iter(txt.begin(), txt.end());
    auto token = KJ_REQUIRE_NONNULL(attrValue(iter));
    KJ_LOG(INFO, token);
    EXPECT_EQ(token, "foo<>\"&"_kj);
    EXPECT_TRUE(iter.atEnd());
  }
  {
    constexpr auto txt = "\'foo&lt;&gt;&quot;&\'"_kj;
    Input iter(txt.begin(), txt.end());
    auto token = KJ_REQUIRE_NONNULL(attrValue(iter));
    KJ_LOG(INFO, token);
    EXPECT_EQ(token, "foo<>\"&"_kj);
    EXPECT_TRUE(iter.atEnd());
  }
  {
    constexpr auto txt = R"(<?xml foo="bar"?>)"_kj;
    Input iter(txt.begin(), txt.end());
    auto attrs = KJ_REQUIRE_NONNULL(decl(iter));
    EXPECT_TRUE(iter.atEnd());
  }
  /*
  {
    constexpr auto txt = R"(<!-- foo -->)"_kj;
    Input iter(txt.begin(), txt.end());
    auto attrs = KJ_REQUIRE_NONNULL(comment(iter));
    EXPECT_TRUE(iter.atEnd());
  }
  */
}

int main(int argc, char* argv[]) {
  kj::TopLevelProcessContext processCtx{argv[0]};
  processCtx.increaseLoggingVerbosity();

  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

