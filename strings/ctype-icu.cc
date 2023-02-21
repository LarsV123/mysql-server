#include "ctype-icu.h"
#include <stdio.h>
// #include <unicode/ucnv.h>
// #include <unicode/unistr.h>
// #include <unicode/ustream.h>
// #include <unicode/utypes.h>
// #include <unicode/ucase.h>
#include <unicode/coll.h>

#include <unicode/errorcode.h>
#include <unicode/regex.h>
#include <unicode/unistr.h>
#include <unicode/utypes.h>

void say_hello() { printf("Hello from ctype-icu.cc\n"); }

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
