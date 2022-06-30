/************************************************************\
 * Copyright 2022 Lawrence Livermore National Security, LLC
 * (c.f. AUTHORS, NOTICE.LLNS, COPYING)
 *
 * This file is part of the Flux resource manager framework.
 * For details, see https://github.com/flux-framework.
 *
 * SPDX-License-Identifier: LGPL-3.0
\************************************************************/

#ifndef HAVE_PIDINFO_H
#define HAVE_PIDINFO_H 1

struct pid_info {
    pid_t pid;
    uid_t pid_owner;
    char cg_path [4096];
    uid_t cg_owner;
};

struct pid_info *pid_info_create (pid_t pid);
void pid_info_destroy (struct pid_info *pi);

#endif /* !HAVE_PIDINFO_H */
