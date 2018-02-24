/* The MIT License (MIT)
 *
 * Copyright (c) 2014-2018 David Medina and Tim Warburton
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 */

#include "occa/tools/testing.hpp"

#include "token.hpp"
#include "tokenizer.hpp"
#include "processingStages.hpp"

void testSkipMethods();
void testPushPop();
void testPeekMethods();
void testTokenMethods();
void testCommentSkipping();
void testStringMethods();
void testStringMerging();
void testPrimitiveMethods();
void testErrors();

using namespace occa::lang;

//---[ Util Methods ]-------------------
std::string source;
tokenizer_t tokenizer;
stringTokenMerger stringMerger;
occa::stream<token_t*> mergeTokenStream = tokenizer.map(stringMerger);
token_t *token = NULL;

void setStream(const std::string &s) {
  delete token;
  token = NULL;
  source = s;
  tokenizer.set(source.c_str());
}

void getToken() {
  delete token;
  token = NULL;
  tokenizer >> token;
}

void getStringMergeToken() {
  delete token;
  token = NULL;
  mergeTokenStream >> token;
}

void setToken(const std::string &s) {
  setStream(s);
  getToken();
}

std::string getHeader(const std::string &s) {
  setStream(s);
  return tokenizer.getHeader();
}

int getTokenType() {
  return token ? token->type() : 0;
}
//======================================

//---[ Macro Util Methods ]-------------
#define testStringPeek(s, encoding_)            \
  setStream(s);                                 \
  OCCA_ASSERT_EQUAL_BINARY(                     \
    (encoding_ << tokenType::encodingShift) |   \
    tokenType::string,                          \
    tokenizer.peek())

#define testCharPeek(s, encoding_)              \
  setStream(s);                                 \
  OCCA_ASSERT_EQUAL_BINARY(                     \
    (encoding_ << tokenType::encodingShift) |   \
    tokenType::char_,                           \
    tokenizer.peek())

#define testStringToken(s, encoding_)                         \
  setToken(s);                                                \
  OCCA_ASSERT_EQUAL_BINARY(tokenType::string,                 \
                           getTokenType());                   \
  OCCA_ASSERT_EQUAL_BINARY(encoding_,                         \
                           token->to<stringToken>().encoding)

#define testStringMergeToken(s, encoding_)                    \
  setStream(s);                                               \
  getStringMergeToken();                                      \
  OCCA_ASSERT_EQUAL_BINARY(tokenType::string,                 \
                           getTokenType());                   \
  OCCA_ASSERT_EQUAL_BINARY(encoding_,                         \
                           token->to<stringToken>().encoding)

#define testCharToken(s, encoding_)                         \
  setToken(s);                                              \
  OCCA_ASSERT_EQUAL_BINARY(tokenType::char_,                \
                           getTokenType())                  \
  OCCA_ASSERT_EQUAL_BINARY(encoding_,                       \
                           token->to<charToken>().encoding)

#define testStringValue(s, value_)              \
  setStream(s);                                 \
  getToken();                                   \
  testNextStringValue(value_)

#define testStringMergeValue(s, value_)         \
  setStream(s);                                 \
  getStringMergeToken();                        \
  testNextStringValue(value_)

#define testNextStringValue(value_)             \
  OCCA_ASSERT_EQUAL_BINARY(                     \
    tokenType::string,                          \
    getTokenType()                              \
  );                                            \
  OCCA_ASSERT_EQUAL(                            \
    value_,                                     \
    token->to<stringToken>().value              \
  )

#define testPrimitiveToken(type_, value_)       \
  getToken();                                   \
  OCCA_ASSERT_EQUAL_BINARY(                     \
    tokenType::primitive,                       \
    getTokenType());                            \
  OCCA_ASSERT_EQUAL(                            \
    value_,                                     \
    (type_) token->to<primitiveToken>().value   \
  )
//======================================

//---[ Tests ]--------------------------
int main(const int argc, const char **argv) {
  testSkipMethods();
  testPushPop();
  testPeekMethods();
  testTokenMethods();
  testCommentSkipping();
  testStringMethods();
  testStringMerging();
  testPrimitiveMethods();
  testErrors();

  return 0;
}

