/*
 * Plugin to log the environment the job receives, for testing purposes.
 *
 * (C) 2018 Ghent University
 *
 * Author: Andy Georges
 *
 * Loosely based on Spank plugin private-tmpdir (c) HPC2N.umu.se
 *
 */

/* Needs to be defined before first invocation of features.h so enable
 * it early. */
#define _GNU_SOURCE        /* See feature_test_macros(7) */
#define _XOPEN_SOURCE 500

#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <slurm/spank.h>
#include <unistd.h>

SPANK_PLUGIN(env-test, 1);

// Default

// Globals

/*
 *  Called from both srun and slurmd.
 */
int slurm_spank_init(spank_t sp, int ac, char **av)
{
    char **env = environ;

    slurm_debug("SPANK [env-test]: slurm_spank_init");

    if (spank_context() == S_CTX_LOCAL) {
    	slurm_debug("SPANK [env-test]: executing in local context");
    } else if (spank_context() == S_CTX_REMOTE) {
    	slurm_debug("SPANK [env-test]: executing in remote context");
    }

    while (*env != NULL) {
        slurm_debug2("SPANK [env-test]: %s", *env);
        env++;
    }

    return ESPANK_SUCCESS;
}

int slurm_spank_exit(spank_t sp, int ac, char **av) {
    return ESPANK_SUCCESS;
}


int slurm_spank_job_prolog(spank_t sp, int ac, char**av) {
    char **env = environ;

    slurm_debug("SPANK [env-test]: slurm_spank_job_prolog");

    if (spank_context() == S_CTX_LOCAL) {
    	slurm_debug("SPANK [env-test]: executing in local context");
    } else if (spank_context() == S_CTX_REMOTE) {
    	slurm_debug("SPANK [env-test]: executing in remote context");
    }

    while (*env != NULL) {
        slurm_debug2("SPANK [env-test]: env var: %s", *env);
        env++;
    }

}
