#include "ctype-icu.h"
#include <stdio.h>
#include <iostream>

#include <unicode/errorcode.h>
#include <unicode/regex.h>
#include <unicode/unistr.h>
#include <unicode/utypes.h>

void say_hello() {
  icu::UnicodeString hello("Hello, ICU! This is ctype-icu.cc");
  std::string hello_utf8;
  hello.toUTF8String(hello_utf8);
  std::cout << hello_utf8 << std::endl;
}

// icu::UnicodeString hello("Hello, ICU!");
// std::cout << hello.toUTF8String() << std::endl;

// icu::UnicodeString hello("Hello, ICU!");
// std::string hello_utf8;
// hello.toUTF8String(hello_utf8);
// std::cout << hello_utf8 << std::endl;

// const char *str1 = "hello";
// const char *str2 = "world";

// UErrorCode error = U_ZERO_ERROR;
// auto collator = Collator::createInstance(error);

// if (collator->compare(str1, str2) < 0)
//   printf("str1 comes before str2\n");
// else
//   printf("str2 comes before str1\n");

// delete collator;
// caseup - converts the given string to uppercase using length
// size_t icu_caseup_utf8mb4(const CHARSET_INFO *cs, char *src, size_t srclen,
//                           char *dst, size_t dstlen) {
//   printf("icu_caseup_utf8mb4 called in ctype-icu.cc\n");

//   // Convert the input UTF-8 string to a UnicodeString
//   icu::UnicodeString input = icu::UnicodeString::fromUTF8(src);

//   // Convert the UnicodeString to upper case
//   input.toUpper();

//   // Convert the upper case UnicodeString to UTF-8 and copy it to the output
//   // buffer
//   size_t outputSize = input.extract(0, input.length(), dst, dstlen, "UTF-8");

//   return outputSize;
// }