void testSkipMethods() {
  setStream("ab\nc\n\n\n\n\n\nd\ne");
  const char *c = source.c_str();

  tokenizer.skipTo('a');
  OCCA_ASSERT_EQUAL('a', *tokenizer.fp.start);

  tokenizer.skipTo('b');
  OCCA_ASSERT_EQUAL('b' , *tokenizer.fp.start);

  tokenizer.skipTo('e');
  OCCA_ASSERT_EQUAL('e' , *tokenizer.fp.start);

  tokenizer.fp.start = c;
  tokenizer.skipTo("c\n");
  OCCA_ASSERT_EQUAL(c + 2, tokenizer.fp.start);

  tokenizer.fp.start = c + 6;
  tokenizer.skipFrom("\n");
  OCCA_ASSERT_EQUAL('d' , *tokenizer.fp.start);
}

void testPushPop() {
  setStream("a\nb\nc\nd\ne");
  const char *c = source.c_str();

  tokenizer.push();
  tokenizer.skipTo('c');
  OCCA_ASSERT_EQUAL(3,
                    tokenizer.fp.line);
  OCCA_ASSERT_EQUAL(c + 4,
                    tokenizer.fp.start);
  tokenizer.popAndRewind();
  OCCA_ASSERT_EQUAL(1,
                    tokenizer.fp.line);
  OCCA_ASSERT_EQUAL(c + 0,
                    tokenizer.fp.start);
  tokenizer.push();
  tokenizer.push();
  tokenizer.push();
  tokenizer.skipTo('c');
  tokenizer.pop();
  tokenizer.pop();
  tokenizer.pop();
  OCCA_ASSERT_EQUAL(3,
                    tokenizer.fp.line);
  OCCA_ASSERT_EQUAL(c + 4,
                    tokenizer.fp.start);
}

void testPeekMethods() {
  setStream("abcd");
  OCCA_ASSERT_EQUAL_BINARY(
    tokenType::identifier,
    tokenizer.peek()
  );
  setStream("_abcd");
  OCCA_ASSERT_EQUAL_BINARY(
    tokenType::identifier,
    tokenizer.peek()
  );
  setStream("_abcd020230");
  OCCA_ASSERT_EQUAL_BINARY(
    tokenType::identifier,
    tokenizer.peek()
  );

  setStream("1");
  OCCA_ASSERT_EQUAL_BINARY(
    tokenType::primitive,
    tokenizer.peek()
  );
  setStream("2.0");
  OCCA_ASSERT_EQUAL_BINARY(
    tokenType::primitive,
    tokenizer.peek()
  );
  setStream("2.0e10");
  OCCA_ASSERT_EQUAL_BINARY(
    tokenType::primitive,
    tokenizer.peek()
  );
  setStream(".0e10-10");
  OCCA_ASSERT_EQUAL_BINARY(
    tokenType::primitive,
    tokenizer.peek()
  );

  setStream("+");
  OCCA_ASSERT_EQUAL_BINARY(
    tokenType::op,
    tokenizer.peek()
  );
  setStream("<");
  OCCA_ASSERT_EQUAL_BINARY(
    tokenType::op,
    tokenizer.peek()
  );
  setStream("->*");
  OCCA_ASSERT_EQUAL_BINARY(
    tokenType::op,
    tokenizer.peek()
  );
  setStream("==");
  OCCA_ASSERT_EQUAL_BINARY(
    tokenType::op,
    tokenizer.peek()
  );

  testStringPeek("\"\""                , 0);
  testStringPeek("\"string\\\"string\"", 0);
  testStringPeek("R\"(string)\""       , encodingType::R);
  testStringPeek("u8\"string\""        , encodingType::u8);
  testStringPeek("u\"string\""         , encodingType::u);
  testStringPeek("U\"string\""         , encodingType::U);
  testStringPeek("L\"string\""         , encodingType::L);
  testStringPeek("u8R\"(string)\""     , (encodingType::u8 |
                                          encodingType::R));
  testStringPeek("uR\"(string)\""      , (encodingType::u |
                                          encodingType::R));
  testStringPeek("UR\"(string)\""      , (encodingType::U |
                                          encodingType::R));
  testStringPeek("LR\"(string)\""      , (encodingType::L |
                                          encodingType::R));

  testCharPeek("'a'"    , 0);
  testCharPeek("'\\''"  , 0);
  testCharPeek("u'\\''" , encodingType::u);
  testCharPeek("U'\\''" , encodingType::U);
  testCharPeek("L'\\''" , encodingType::L);

  OCCA_ASSERT_EQUAL("foobar",
                    getHeader("<foobar>"));
  OCCA_ASSERT_EQUAL("foobar",
                    getHeader("\"foobar\""));
}

