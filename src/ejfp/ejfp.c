//
// ejfp.c
//
// Created: 2023-05-20
//  Author: Dmitry Murashov (dmtr <DOT> murashov <AT> geoscan.aero)
//

#define JSMN_STATIC  // Import quirks

#include "ejfp.h"
#include <jsmn/jsmn.h>

void ejfpInitialize(Ejfp *aEjfp)
{
	jsmn_init(&aEjfp->jsmnParser);
}
