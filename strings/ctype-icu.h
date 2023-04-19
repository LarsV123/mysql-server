#ifndef CTYPE_ICU_H
#define CTYPE_ICU_H
#include <m_ctype.h>
#include <stdio.h>
#include <unicode/errorcode.h>
#include <unicode/regex.h>
#include <unicode/tblcoll.h>
#include <unicode/uchar.h>
#include <unicode/ucol.h>
#include <unicode/unistr.h>
#include <unicode/unorm2.h>
#include <unicode/usearch.h>
#include <unicode/ustring.h>
#include <unicode/utypes.h>
#include <cstring>
#include <iostream>
#include <map>
#include "unicode/coll.h"

#include "mb_wc.h"

// Util functions and variables
void log(const char *file, const char *msg);
// const bool ICU_DEBUG = true;
const bool ICU_DEBUG = false;

// How many extra tailoring rules to apply
const int TAILORING_PREFIX_SIZE = 5;

icu::UnicodeString getRulePrefix();

// Thread local collator objects
icu::RuleBasedCollator *get_collator(const CHARSET_INFO *cs);

// Init and unload functions
bool icu_coll_init(CHARSET_INFO *cs, MY_CHARSET_LOADER *loader);
void icu_coll_uninit(CHARSET_INFO *cs);

// Charset functions
size_t icu_caseup(const CHARSET_INFO *cs, char *src, size_t srclen, char *dst,
                  size_t dstlen);
size_t icu_casedn(const CHARSET_INFO *cs, char *src, size_t srclen, char *dst,
                  size_t dstlen);

// Collation functions
int icu_strnncoll_utf8(const CHARSET_INFO *cs, const uchar *s, size_t slen,
                       const uchar *t, size_t tlen, bool t_is_prefix);

int icu_strnncollsp(const CHARSET_INFO *cs, const uchar *s, size_t slen,
                    const uchar *t, size_t tlen);

// This creates sort keys for the given string
template <class Mb_wc, int LEVELS_FOR_COMPARE>
size_t icu_strnxfrm_tmpl(const CHARSET_INFO *cs, const Mb_wc mb_wc, uchar *dst,
                         size_t dstlen, const uchar *src, size_t srclen,
                         uint flags);

// This is a wrapper for icu_strnxfrm_utf8 which sets the template parameters
size_t icu_strnxfrm(const CHARSET_INFO *cs, uchar *dst, size_t dstlen,
                    uint num_codepoints, const uchar *src, size_t srclen,
                    uint flags);

#endif  // CTYPE_ICU_H