void testTokenMethods() {
  setToken("abcd");
  OCCA_ASSERT_EQUAL_BINARY(
    tokenType::identifier,
    getTokenType()
  );
  setToken("_abcd");
  OCCA_ASSERT_EQUAL_BINARY(
    tokenType::identifier,
    getTokenType()
  );
  setToken("_abcd020230");
  OCCA_ASSERT_EQUAL_BINARY(
    tokenType::identifier,
    getTokenType()
  );

  setToken("true");
  OCCA_ASSERT_EQUAL_BINARY(
    tokenType::primitive,
    getTokenType()
  );
  setToken("false");
  OCCA_ASSERT_EQUAL_BINARY(
    tokenType::primitive,
    getTokenType()
  );

  setToken("1");
  OCCA_ASSERT_EQUAL_BINARY(
    tokenType::primitive,
    getTokenType()
  );
  setToken("2.0");
  OCCA_ASSERT_EQUAL_BINARY(
    tokenType::primitive,
    getTokenType()
  );
  setToken("2.0e10");
  OCCA_ASSERT_EQUAL_BINARY(
    tokenType::primitive,
    getTokenType()
  );
  setToken(".0e10-10");
  OCCA_ASSERT_EQUAL_BINARY(
    tokenType::primitive,
    getTokenType()
  );

  setToken("+");
  OCCA_ASSERT_EQUAL_BINARY(
    tokenType::op,
    getTokenType()
  );
  setToken("<");
  OCCA_ASSERT_EQUAL_BINARY(
    tokenType::op,
    getTokenType()
  );
  setToken("->*");
  OCCA_ASSERT_EQUAL_BINARY(
    tokenType::op,
    getTokenType()
  );
  setToken("==");
  OCCA_ASSERT_EQUAL_BINARY(
    tokenType::op,
    getTokenType()
  );

  testStringToken("\"\""                , 0);
  testStringToken("\"string\\\"string\"", 0);
  testStringToken("R\"(string)\""       , encodingType::R);
  testStringToken("u8\"string\""        , encodingType::u8);
  testStringToken("u\"string\""         , encodingType::u);
  testStringToken("U\"string\""         , encodingType::U);
  testStringToken("L\"string\""         , encodingType::L);
  testStringToken("u8R\"(string)\""     , (encodingType::u8 |
                                           encodingType::R));
  testStringToken("uR\"(string)\""      , (encodingType::u |
                                           encodingType::R));
  testStringToken("UR\"(string)\""      , (encodingType::U |
                                           encodingType::R));
  testStringToken("LR\"(string)\""      , (encodingType::L |
                                           encodingType::R));

  testCharToken("'a'"    , 0);
  testCharToken("'\\''"  , 0);
  testCharToken("u'\\''" , encodingType::u);
  testCharToken("U'\\''" , encodingType::U);
  testCharToken("L'\\''" , encodingType::L);
}

void addEncodingPrefixes(std::string &s1,
                         std::string &s2,
                         std::string &s3,
                         const std::string &encoding1,
                         const std::string &encoding2) {
  const std::string a  = "\"a\"";
  const std::string b  = "\"b\"";
  s1 = (encoding1 + a) + " " + b;
  s2 = a               + " " + (encoding2 + b);
  s3 = (encoding1 + a) + " " + (encoding2 + b);
}

void testCommentSkipping() {
  setStream("// test    \n"
            "1.0");
  OCCA_ASSERT_EQUAL_BINARY(
    tokenType::newline,
    tokenizer.peek()
  );
  getToken();
  OCCA_ASSERT_EQUAL_BINARY(
    tokenType::primitive,
    tokenizer.peek()
  );
  setStream("/* 2.0 */ foo");
  OCCA_ASSERT_EQUAL_BINARY(
    tokenType::identifier,
    tokenizer.peek()
  );
  setStream("/*         \n"
            "2.0        \n"
            "*/ foo");
  OCCA_ASSERT_EQUAL_BINARY(
    tokenType::identifier,
    tokenizer.peek()
  );
}

