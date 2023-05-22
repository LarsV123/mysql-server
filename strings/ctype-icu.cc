#include "ctype-icu.h"
#include "ctype-icu-tailorings.h"

const char *CTYPE_ICU_FILENAME = "ctype-icu.cc";
void log(const char *file [[maybe_unused]], const char *msg [[maybe_unused]]) {
  if (ICU_DEBUG) {
    printf("%s: %s \n", file, msg);
  }
}

// Common status for all collators
thread_local icu::ErrorCode STATUS = icu::ErrorCode();

// Map of collators for all locales (per thread)
thread_local std::unordered_map<uint, icu::RuleBasedCollator *> COLL_MAP =
    std::unordered_map<uint, icu::RuleBasedCollator *>();

icu::UnicodeString getRulePrefix() {
  // Hardcoded prefix for all tailoring rules
  // This is a hack to simulate adding different prefixes to the tailoring,
  // which could be used to restore the original collation order if there were
  // changes to the root collation order in a newer version of ICU.
  // This is not a complete solution, but it's enough for a prototype.

  // In a production system, this could use the ICU version to determine
  // which strings to add to the prefix.
  if (U_ICU_VERSION_MAJOR_NUM > 60) {
    log(CTYPE_ICU_FILENAME, "ICU version > 60");
  } else {
    log(CTYPE_ICU_FILENAME, "ICU version <= 60");
  }
  if (U_ICU_VERSION_MAJOR_NUM > 65) {
    log(CTYPE_ICU_FILENAME, "ICU version > 65");
  } else {
    log(CTYPE_ICU_FILENAME, "ICU version <= 65");
  }
  if (U_ICU_VERSION_MAJOR_NUM > 66 && U_ICU_VERSION_MINOR_NUM > 0) {
    log(CTYPE_ICU_FILENAME, "ICU version > 66");
  } else {
    log(CTYPE_ICU_FILENAME, "ICU version <= 66");
  }
  if (U_ICU_VERSION_MAJOR_NUM > 66 && U_ICU_VERSION_MINOR_NUM > 1) {
    log(CTYPE_ICU_FILENAME, "ICU version > 66.1");
  } else {
    log(CTYPE_ICU_FILENAME, "ICU version <= 66.1");
  }
  if (U_ICU_VERSION_MAJOR_NUM > 66 && U_ICU_VERSION_MINOR_NUM > 1 &&
      U_ICU_VERSION_PATCHLEVEL_NUM > 1) {
    log(CTYPE_ICU_FILENAME, "ICU version > 66.1.1");
  } else {
    log(CTYPE_ICU_FILENAME, "ICU version <= 66.1.1");
  }
  if (U_ICU_VERSION_MAJOR_NUM > 70) {
    log(CTYPE_ICU_FILENAME, "ICU version > 70");
  } else {
    log(CTYPE_ICU_FILENAME, "ICU version <= 70");
  }

  // This is an example prefix, where we gradually build a prefix to compensate
  // for different changes/fixes in the root collation order.

  // The changes are nonsense, but should have at least as much performance
  // impact as real changes.
  // These rules essentially move arbitrary emojis to the end of the collation.

  // Reset at the last valid code point
  auto r0 = icu::UnicodeString("&\U00010FFFD");

  auto r1 = icu::UnicodeString(
      "<🐶<🐱<🐭<🐹<🐰<🦊<🐻<🐼<🐨<🐯"
      "<🦁<🐮<🐷<🦄<🐴<🐗<🐺<🐸<🐒<🦍");

  auto r2 = icu::UnicodeString(
      "<🦌<🦒<🦓<🦏<🦛<🦘<🦙<🦞<🦩<🦚"
      "<🦜<🦢<🦆<🦉<🦔<🦇<🦋<🐜<🐌<🦑");

  auto r3 = icu::UnicodeString(
      "<🦐<🦀<🐍<🦎<🐢<🦕<🦖<🦟<🦗<🕷"
      "<🕸<🦂<🦠<🦨<🦦<🦥<🦡<🦧<🐊<🐅");

  auto r4 = icu::UnicodeString(
      "<🐆<🦈<🌵<🌴<🌲<🌳<🌺<🌸<🌻<🌼"
      "<🌷<🍂<🍁<🍄<🍀<🌿<🌱<🍃<🌾<🌰");

  auto r5 = icu::UnicodeString(
      "<🍇<🍈<🍉<🍊<🍋<🍌<🍍<🍎<🍏<🍐"
      "<🍑<🍒<🍓<🥝<🍅<🥑<🍆<🥔<🥕<🌽");

  if (ICU_DEBUG) {
    printf("TAILORING_PREFIX_SIZE: %d\n", TAILORING_PREFIX_SIZE);
  }
  switch (TAILORING_PREFIX_SIZE) {
    case 0:
      return icu::UnicodeString();
    case 1:
      return r0 + r1;
    case 2:
      return r0 + r1 + r2;
    case 3:
      return r0 + r1 + r2 + r3;
    case 4:
      return r0 + r1 + r2 + r3 + r4;
    case 5:
      return r0 + r1 + r2 + r3 + r4 + r5;
    default:
      log(CTYPE_ICU_FILENAME, "Invalid TAILORING_PREFIX_SIZE");
      assert(false);  // Should never happen
      return icu::UnicodeString();
  }
}

