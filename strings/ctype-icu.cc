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
