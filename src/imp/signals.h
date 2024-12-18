/************************************************************\
 * Copyright 2024 Lawrence Livermore National Security, LLC
 * (c.f. AUTHORS, NOTICE.LLNS, COPYING)
 *
 * This file is part of the Flux resource manager framework.
 * For details, see https://github.com/flux-framework.
 *
 * SPDX-License-Identifier: LGPL-3.0
\************************************************************/

#ifndef HAVE_IMP_SIGNALS_H
#define HAVE_IMP_SIGNALS_H 1

#include <sys/types.h>
#include "imp_state.h"

/*  Set the target of IMP signal forwarding. `pid` may be less than -1,
 *  in which case the entire process group `-pid` will be signaled.
 */
void imp_set_signal_child (pid_t pid);

/*  Setup RFC 15 standard IMP signal forwarding
 */
void imp_setup_signal_forwarding (struct imp_state *imp);

void imp_sigblock_all (void);

void imp_sigunblock_all (void);

/*  Set default signal disposition and then raise signal 'sig'.
 *  If raise fails for any reason, then exit with standard 128+sig.
 */
void imp_raise (int sig) __attribute__ ((noreturn));;

#endif /* !HAVE_IMP_SIGNALS_H */
