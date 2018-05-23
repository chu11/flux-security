/*****************************************************************************\
 *  Copyright (c) 2017 Lawrence Livermore National Security, LLC.  Produced at
 *  the Lawrence Livermore National Laboratory (cf, AUTHORS, DISCLAIMER.LLNS).
 *  LLNL-CODE-658032 All rights reserved.
 *
 *  This file is part of the Flux resource manager framework.
 *  For details, see https://github.com/flux-framework.
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License as published by the Free
 *  Software Foundation; either version 2 of the license, or (at your option)
 *  any later version.
 *
 *  Flux is distributed in the hope that it will be useful, but WITHOUT
 *  ANY WARRANTY; without even the IMPLIED WARRANTY OF MERCHANTABILITY or
 *  FITNESS FOR A PARTICULAR PURPOSE.  See the terms and conditions of the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
 *  See also:  http://www.gnu.org/licenses/
\*****************************************************************************/

#if HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#include <string.h>
#include <errno.h>

#include "imp_log.h"
#include "sudosim.h"

const char * sudo_user_name (void)
{
    if (getuid() == 0)
        return (getenv ("SUDO_USER"));
    return (NULL);
}

bool sudo_is_active (void)
{
    return (sudo_user_name() != NULL);
}

int sudo_simulate_setuid (void)
{
    const char *user = NULL;

    /*  Ignore SUDO_USER unless real UID is 0. We're then fairly sure this
     *   process was run under sudo, or someone with privileges wants to
     *   simulate running under sudo.
     */
    if ((user = sudo_user_name ())) {
        struct passwd *pwd = getpwnam (user);

        /*  Fail in the abnormal condition that SUDO_USER is not found.
         */
        if (pwd == NULL)
            return (-1);

        /*  O/w, set real UID/GID to the SUDO_USER credentials so it
         *   appears that this process is setuid.
         */
        if (setresgid (pwd->pw_gid, -1, -1) < 0) {
            imp_warn ("sudosim: setresgid: %s", strerror (errno));
            return (-1);
        }
        if (setresuid (pwd->pw_uid, -1, -1) < 0) {
            imp_warn ("sudosim: setresuid: %s", strerror (errno));
            return (-1);
        }
    }
    return (0);
}

/*
 * vi: ts=4 sw=4 expandtab
 */