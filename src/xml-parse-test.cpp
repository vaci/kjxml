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
}

int main(int argc, char* argv[]) {
  kj::TopLevelProcessContext processCtx{argv[0]};
  processCtx.increaseLoggingVerbosity();

  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

