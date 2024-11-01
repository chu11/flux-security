/************************************************************\
 * Copyright 2024 Lawrence Livermore National Security, LLC
 * (c.f. AUTHORS, NOTICE.LLNS, COPYING)
 *
 * This file is part of the Flux resource manager framework.
 * For details, see https://github.com/flux-framework.
 *
 * SPDX-License-Identifier: LGPL-3.0
\************************************************************/

#if HAVE_CONFIG_H
#include "config.h"
#endif
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <dirent.h>
#include <ctype.h>
#include <stdbool.h>
#include <limits.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/vfs.h>
#ifdef HAVE_LINUX_MAGIC_H
#include <linux/magic.h>
#endif
#ifndef TMPFS_MAGIC
#define TMPFS_MAGIC 0x01021994 /* from linux/magic.h */
#endif
#ifndef CGROUP_SUPER_MAGIC
#define CGROUP_SUPER_MAGIC 0x27e0eb
#endif
#ifndef CGROUP2_SUPER_MAGIC
#define CGROUP2_SUPER_MAGIC 0x63677270
#endif
#include <signal.h>

#include "src/libutil/strlcpy.h"

#include "cgroup.h"
#include "imp_log.h"

static char *remove_leading_dotdot (char *relpath)
{
    while (strncmp (relpath, "/..", 3) == 0)
        relpath += 3;
    return relpath;
}

/*
 *  Look up the current cgroup relative path from /proc/self/cgroup.
 *
 *  If cgroup->unified is true, then look for the first entry where
 *   'subsys' is an empty string.
 *
 *  Otherwise, use the `name=systemd` line.
 *
 *  See NOTES: /proc/[pid]/cgroup in cgroups(7).
 */
static int cgroup_init_path (struct cgroup_info *cgroup)
{
    int rc = -1;
    int n;
    FILE *fp;
    size_t size = 0;
    char *line = NULL;
    int saved_errno;

    if (!(fp = fopen ("/proc/self/cgroup", "r")))
        return -1;

    while ((n = getline (&line, &size, fp)) >= 0) {
        char *nl;
        char *relpath = NULL;
        char *subsys = strchr (line, ':');
        if ((nl = strchr (line, '\n')))
            *nl = '\0';
        if (subsys == NULL
            || *(++subsys) == '\0'
            || !(relpath = strchr (subsys, ':')))
            continue;

        /* Nullify subsys, relpath is already nul-terminated at newline */
        *(relpath++) = '\0';

        /* Remove leading /.. in relpath. This could be due to cgroup
         * mounted in a container.
         */
        relpath = remove_leading_dotdot (relpath);

        /*  If unified cgroups are being used, then stop when we find
         *   subsys="". Otherwise stop at subsys="name=systemd":
         */
        if ((cgroup->unified && subsys[0] == '\0')
            || (!cgroup->unified && strcmp (subsys, "name=systemd") == 0)) {
            int len = sizeof (cgroup->path);
            if (snprintf (cgroup->path,
                          len,
                          "%s%s",
                          cgroup->mount_dir,
                          relpath) < len)
                rc = 0;
            break;
        }
    }
    if (rc < 0)
        errno = ENOENT;

    saved_errno = errno;
    free (line);
    fclose (fp);
    errno = saved_errno;
    return rc;
}

/*  Determine if this system is using the unified (v2) or legacy (v1)
 *   cgroups hierarchy (See https://systemd.io/CGROUP_DELEGATION/)
 *   and mount point for systemd managed cgroups.
 */