icu::UnicodeString getRules(const CHARSET_INFO *cs) {
  // Hardcoded mapping of collation number to tailoring
  // Used because other parts of the system expect the tailoring in a different
  // format than ICU does, so we can't put it in the CHARSET_INFO struct.
  // Should be converted to a map or something, but this is enough
  // for a prototype.

  switch (cs->number) {
    case 324:
      return icu::UnicodeString(ICU_NB_NO);
    case 325:
      return icu::UnicodeString(ICU_EN_US);
    case 326:
      return icu::UnicodeString(ICU_EN_US);
    case 327:
      return icu::UnicodeString(ICU_FR_FR);
    case 328:
      return icu::UnicodeString(ICU_ZH_HANS);
    case 329:
      return icu::UnicodeString(ICU_JA_JP);
    case 330:
      return icu::UnicodeString(ICU_JA_JP);
    default:
      assert(false);
      return icu::UnicodeString("");
  }
}

bool icu_coll_init(const CHARSET_INFO *cs) {
  log(CTYPE_ICU_FILENAME, "Creating new collator");

  if (ICU_DEBUG) {
    printf("  Collation name: %s\n", cs->m_coll_name);
    printf("  Tailoring: %s\n", cs->comment);
    printf("  ICU version: %s\n", U_ICU_VERSION);
  }

  // Create collator from tailoring
  auto prefix = getRulePrefix();
  auto rules = getRules(cs);
  auto tailoring = prefix + rules;
  icu::RuleBasedCollator *collator =
      new icu::RuleBasedCollator(tailoring, STATUS);

  // Set comparison level
  // https://unicode-org.github.io/icu/userguide/collation/concepts.html#comparison-levels
  switch (cs->levels_for_compare) {
    case 1:
      // Equivalent to ai_ci (accent insensitive, case insensitive)
      log(CTYPE_ICU_FILENAME, "Setting collator strength to PRIMARY");
      collator->setStrength(icu::Collator::PRIMARY);
      break;
    case 2:
      // Equivalent to as_ci (accent sensitive, case insensitive)
      log(CTYPE_ICU_FILENAME, "Setting collator strength to SECONDARY");
      collator->setStrength(icu::Collator::SECONDARY);
      break;
    case 3:
      // Equivalent to as_cs (accent sensitive, case sensitive)
      log(CTYPE_ICU_FILENAME, "Setting collator strength to TERTIARY");
      collator->setStrength(icu::Collator::TERTIARY);
      break;
    case 4:
      // Equivalent to as_cs_ks (accent, case and kana sensitive)
      log(CTYPE_ICU_FILENAME, "Setting collator strength to QUATERNARY");
      collator->setStrength(icu::Collator::QUATERNARY);
      break;
    default:
      // Compare by code point (equivalent to binary)
      // Significant performance cost for comparisons and sort key generation
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

icu::RuleBasedCollator *get_collator(const CHARSET_INFO *cs) {
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
  auto collator = get_collator(cs);

  // Create StringPieces from the input strings
  icu::StringPiece sp1 =
      icu::StringPiece(reinterpret_cast<const char *>(s), slen);
  icu::StringPiece sp2 =
      icu::StringPiece(reinterpret_cast<const char *>(t), tlen);

  // Compare the two strings
  int cmp = collator->compareUTF8(sp1, sp2, STATUS);
  return cmp;
}

icu::UnicodeString convert_utf8_to_utf16(const uchar *src, size_t srclen) {
  icu::StringPiece sp(reinterpret_cast<const char *>(src), srclen);
  return icu::UnicodeString::fromUTF8(sp);
}

// Makes a sort key suitable for memcmp() for the given string
// Equivalent to my_strnxfrm_uca_900_tmpl
template <class Mb_wc, int LEVELS_FOR_COMPARE>
static size_t icu_strnxfrm_tmpl(const CHARSET_INFO *cs,
                                const Mb_wc mb_wc [[maybe_unused]], uchar *dst,
                                size_t dstlen, const uchar *src, size_t srclen,
                                uint flags [[maybe_unused]]) {
  log(CTYPE_ICU_FILENAME, "icu_strnxfrm_tmpl");

  // Convert the input string from UTF-8 to UTF-16
  auto input = convert_utf8_to_utf16(src, srclen);

  // Get a collator for this locale
  auto collator = get_collator(cs);

  // Generate a sort key and the length of the required buffer
  size_t expectedLen = collator->getSortKey(input, dst, dstlen);

  if (expectedLen > dstlen) {
    // Output buffer is too small and gets filled to capacity
    // We don't know if this is a perfect fit or an overflow
    log(CTYPE_ICU_FILENAME, "icu_strnxfrm_tmpl: output buffer too small");

    // Delete the generated UnicodeString
    input.remove();
    return dstlen;
  }

  // Delete the generated UnicodeString
  input.remove();
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