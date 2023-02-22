#include "ctype-icu.h"
#include <stdio.h>
#include <iostream>

#include <unicode/errorcode.h>
#include <unicode/regex.h>
#include <unicode/uchar.h>
#include <unicode/ucol.h>
#include <unicode/unistr.h>
#include <unicode/unorm2.h>
#include <unicode/ustring.h>
#include <unicode/utypes.h>

// See examples at
// https://github.com/unicode-org/icu/blob/main/icu4c/source/samples/ustring/ustring.cpp

void say_hello() {
  icu::UnicodeString hello("Hello, ICU! This is ctype-icu.cc");
  std::string hello_utf8;
  hello.toUTF8String(hello_utf8);
  std::cout << hello_utf8 << std::endl;
}

///////////////////////////////////////////////////////////////////////////
// Charset functions
///////////////////////////////////////////////////////////////////////////

// caseup - converts the given string to uppercase using length
size_t icu_caseup(const CHARSET_INFO *cs [[maybe_unused]], char *src,
                  size_t srclen [[maybe_unused]], char *dst, size_t dstlen) {
  printf("icu_caseup called in ctype-icu.cc\n");

  // Convert the input UTF-8 string to a UnicodeString
  icu::UnicodeString input = icu::UnicodeString::fromUTF8(src);

  // Convert the UnicodeString to upper case
  input.toUpper();

  // Convert the upper case UnicodeString to UTF-8 and copy it to the output
  // buffer
  size_t outputSize = input.extract(0, input.length(), dst, dstlen, "UTF-8");

  return outputSize;
}

// casedn - converts the given string to lowercase using length
size_t icu_casedn(const CHARSET_INFO *cs [[maybe_unused]], char *src,
                  size_t srclen [[maybe_unused]], char *dst, size_t dstlen) {
  printf("icu_casedn called in ctype-icu.cc\n");

  // Convert the input UTF-8 string to a UnicodeString
  icu::UnicodeString input = icu::UnicodeString::fromUTF8(src);

  // Convert the UnicodeString to upper case
  input.toLower();

  // Convert the upper case UnicodeString to UTF-8 and copy it to the output
  // buffer
  size_t outputSize = input.extract(0, input.length(), dst, dstlen, "UTF-8");

  return outputSize;
}

///////////////////////////////////////////////////////////////////////////
// Collation functions
///////////////////////////////////////////////////////////////////////////
// size_t icu_strnxfrm(const CHARSET_INFO *cs, uchar *dst, size_t dstlen,
//                     uint num_codepoints [[maybe_unused]], const uchar *src,
//                     size_t srclen, uint flags) {
//   printf("icu_strnxfrm called in ctype-icu.cc\n");

// }

// strnncoll - compares two strings (use in wrapper_strnncoll and
// wrapper_strnncollsp?)
// int icu_strnncoll(const CHARSET_INFO *cs [[maybe_unused]], const uchar *s,
//                   size_t slen [[maybe_unused]], const uchar *t,
//                   size_t tlen [[maybe_unused]],
//                   bool t_is_prefix [[maybe_unused]]) {
//   printf("icu_strnncoll called in ctype-icu.cc!\n");
//   // size_t len = std::min(slen, tlen);
//   // int cmp = len == 0 ? 0 : memcmp(s, t, len);
//   // return cmp ? cmp : (int)((t_is_prefix ? len : slen) - tlen);

//   // Create a collator for the given locale
//   icu::UErrorCode status = U_ZERO_ERROR;
//   icu::Collator *collator = icu::Collator::createInstance(status);

//   // Compare the two strings
//   icu::UnicodeString s1 = icu::UnicodeString::fromUTF8((const char *)s);
//   icu::UnicodeString s2 = icu::UnicodeString::fromUTF8((const char *)t);
//   int cmp = collator->compare(s1, s2);
//   return cmp;
// }