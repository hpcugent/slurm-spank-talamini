/*
 * Plugin to set the PBS_NODEFILE env variable and its contents for the job
 *
 * Copyright 2018-2018 Ghent University
 *
 * This file is part of slurm-spank-talamini
 * originally created by the HPC team of Ghent University (http://ugent.be/hpc/en),
 * with support of Ghent University (http://ugent.be/hpc),
 * the Flemish Supercomputer Centre (VSC) (https://www.vscentrum.be),
 * the Flemish Research Foundation (FWO) (http://www.fwo.be/en)
 * and the Department of Economy, Science and Innovation (EWI) (http://www.ewi-vlaanderen.be/en).
 *
 * https://github.com/hpcugent/slurm-spank-talamini
 *
 * slurm-spank-talamini is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Library General Public License as
 * published by the Free Software Foundation, either version 2 of
 * the License, or (at your option) any later version.
 *
 * slurm-spank-talamini is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with vsc-jobs. If not, see <http://www.gnu.org/licenses/>.
 *
 * Author: Andy Georges
 *
 * Loosely based on Spank plugin private-tmpdir (c) HPC2N.umu.se
 *
 * This plugin generates the PBS_NODEFILE in the $TMPDIR of the job. We assume
 * other plugins or job epilog will clean these up, so we do not remove files
 * ourselves.
 */

/* Needs to be defined before first invocation of features.h so enable
 * it early. */
#define _GNU_SOURCE        /* See feature_test_macros(7) */
#define _XOPEN_SOURCE 500

#include <pwd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <slurm/spank.h>
#include <unistd.h>


SPANK_PLUGIN(env-test, 1);

// Gobals
static char *generate_pbsnodefile_command;
static int init_opts = 0;
static int init_post_opt = 0;
static int init_complete = 0;


int slurm_spank_init(spank_t sp, int ac, char **av)
{
    int i = 0;

    if (init_opts) {
        return ESPANK_SUCCESS;
    }
    init_opts = 1;

    for(i = 0; i < ac; i++) {
        if(strncmp(av[i], "command=", 8) == 0) {
            const char *optarg = av[i] + 8;
            if (!strlen(optarg)) {
                slurm_error("spank: pbs_nodefile: no generation command specified");
                return ESPANK_ERROR;
            }

            generate_pbsnodefile_command = strdup(optarg);

            if(!generate_pbsnodefile_command) {
                slurm_error("spank: pbs_nodefile: cannot malloc to store command");
                return ESPANK_ERROR;
            }
        }
    }

    slurm_debug("spank: pbs_nodefile: command: %s", generate_pbsnodefile_command);

    init_complete = 1;
    return ESPANK_SUCCESS;
}

int slurm_spank_exit(spank_t sp, int ac, char **av) {

    if (init_complete) {
        free(generate_pbsnodefile_command);
        slurm_debug("spank: pbs_nodefile: freed generate_pbsnodefile_command");
    }

    return ESPANK_SUCCESS;
}

/*
 *  Called from both srun and slurmd after the init is complete
 */
int slurm_spank_init_post_opt(spank_t sp, int ac, char **av)
{
    const int len = 32768;
    char *command = NULL;
    char taskcount[64];
    spank_err_t err;
    int pos = 0;                // End of (current) command buffer
    int written = 0;            // Total number of characters (bytes) written into the command buffer

    FILE *fp;                   // Communication with the subprocess
    char path[256];             // Store the path name returned by the subprocess. File is in $TMPDIR, which is short :)
    char user[256];             // Store the user name to chown the file
    struct passwd* pwd;

    if (init_post_opt) {
        return ESPANK_SUCCESS;
    }
    init_post_opt = 1;

    slurm_debug("spank: pbs_nodefile: slurm_spank_init_post_opt");

    if (spank_context() != S_CTX_REMOTE) {
        slurm_debug("spank: pbs_nodefile: not doing anything in non remote context");
        return ESPANK_SUCCESS;
    }

    if ((err = spank_getenv(sp, "USER", user, 256)) != ESPANK_SUCCESS) {
        slurm_debug("spank: pbs_nodefile: unable to retrieve user name");
        return ESPANK_ERROR;
    }

    command = malloc(len);
    if (command == NULL) {
        slurm_debug("spank: pbs_nodefile: unable to allocate memory for storing command");
        return ESPANK_ERROR;
    }
    pos = sprintf(command, "2>/dev/null /usr/bin/env -i SLURM_NODELIST='");
    if (pos < 0) {
        slurm_error("spank: pbs_nodefile: cannot write command to buffer");
        free(command);
        return ESPANK_ERROR;
    }

    if ((err = spank_getenv(sp, "SLURM_NODELIST", command + pos, len)) != ESPANK_SUCCESS) {
        slurm_error("spank: pbs_nodefile: could not get SLURM_NODELIST: error %d", err);
        free(command);
        return err;
    }
    slurm_debug("spank: pbs_nodefile: SLURM_NODELIST: %s", command + pos);
    pos = strlen(command);

    if ((err = spank_getenv(sp, "SLURM_TASKS_PER_NODE", taskcount, len)) != ESPANK_SUCCESS) {
        slurm_error("spank: pbs_nodefile: could not get SLURM_TASKS_PER_NODE: error %d", err);
        free(command);
        return err;
    }
    slurm_debug("spank: pbs_nodefile: SLURM_TASKS_PER_NODE: %s", taskcount);

    written = snprintf(command + pos, len - pos - 1,"' SLURM_TASKS_PER_NODE='%s' %s", taskcount, generate_pbsnodefile_command);
    slurm_debug("spank: pbs_nodefile: command %s", command);
    if (written >= len - pos - 2) {
        slurm_error("spank: pbs_nodefile: tried to write more than %d bytes to buffer, aborting.", len);
        return ESPANK_ERROR;
    }

    fp = popen(command, "r");
    if (fp == NULL) {
        slurm_error("spank: pbs_nodefile: Cannot execute command %s", generate_pbsnodefile_command);
        return ESPANK_ERROR;
    }

    if (fscanf(fp, "%s", path) != 1) {
        slurm_error("spank: pbs_nodefile: could not get name of temporary");
        pclose(fp);
        free(command);
        return ESPANK_ERROR;
    }
    slurm_debug("spank: pbs_nodefile: PBS_NODELIST available in %s", path);
    pclose(fp);
    free(command);

    if ((err = spank_setenv(sp, "PBS_NODEFILE", path, 1)) != ESPANK_SUCCESS) {
        slurm_error("spank: pbs_nodefile: cannot set PBS_NODEFILE env variable for job: error %d", err);
        return err;
    }

    pwd = getpwnam(user);
    if (pwd == NULL) {
        slurm_error("spank: pbs_nodefile: cannot get UID for user %s", user);
        return ESPANK_ERROR;
    }

    if(chown(path, pwd->pw_uid, pwd->pw_gid) != 0) {
        slurm_error("spank: pbs_nodefile: cannot chown %s to uid %d, gid %d", path, pwd->pw_uid, pwd->pw_gid);
        return ESPANK_ERROR;
    }

    return ESPANK_SUCCESS;
}