static int cgroup_init_mount_dir_and_type (struct cgroup_info *cg)
{
    struct statfs fs;

    /*  Assume unified unless we discover otherwise
     */
    cg->unified = true;

    /*  Check if either /sys/fs/cgroup or /sys/fs/cgroup/unified
     *   are mounted as type cgroup2. If so, use this as the mount dir
     *   (Note: these paths are guaranteed to fit in cg->mount_dir, so
     *    no need to check for truncation)
     */
    (void) strlcpy (cg->mount_dir, "/sys/fs/cgroup", sizeof (cg->mount_dir));
    if (statfs (cg->mount_dir, &fs) < 0)
        return -1;

    /* if cgroup2 fs mounted: unified hierarchy for all users of cgroupfs
     */
    if (fs.f_type == CGROUP2_SUPER_MAGIC)
        return 0;

    /*  O/w, check if cgroup2 unified hierarchy mounted at
     *   /sys/fs/cgroup/unified
     */
    (void) strlcpy (cg->mount_dir,
                    "/sys/fs/cgroup/unified",
                    sizeof (cg->mount_dir));
    if (statfs (cg->mount_dir, &fs) < 0)
        return -1;

    if (fs.f_type == CGROUP2_SUPER_MAGIC)
        return 0;

    /*  O/w, if /sys/fs/cgroup is mounted as tmpfs, we need to check
     *   for /sys/fs/cgroup/systemd mounted as cgroupfs (legacy).
     */
    if (fs.f_type == TMPFS_MAGIC) {

        (void) strlcpy (cg->mount_dir,
                        "/sys/fs/cgroup/systemd",
                        sizeof (cg->mount_dir));
        if (statfs (cg->mount_dir, &fs) == 0
            && fs.f_type == CGROUP_SUPER_MAGIC) {
            cg->unified = false;
            return 0;
        }
    }

    /*  Unable to determine cgroup mount point and/or unified vs legacy */
    return -1;
}

void cgroup_info_destroy (struct cgroup_info *cg)
{
    if (cg) {
        int saved_errno = errno;
        free (cg);
        errno = saved_errno;
    }
}

struct cgroup_info *cgroup_info_create (void)
{
    struct cgroup_info *cgroup = calloc (1, sizeof (*cgroup));
    if (!cgroup)
        return NULL;

    if (cgroup_init_mount_dir_and_type (cgroup) < 0
        || cgroup_init_path (cgroup) < 0) {
        cgroup_info_destroy (cgroup);
        return NULL;
    }
    /* Note: GNU basename(3) never modifies its argument. (_GNU_SOURCE
     * is defined in config.h.)
     */
    if (strncmp (basename (cgroup->path), "imp-shell", 9) == 0)
        cgroup->use_cgroup_kill = true;

    return cgroup;
}

int cgroup_kill (struct cgroup_info *cgroup, int sig)
{
    int count = 0;
    int rc = 0;
    int saved_errno = 0;
    char path [PATH_MAX+14]; /* cgroup->path[PATH_MAX] + "/cgroup.procs" */
    FILE *fp;
    unsigned long child;
    pid_t current_pid = getpid ();

    /* Note: path is guaranteed to have enough space to append "/cgroup.procs"
     */
    (void) snprintf (path, sizeof (path), "%s/cgroup.procs", cgroup->path);

    if (!(fp = fopen (path, "r")))
        return -1;
    while (fscanf (fp, "%lu", &child) == 1) {
        pid_t pid = child;
        if (pid == current_pid)
            continue;
        if (kill (pid, sig) < 0) {
            saved_errno = errno;
            rc = -1;
            imp_warn ("Failed to send signal %d to pid %lu",
                      sig,
                      child);
            continue;
        }
        count++;
    }
    fclose (fp);
    if (rc < 0 && count == 0) {
        count = -1;
        errno = saved_errno;
    }
    return count;
}

int cgroup_wait_for_empty (struct cgroup_info *cgroup)
{
    int n;

    /*  Only wait for empty cgroup if cgroup kill is enabled.
     */
    if (!cgroup->use_cgroup_kill)
        return 0;

    while ((n = cgroup_kill (cgroup, 0)) > 0) {
        /*  Note: inotify/poll() do not work on the cgroup.procs virtual
         *  file. Therefore, wait at most 1s and check to see if the cgroup
         *  is empty again. If the job execution system requests a signal to
         *  be delivered then the sleep will be interrupted, in which case a
         *  a small delay is added in hopes that any terminated processes
         *  will have been removed from cgroup.procs by then.
         */
        if (usleep (1e6) < 0 && errno == EINTR)
            usleep (2000);
    }
    return 0;
}

/* vi: ts=4 sw=4 expandtab
 */
