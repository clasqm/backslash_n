#ifndef TYPES__JNG
#define TYPES__JNG

/* Haiku already defines int8/uint8/int16/uint16/int32/uint32 and status_t in
 * <SupportDefs.h>.  The original BeOS header defined them locally, with
 * "long" for int32, which is wrong on 64-bit Haiku where long is 64 bits wide.
 */
#include <SupportDefs.h>

const	int JNG_OK = 0;
const	int JNG_ERROR = -1;

#endif
