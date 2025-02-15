/*
 * Copyright (c) 2014 .SE (The Internet Infrastructure Foundation).
 * Copyright (c) 2014 OpenDNSSEC AB (svb)
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
 * IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
 * IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#include <getopt.h>
#include "daemon/engine.h"
#include "cmdhandler.h"
#include "daemon/enforcercommands.h"
#include "log.h"
#include "str.h"
#include "clientpipe.h"
#include "longgetopt.h"
#include "policy/policy_export.h"

#include "policy/policy_export_cmd.h"

static const char *module_str = "policy_export_cmd";

/* TODO: add export to specific file */

static void
usage(int sockfd)
{
    client_printf(sockfd,
        "policy export\n"
        "	--policy <policy> | --all		aka -p | -a \n"
    );
}

static void
help(int sockfd)
{
    client_printf(sockfd,
        "Export a specified policy or all of them from the database.\n"
	"\nOptions:\n"
        "policy|all	limit the operation to a specified policy or all of them\n\n"
    );
}

static int
run(cmdhandler_ctx_type* context, int argc, char* argv[])
{
    int sockfd = context->sockfd;
    struct longgetopt optctx;
    int long_index = 0, opt = 0;
    const char* policy_name = NULL;
    int all = 0;
    policy_t* policy;
    db_connection_t* dbconn = getconnectioncontext(context);;

    static struct option long_options[] = {
        {"policy", required_argument, 0, 'p'},
        {"all", no_argument, 0, 'a'},
        {0, 0, 0, 0}
    };

    for(opt = longgetopt(argc, argv, "p:a", long_options, &long_index, &optctx); opt != -1;
        opt = longgetopt(argc, argv, NULL,  long_options, &long_index, &optctx)) {
        switch (opt) {
            case 'p':
                policy_name = optctx.optarg;
                break;
            case 'a':
                all = 1;
                break;
            default:
                client_printf_err(sockfd, "unknown arguments\n");
                ods_log_error("[%s] unknown arguments for policy export command", module_str);
                return -1;
        }
    }

    if (!dbconn) {
        return 1;
    }

    if (all) {
        if (policy_export_all(sockfd, dbconn, NULL) != POLICY_EXPORT_OK) {
            return 1;
        }
    }
    else if (policy_name) {
        if (!(policy = policy_new_get_by_name(dbconn, policy_name))) {
            client_printf_err(sockfd, "Unable to find policy %s!\n", policy_name);
            return 1;
        }
        if (policy_export(sockfd, policy, NULL) != POLICY_EXPORT_OK) {
            policy_free(policy);
            return 1;
        }
        policy_free(policy);
    }
    else {
        client_printf_err(sockfd, "Either --all or --policy needs to be given!\n");
        return 1;
    }

    return 0;
}

struct cmd_func_block policy_export_funcblock = {
    "policy export", &usage, &help, NULL, NULL, &run, NULL
};
