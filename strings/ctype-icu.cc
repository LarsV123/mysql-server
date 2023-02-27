#include "ctype-icu.h"
#include <stdio.h>
#include <cstring>
#include <iostream>

#include <unicode/errorcode.h>
#include <unicode/regex.h>
#include <unicode/uchar.h>
#include <unicode/ucol.h>
#include <unicode/unistr.h>
#include <unicode/unorm2.h>
#include <unicode/usearch.h>
#include <unicode/ustring.h>
#include <unicode/utypes.h>
#include "mb_wc.h"
#include "unicode/coll.h"

const char *CTYPE_ICU_FILENAME = "ctype-icu.cc";
void log(const char *file [[maybe_unused]], const char *msg [[maybe_unused]]) {
  printf("%s: %s \n", file, msg);
}

// See examples at
// https://github.com/unicode-org/icu/blob/main/icu4c/source/samples/ustring/ustring.cpp

///////////////////////////////////////////////////////////////////////////
// Charset functions
///////////////////////////////////////////////////////////////////////////

// caseup - converts the given string to uppercase using length
// Equivalent to my_caseup_utf8mb4
size_t icu_caseup(const CHARSET_INFO *cs [[maybe_unused]], char *src,
                  size_t srclen [[maybe_unused]], char *dst, size_t dstlen) {
  log(CTYPE_ICU_FILENAME, "icu_caseup");
  // TODO: Add collator for locale support
  // TODO: Split this into caseup_str and caseup

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
// Equivalent to my_casedn_utf8mb4
size_t icu_casedn(const CHARSET_INFO *cs [[maybe_unused]], char *src,
                  size_t srclen [[maybe_unused]], char *dst, size_t dstlen) {
  log(CTYPE_ICU_FILENAME, "icu_casedn");
  // TODO: Add collator for locale support
  // TODO: Split this into casedn_str and casedn

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

// strnncoll - compares two strings
int icu_strnncoll_utf8(const CHARSET_INFO *cs [[maybe_unused]], const uchar *s,
                       size_t slen, const uchar *t, size_t tlen,
                       bool t_is_prefix [[maybe_unused]]) {
  log(CTYPE_ICU_FILENAME, "icu_strnncoll_utf8");
  return icu_strnncollsp(cs, s, slen, t, tlen);
}

// strnncollsp - compares two strings (ignoring trailing spaces)
int icu_strnncollsp(const CHARSET_INFO *cs [[maybe_unused]], const uchar *s,
                    size_t slen, const uchar *t, size_t tlen) {
  log(CTYPE_ICU_FILENAME, "icu_strnncollsp");

  // Create a collator for the given locale
  static UErrorCode status;
  static icu::Collator *collator;
  if (collator == nullptr) {
    log(CTYPE_ICU_FILENAME, "Creating collator in icu_strnncollsp");
    status = U_ZERO_ERROR;
    icu::Locale locale = icu::Locale(cs->comment);
    collator = icu::Collator::createInstance(locale, status);

    // Set comparison level
    switch (cs->levels_for_compare) {
      case 1:
        collator->setStrength(icu::Collator::PRIMARY);
        break;
      case 2:
        collator->setStrength(icu::Collator::SECONDARY);
        break;
      case 3:
        collator->setStrength(icu::Collator::TERTIARY);
        break;
      default:
        collator->setStrength(icu::Collator::IDENTICAL);
        break;
    }

  } else {
    log(CTYPE_ICU_FILENAME, "Collator already created in icu_strnncollsp");
  }

  // Create StringPieces from the input strings
  icu::StringPiece sp1 =
      icu::StringPiece(reinterpret_cast<const char *>(s), slen);
  icu::StringPiece sp2 =
      icu::StringPiece(reinterpret_cast<const char *>(t), tlen);

  // Compare the two strings
  int cmp = collator->compareUTF8(sp1, sp2, status);
  return cmp;
}

// Makes a sort key suitable for memcmp() for the given string
// Equivalent to my_strnxfrm_uca_900_tmpl
template <class Mb_wc, int LEVELS_FOR_COMPARE>
static size_t icu_strnxfrm_tmpl(const CHARSET_INFO *cs,
                                const Mb_wc mb_wc [[maybe_unused]], uchar *dst,
                                size_t dstlen, const uchar *src, size_t srclen,
                                uint flags [[maybe_unused]]) {
  log(CTYPE_ICU_FILENAME, "icu_strnxfrm_tmpl");

  UErrorCode status = U_ZERO_ERROR;
  const char *locale_str = cs->comment;
  icu::Locale locale(locale_str);

  auto sp = icu::StringPiece(reinterpret_cast<const char *>(src), srclen);
  auto input = icu::UnicodeString::fromUTF8(sp);

  // Create a collator for the given locale
  icu::Collator *collator = icu::Collator::createInstance(locale, status);

  // Set the collation strength based on the number of levels for comparison
  switch (LEVELS_FOR_COMPARE) {
    case 1:
      collator->setStrength(icu::Collator::PRIMARY);
      break;
    case 2:
      collator->setStrength(icu::Collator::SECONDARY);
      break;
    case 3:
      collator->setStrength(icu::Collator::TERTIARY);
      break;
    default:
      collator->setStrength(icu::Collator::IDENTICAL);
      break;
  }

  // Generate the sort key using the collator
  // FIXME: Is sending nullptr here correct?
  int32_t sort_key_length = collator->getSortKey(input, nullptr, 0);
  if (dstlen < static_cast<size_t>(sort_key_length)) {
    // Output buffer is too small, return required size
    delete collator;
    return sort_key_length;
  }
  collator->getSortKey(input, reinterpret_cast<uint8_t *>(dst),
                       sort_key_length);
  delete collator;

  return static_cast<size_t>(sort_key_length);
}

// Equivalent to my_strnxfrm_uca_900
size_t icu_strnxfrm(const CHARSET_INFO *cs, uchar *dst, size_t dstlen,
                    uint num_codepoints [[maybe_unused]], const uchar *src,
                    size_t srclen, uint flags) {
  log(CTYPE_ICU_FILENAME, "icu_strnxfrm");
  switch (cs->levels_for_compare) {
    case 1:
      return icu_strnxfrm_tmpl<Mb_wc_utf8mb4, 1>(cs, Mb_wc_utf8mb4(), dst,
                                                 dstlen, src, srclen, flags);
    case 2:
      return icu_strnxfrm_tmpl<Mb_wc_utf8mb4, 2>(cs, Mb_wc_utf8mb4(), dst,
                                                 dstlen, src, srclen, flags);
    default:
      assert(false);
    case 3:
      return icu_strnxfrm_tmpl<Mb_wc_utf8mb4, 3>(cs, Mb_wc_utf8mb4(), dst,
                                                 dstlen, src, srclen, flags);
    case 4:
      return icu_strnxfrm_tmpl<Mb_wc_utf8mb4, 4>(cs, Mb_wc_utf8mb4(), dst,
                                                 dstlen, src, srclen, flags);
  }
}