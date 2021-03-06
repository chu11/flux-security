/************************************************************\
 * Copyright 2017 Lawrence Livermore National Security, LLC
 * (c.f. AUTHORS, NOTICE.LLNS, COPYING)
 *
 * This file is part of the Flux resource manager framework.
 * For details, see https://github.com/flux-framework.
 *
 * SPDX-License-Identifier: LGPL-3.0
\************************************************************/

#include <stdlib.h>

#include "testconfig.h"

/*
 *  For build-tree/test IMP only! Return config pattern from environment
 *   if set, otherwise use built-in "test" configuration pattern, which
 *   will point to src/imp/imp.conf.d
 */
const char * imp_get_config_pattern (void)
{
    const char *p = getenv ("FLUX_IMP_CONFIG_PATTERN");
    if (p == NULL)
         p = imp_config_pattern; /* From testconfig.h */
    return (p);
}

/*  For build-tree/test IMP, return the same config path for
 *   libflux-security as flux-imp. This is what the tests expect
 *   and makes test writing easier (only one env var needed to
 *   override config)
 */
const char * imp_get_security_config_pattern (void)
{
    return imp_get_config_pattern ();
}

/*
 *  vi: ts=4 sw=4 expandtab
 */
