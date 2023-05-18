//
// error.c
//
// Created: 2023-05-18
//  Author: Dmitry Murashov (dmtr <DOT> murashov <AT> geoscan.aero)
//

#include "ejfp/error.h"

static EjfpError sEjfpError = 0;

EjfpError ejfpErrorCode()
{
	return sEjfpError;
}

void ejfpSetErrorCode(EjfpError aEjfpError)
{
	sEjfpError = aEjfpError;
}