void testStringMethods() {
  // Test values
  testStringValue("\"\""                , "");
  testStringValue("\"\\\"\""            , "\"");
  testStringValue("\"string\\\"string\"", "string\"string");
  testStringValue("R\"(string)\""       , "string");
  testStringValue("u8\"string\""        , "string");
  testStringValue("u\"string\""         , "string");
  testStringValue("U\"string\""         , "string");
  testStringValue("L\"string\""         , "string");
  testStringValue("u8R\"(string)\""     , "string");
  testStringValue("uR\"(string)\""      , "string");
  testStringValue("UR\"(string)\""      , "string");
  testStringValue("LR\"(string)\""      , "string");

  // Test raw strings
  testStringValue("R\"*(string)*\""  , "string");
  testStringValue("u8R\"*(string)*\"", "string");
  testStringValue("uR\"*(string)*\"" , "string");
  testStringValue("UR\"*(string)*\"" , "string");
  testStringValue("LR\"*(string)*\"" , "string");

  testStringValue("R\"foo(string)foo\""  , "string");
  testStringValue("u8R\"foo(string)foo\"", "string");
  testStringValue("uR\"foo(string)foo\"" , "string");
  testStringValue("UR\"foo(string)foo\"" , "string");
  testStringValue("LR\"foo(string)foo\"" , "string");
}

void testStringMerging() {
  // Test string concatination
  const std::string ab = "ab";
  std::string s1, s2, s3;
  addEncodingPrefixes(s1, s2, s3, "", "");
  testStringMergeValue(s1, ab);
  testStringMergeValue(s2, ab);
  testStringMergeValue(s3, ab);
  testStringMergeToken(s1, 0);
  testStringMergeToken(s2, 0);
  testStringMergeToken(s3, 0);

  addEncodingPrefixes(s1, s2, s3, "", "u");
  testStringMergeValue(s1, ab);
  testStringMergeValue(s2, ab);
  testStringMergeValue(s3, ab);
  testStringMergeToken(s1, 0);
  testStringMergeToken(s2, encodingType::u);
  testStringMergeToken(s3, encodingType::u);

  addEncodingPrefixes(s1, s2, s3, "", "U");
  testStringMergeValue(s1, ab);
  testStringMergeValue(s2, ab);
  testStringMergeValue(s3, ab);
  testStringMergeToken(s1, 0);
  testStringMergeToken(s2, encodingType::U);
  testStringMergeToken(s3, encodingType::U);

  addEncodingPrefixes(s1, s2, s3, "", "L");
  testStringMergeValue(s1, ab);
  testStringMergeValue(s2, ab);
  testStringMergeValue(s3, ab);
  testStringMergeToken(s1, 0);
  testStringMergeToken(s2, encodingType::L);
  testStringMergeToken(s3, encodingType::L);

  addEncodingPrefixes(s1, s2, s3, "u", "U");
  testStringMergeValue(s1, ab);
  testStringMergeValue(s2, ab);
  testStringMergeValue(s3, ab);
  testStringMergeToken(s1, encodingType::u);
  testStringMergeToken(s2, encodingType::U);
  testStringMergeToken(s3, encodingType::U);

  addEncodingPrefixes(s1, s2, s3, "u", "L");
  testStringMergeValue(s1, ab);
  testStringMergeValue(s2, ab);
  testStringMergeValue(s3, ab);
  testStringMergeToken(s1, encodingType::u);
  testStringMergeToken(s2, encodingType::L);
  testStringMergeToken(s3, encodingType::L);

  addEncodingPrefixes(s1, s2, s3, "U", "L");
  testStringMergeValue(s1, ab);
  testStringMergeValue(s2, ab);
  testStringMergeValue(s3, ab);
  testStringMergeToken(s1, encodingType::U);
  testStringMergeToken(s2, encodingType::L);
  testStringMergeToken(s3, encodingType::L);
}

void testPrimitiveMethods() {
  setStream("1 68719476735L +0.1 .1e-10 -4.5L");
  testPrimitiveToken(int, 1);
  testPrimitiveToken(int64_t, 68719476735L);
  getToken();
  testPrimitiveToken(float, 0.1);
  testPrimitiveToken(float, .1e-10);
  getToken();
  testPrimitiveToken(double, 4.5L);
}

void testErrors() {
  unknownTokenFilter unknownFilter(true);

  setStream("$ test\n\"foo\" \"bar\" ` bar foo");
  occa::stream<token_t*> streamWithErrors = (
    tokenizer.filter(unknownFilter)
  );

  std::cerr << "Testing error outputs:\n";
  while (!streamWithErrors.isEmpty()) {
    token = NULL;
    streamWithErrors >> token;
    delete token;
  }
}
//======================================
