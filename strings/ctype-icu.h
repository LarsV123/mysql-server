#ifndef CTYPE_ICU_H
#define CTYPE_ICU_H
#include <m_ctype.h>

void say_hello();
size_t icu_caseup_utf8mb4(const CHARSET_INFO *cs, char *src, size_t srclen,
                          char *dst, size_t dstlen);

#endif  // CTYPE_ICU_H