#ifndef CTYPE_ICU_H
#define CTYPE_ICU_H
#include <m_ctype.h>

void say_hello();

// Charset functions
size_t icu_caseup(const CHARSET_INFO *cs, char *src, size_t srclen, char *dst,
                  size_t dstlen);
size_t icu_casedn(const CHARSET_INFO *cs, char *src, size_t srclen, char *dst,
                  size_t dstlen);

// Collation functions
// size_t icu_strnxfrm(const CHARSET_INFO *cs, uchar *dst, size_t dstlen,
//                     uint num_codepoints, const uchar *src, size_t srclen,
//                     uint flags);

int icu_strnncoll(const CHARSET_INFO *cs, const uchar *s, size_t slen,
                  const uchar *t, size_t tlen, bool t_is_prefix);
int icu_strnncollsp(const CHARSET_INFO *cs, const uchar *s, size_t slen,
                    const uchar *t, size_t tlen);
#endif  // CTYPE_ICU_H