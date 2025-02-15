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

#include "config.h"
#include <limits.h>
#include <getopt.h>

#include "daemon/engine.h"
#include "cmdhandler.h"
#include "daemon/enforcercommands.h"
#include "log.h"
#include "str.h"
#include "clientpipe.h"
#include "longgetopt.h"
#include "enforcer/enforce_task.h"
#include "keystate/zonelist_import.h"
#include "keystate/zonelist_export.h"

#include "keystate/zonelist_import_cmd.h"

static const char *module_str = "zonelist_import_cmd";

static void
usage(int sockfd)
{
    client_printf(sockfd,
        "zonelist import\n"
        "	[--remove-missing-zones]		aka -r\n"
    /* We require the user to give an absolute path. The daemon
     * and the client might not have the same working directory. */
        "	[--file <absolute path>]		aka -f\n"
    );
}

static void
help(int sockfd)
{
    client_printf(sockfd,
        "Import zones from zonelist.xml into enforcer database.\n"
	"\nOptions:\n"
        "remove-missing-zones	Remove any zones from database not existed in zonelist file\n"
        "file			File to import, instead of zonelist file configured in conf.xml\n\n"
    );
}

static int
run(cmdhandler_ctx_type* context, int argc, char* argv[])
{
    int sockfd = context->sockfd;
    struct longgetopt optctx;
    char path[PATH_MAX];
    int ret, remove_missing_zones = 0;
    int long_index = 0, opt = 0;
    const char* zonelist_path = NULL;
    db_connection_t* dbconn = getconnectioncontext(context);
    engine_type* engine = getglobalcontext(context);

    static struct option long_options[] = {
        {"remove-missing-zones", no_argument, 0, 'r'},
        {"file", required_argument, 0, 'f'},
        {0, 0, 0, 0}
    };

    if (!engine || !engine->config ||
        !engine->config->zonelist_filename || !dbconn)
    {
        return 1;
    }

    for(opt = longgetopt(argc, argv, "rf:", long_options, &long_index, &optctx); opt != -1;
        opt = longgetopt(argc, argv, NULL,  long_options, &long_index, &optctx)) {
        switch (opt) {
            case 'r':
                remove_missing_zones = 1;
                break;
            case 'f':
                zonelist_path = optctx.optarg;
                break;
            default:
                client_printf_err(sockfd, "unknown arguments\n");
                ods_log_error("[%s] unknown arguments for zonelist import command", module_str);
                return -1;
        }
    }

    ret = zonelist_import(sockfd, engine, dbconn, remove_missing_zones, zonelist_path);
    if (ret == ZONELIST_IMPORT_NO_CHANGE) {
        return 0;
    } else if (ret != ZONELIST_IMPORT_OK) {
        return 1;
    }

    if (snprintf(path, sizeof(path), "%s/%s", engine->config->working_dir, OPENDNSSEC_ENFORCER_ZONELIST) >= (int)sizeof(path)
        || zonelist_export(sockfd, dbconn, path, 0) != ZONELIST_EXPORT_OK)
    {
        ods_log_error("[%s] internal zonelist export failed", module_str);
        client_printf_err(sockfd, "Unable to export the internal zonelist %s, updates will not reach the Signer!\n", path);
        return 1;
    } else {
        ods_log_info("[%s] internal zonelist exported successfully", module_str);
    }

    /* YBS Only flush for zones with changed policy */
    enforce_task_flush_all(engine, dbconn);

    return 0;
}

struct cmd_func_block zonelist_import_funcblock = {
    "zonelist import", &usage, &help, NULL, NULL, &run, NULL
};
