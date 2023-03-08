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

thread_local ICU_COLLATOR *COLL_STRUCT = nullptr;
thread_local std::unordered_map<uint, ICU_COLLATOR *> COLL_MAP =
    std::unordered_map<uint, ICU_COLLATOR *>();

bool icu_coll_init(const CHARSET_INFO *cs) {
  // TODO: Implement tailoring

  log(CTYPE_ICU_FILENAME, "Creating new collator");
  COLL_STRUCT = new ICU_COLLATOR();
  icu::Locale locale = icu::Locale(cs->comment);

  if (ICU_DEBUG) {
    printf("  Locale: %s\n", locale.getName());
  }

  UErrorCode status = U_ZERO_ERROR;
  icu::Collator *collator = icu::Collator::createInstance(locale, status);

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
  COLL_STRUCT->status = status;
  COLL_STRUCT->collator = collator;
  COLL_MAP[cs->number] = COLL_STRUCT;
  return true;
}
void icu_coll_uninit(CHARSET_INFO *cs [[maybe_unused]]){
    // TODO: Implement this? Do we need to clean up anything?
};

ICU_COLLATOR *get_collator(const CHARSET_INFO *cs) {
  log(CTYPE_ICU_FILENAME, "get_collator");

  // Check if the collator is already in the map
  if (COLL_MAP.find(cs->number) != COLL_MAP.end()) {
    log(CTYPE_ICU_FILENAME, "Collator already exists");
    COLL_STRUCT = COLL_MAP[cs->number];
  } else {
    icu_coll_init(cs);
  }
  return COLL_STRUCT;
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
  auto collator_struct = get_collator(cs);
  auto collator = collator_struct->collator;
  auto status = collator_struct->status;

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

  auto sp = icu::StringPiece(reinterpret_cast<const char *>(src), srclen);
  auto input = icu::UnicodeString::fromUTF8(sp);

  // Get a collator for this locale
  auto collator = get_collator(cs)->collator;

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