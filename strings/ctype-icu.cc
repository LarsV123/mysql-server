#include "ctype-icu.h"
#include <stdio.h>
#include <cstring>
#include <iostream>
#include <map>

#include "mb_wc.h"

const char *CTYPE_ICU_FILENAME = "ctype-icu.cc";
void log(const char *file [[maybe_unused]], const char *msg [[maybe_unused]]) {
  if (ICU_DEBUG) {
    printf("%s: %s \n", file, msg);
  }
}

// Common status for all collators
thread_local icu::ErrorCode STATUS = icu::ErrorCode();

// Map of collators for all locales (per thread)
thread_local std::unordered_map<uint, icu::Collator *> COLL_MAP =
    std::unordered_map<uint, icu::Collator *>();

bool icu_coll_init(const CHARSET_INFO *cs) {
  // TODO: Implement tailoring

  log(CTYPE_ICU_FILENAME, "Creating new collator");
  icu::Locale locale = icu::Locale(cs->comment);

  if (ICU_DEBUG) {
    printf("  Locale: %s\n", locale.getName());
  }

  icu::Collator *collator = icu::Collator::createInstance(locale, STATUS);

  // Set comparison level
  switch (cs->levels_for_compare) {
    case 1:
      log(CTYPE_ICU_FILENAME, "Setting collator strength to PRIMARY");
      collator->setStrength(icu::Collator::PRIMARY);
      break;
    case 2:
      log(CTYPE_ICU_FILENAME, "Setting collator strength to SECONDARY");
      collator->setStrength(icu::Collator::SECONDARY);
      break;
    case 3:
      log(CTYPE_ICU_FILENAME, "Setting collator strength to TERTIARY");
      collator->setStrength(icu::Collator::TERTIARY);
      break;
    default:
      log(CTYPE_ICU_FILENAME, "Setting collator strength to IDENTICAL");
      collator->setStrength(icu::Collator::IDENTICAL);
      break;
  }
  COLL_MAP[cs->number] = collator;
  return true;
}
void icu_coll_uninit(CHARSET_INFO *cs [[maybe_unused]]) {
  // Delete all collators
  log(CTYPE_ICU_FILENAME, "icu_coll_uninit");
  for (auto &[key, value] : COLL_MAP) {
    delete value;
  }
};

icu::Collator *get_collator(const CHARSET_INFO *cs) {
  log(CTYPE_ICU_FILENAME, "get_collator");

  // Check if the collator is already in the map
  if (COLL_MAP.find(cs->number) != COLL_MAP.end()) {
    log(CTYPE_ICU_FILENAME, "Collator already exists");
  } else {
    icu_coll_init(cs);
  }
  return COLL_MAP[cs->number];
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

  // Get a collator for this locale
  icu::Collator *collator = get_collator(cs);

  // Create StringPieces from the input strings
  icu::StringPiece sp1 =
      icu::StringPiece(reinterpret_cast<const char *>(s), slen);
  icu::StringPiece sp2 =
      icu::StringPiece(reinterpret_cast<const char *>(t), tlen);

  // Compare the two strings
  int cmp = collator->compareUTF8(sp1, sp2, STATUS);
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

  auto sp = icu::StringPiece(reinterpret_cast<const char *>(src), srclen);
  auto input = icu::UnicodeString::fromUTF8(sp);

  // Get a collator for this locale
  icu::Collator *collator = get_collator(cs);

  // Generate a sort key and the length of the required buffer
  size_t expectedLen = collator->getSortKey(input, dst, dstlen);

  if (expectedLen > dstlen) {
    // Output buffer is too small and gets filled to capacity
    // We don't know if this is a perfect fit or an overflow
    log(CTYPE_ICU_FILENAME, "icu_strnxfrm_tmpl: output buffer too small");
    return dstlen;
  }

  return expectedLen;
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