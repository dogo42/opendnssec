#include "config.h"

#include "file.h"
#include "str.h"
#include "locks.h"
#include "log.h"
#include "status.h"
#include "util.h"
#include "longgetopt.h"
#include "daemon/engine.h"
#include "cmdhandler.h"
#include "signercommands.h"
#include "clientpipe.h"

static char const * cmdh_str = "cmdhandler";

static const char*
cmdargument(const char* cmd, const char* matchValue, const char* defaultValue)
{
    const char* s = cmd;
    if (!s)
        return defaultValue;
    while(*s && !isspace(*s))
        ++s;
    while(*s && isspace(*s))
        ++s;
    if(matchValue) {
        if (!strcmp(s,matchValue))
            return s;
        else
            return defaultValue;
    } else if(*s) {
        return s;
    } else {
        return defaultValue;
    }
}

/**
 * Handle the 'help' command.
 *
 */
static int
cmdhandler_handle_cmd_help(cmdhandler_ctx_type* context, char *cmd)
{
    int sockfd = context->sockfd;
    char buf[ODS_SE_MAXLINE];

    (void) snprintf(buf, ODS_SE_MAXLINE,
        "Commands:\n"
        "zones                       Show the currently known zones.\n"
        "sign <zone> [--serial <nr>] Read zone and schedule for immediate "
                                    "(re-)sign.\n"
        "                            If a serial is given, that serial is used "
                                    "in the output zone.\n"
        "sign --all                  Read all zones and schedule all for "
                                    "immediate (re-)sign.\n"
    );
    client_printf(sockfd, "%s", buf);

    (void) snprintf(buf, ODS_SE_MAXLINE,
        "clear <zone>                Delete the internal storage of this "
                                    "zone.\n"
        "                            All signatures will be regenerated "
                                    "on the next re-sign.\n"
        "queue                       Show the current task queue.\n"
        "flush                       Execute all scheduled tasks "
                                    "immediately.\n"
    );
    client_printf(sockfd, "%s", buf);

    (void) snprintf(buf, ODS_SE_MAXLINE,
        "update <zone>               Update this zone signer "
                                    "configurations.\n"
        "update [--all]              Update zone list and all signer "
                                    "configurations.\n"
        "retransfer <zone>           Retransfer the zone from the master.\n"
        "start                       Start the engine.\n"
        "running                     Check if the engine is running.\n"
        "reload                      Reload the engine.\n"
        "stop                        Stop the engine.\n"
        "verbosity <nr>              Set verbosity.\n"
    );
    client_printf(sockfd, "%s", buf);
    return 0;
}


/**
 * Handle the 'zones' command.
 *
 */
static int
cmdhandler_handle_cmd_zones(cmdhandler_ctx_type* context, char *cmd)
{
    int sockfd = context->sockfd;
    engine_type* engine;
    char buf[ODS_SE_MAXLINE];
    size_t i;
    ldns_rbnode_t* node = LDNS_RBTREE_NULL;
    zone_type* zone = NULL;
    engine = getglobalcontext(context);
    if (!engine->zonelist || !engine->zonelist->zones) {
        (void)snprintf(buf, ODS_SE_MAXLINE, "There are no zones configured\n");
        client_printf(sockfd, "%s", buf);
        return 0;
    }
    /* how many zones */
    pthread_mutex_lock(&engine->zonelist->zl_lock);
    (void)snprintf(buf, ODS_SE_MAXLINE, "There are %i zones configured\n",
        (int) engine->zonelist->zones->count);
    client_printf(sockfd, "%s", buf);
    /* list zones */
    node = ldns_rbtree_first(engine->zonelist->zones);
    while (node && node != LDNS_RBTREE_NULL) {
        zone = (zone_type*) node->data;
        for (i=0; i < ODS_SE_MAXLINE; i++) {
            buf[i] = 0;
        }
        (void)snprintf(buf, ODS_SE_MAXLINE, "- %s\n", zone->name);
        client_printf(sockfd, "%s", buf);
        node = ldns_rbtree_next(node);
    }
    pthread_mutex_unlock(&engine->zonelist->zl_lock);
    return 0;
}


/**
 * Handle the 'update' command.
 *
 */
static int
cmdhandler_handle_cmd_update(cmdhandler_ctx_type* context, char *cmd)
{
    int sockfd = context->sockfd;
    engine_type* engine;
    char buf[ODS_SE_MAXLINE];
    zone_type* zone = NULL;
    ods_status zl_changed = ODS_STATUS_OK;
    engine = getglobalcontext(context);
    ods_log_assert(engine);
    ods_log_assert(engine->taskq);
    if (cmdargument(cmd, "--all", NULL)) {
        pthread_mutex_lock(&engine->zonelist->zl_lock);
        zl_changed = zonelist_update(engine->zonelist,
            engine->config->zonelist_filename);
        if (zl_changed == ODS_STATUS_UNCHANGED) {
            (void)snprintf(buf, ODS_SE_MAXLINE, "Zone list has not changed."
                " Signer configurations updated.\n");
            client_printf(sockfd, "%s", buf);
        } else if (zl_changed == ODS_STATUS_OK) {
            (void)snprintf(buf, ODS_SE_MAXLINE, "Zone list updated: %i "
            "removed, %i added, %i updated.\n",
                engine->zonelist->just_removed,
                engine->zonelist->just_added,
                engine->zonelist->just_updated);
            client_printf(sockfd, "%s", buf);
        } else {
            pthread_mutex_unlock(&engine->zonelist->zl_lock);
            (void)snprintf(buf, ODS_SE_MAXLINE, "Zone list has errors.\n");
            client_printf(sockfd, "%s", buf);
        }
        if (zl_changed == ODS_STATUS_OK ||
            zl_changed == ODS_STATUS_UNCHANGED) {
            engine->zonelist->just_removed = 0;
            engine->zonelist->just_added = 0;
            engine->zonelist->just_updated = 0;
            pthread_mutex_unlock(&engine->zonelist->zl_lock);
            /**
              * Always update the signconf for zones, even if zonelist has
              * not changed: ODS_STATUS_OK.
              */
            engine_update_zones(engine, ODS_STATUS_OK);
        }
    } else {
        /* look up zone */
        pthread_mutex_lock(&engine->zonelist->zl_lock);
        zone = zonelist_lookup_zone_by_name(engine->zonelist, cmdargument(cmd, NULL, ""),
            LDNS_RR_CLASS_IN);
        /* If this zone is just added, don't update (it might not have a
         * task yet) */
        if (zone && zone->zl_status == ZONE_ZL_ADDED) {
            zone = NULL;
        }
        pthread_mutex_unlock(&engine->zonelist->zl_lock);

        if (!zone) {
            (void)snprintf(buf, ODS_SE_MAXLINE, "Error: Zone %s not found.\n",
                cmdargument(cmd, NULL, ""));
            client_printf(sockfd, "%s", buf);
            /* update all */
            cmdhandler_handle_cmd_update(context, "update --all");
            return 1;
        }

        pthread_mutex_lock(&zone->zone_lock);
        schedule_scheduletask(engine->taskq, TASK_FORCESIGNCONF, zone->name, zone, &zone->zone_lock, schedule_PROMPTLY);
        pthread_mutex_unlock(&zone->zone_lock);

        (void)snprintf(buf, ODS_SE_MAXLINE, "Zone %s config being updated.\n",
        cmdargument(cmd, NULL, ""));
        client_printf(sockfd, "%s", buf);
        ods_log_verbose("[%s] zone %s scheduled for immediate update signconf",
            cmdh_str, cmdargument(cmd, NULL, ""));
        engine_wakeup_workers(engine);
    }
    return 0;
}


/**
 * Handle the 'retransfer' command.
 *
 */
static int
cmdhandler_handle_cmd_retransfer(cmdhandler_ctx_type* context, char *cmd)
{
    int sockfd = context->sockfd;
    engine_type* engine;
    char buf[ODS_SE_MAXLINE];
    zone_type* zone = NULL;
    engine = getglobalcontext(context);
    ods_log_assert(engine->taskq);
    /* look up zone */
    pthread_mutex_lock(&engine->zonelist->zl_lock);
    zone = zonelist_lookup_zone_by_name(engine->zonelist, cmdargument(cmd, NULL, ""),
        LDNS_RR_CLASS_IN);
    /* If this zone is just added, don't retransfer (it might not have a
     * task yet) */
    if (zone && zone->zl_status == ZONE_ZL_ADDED) {
        zone = NULL;
    }
    pthread_mutex_unlock(&engine->zonelist->zl_lock);

    if (!zone) {
        (void)snprintf(buf, ODS_SE_MAXLINE, "Error: Zone %s not found.\n",
            cmdargument(cmd, NULL, ""));
        client_printf(sockfd, "%s", buf);
    } else if (zone->adinbound->type != ADAPTER_DNS) {
        (void)snprintf(buf, ODS_SE_MAXLINE,
            "Error: Zone %s not configured to use DNS input adapter.\n",
            cmdargument(cmd, NULL, ""));
        client_printf(sockfd, "%s", buf);
    } else {
        zone->xfrd->serial_retransfer = 1;
        xfrd_set_timer_now(zone->xfrd);
        ods_log_debug("[%s] forward a notify", cmdh_str);
        dnshandler_fwd_notify(engine->dnshandler,
            (uint8_t*) ODS_SE_NOTIFY_CMD, strlen(ODS_SE_NOTIFY_CMD));
        (void)snprintf(buf, ODS_SE_MAXLINE, "Zone %s being re-transfered.\n", cmdargument(cmd, NULL, ""));
        client_printf(sockfd, "%s", buf);
        ods_log_verbose("[%s] zone %s being re-transfered", cmdh_str, cmdargument(cmd, NULL, ""));
    }
    return 0;
}


static uint32_t
max(uint32_t a, uint32_t b)
{
    return (a<b?b:a);
}

static ods_status
forceread(engine_type* engine, zone_type *zone, int force_serial, uint32_t serial, int sockfd)
{
        pthread_mutex_lock(&zone->zone_lock);
        if (force_serial) {
            ods_log_assert(zone->db);
            if (!util_serial_gt(serial, max(zone->db->outserial,
                zone->db->inbserial))) {
                pthread_mutex_unlock(&zone->zone_lock);
                client_printf(sockfd, "Error: Unable to enforce serial %u for zone %s.\n", serial, zone->name);
                return 1;
            }
            zone->db->altserial = serial;
            zone->db->force_serial = 1;
        }
        schedule_scheduletask(engine->taskq, TASK_FORCEREAD, zone->name, zone, &zone->zone_lock, schedule_IMMEDIATELY);
        pthread_mutex_unlock(&zone->zone_lock);
        return 0;
}

static struct option signoptions[] = {
    { "all",     0, NULL, 'a' },
    { "zone",    1, NULL, 'z' },
    { "serial",  1, NULL, 's' },
    { "time",    1, NULL, 't' },
    { NULL,      0, NULL, 0 }
};

int
getlong(char* s, char** endptr, long *result)
{
    char *end;
    while(isspace(*s))
        ++s;
    errno = 0;
    *result = strtol(s, &end, 0);
    if(errno == ERANGE) {
        *endptr = NULL;
        return -1;
    }
    if(end) {
        if(s == end) {
            if(endptr)
                *endptr = end;
            return -1;
        }
        while(isspace(*end))
            ++end;
        if(endptr) {
            *endptr = end;
            return 0;
        } else if(*end)
            return -1;
        else
            return 0;
    } else {
        *endptr = end;
        return -1;
    }
}

static int
cmdhandler_handle_cmd_sign(cmdhandler_ctx_type* context, int argc, char* argv[])
{
    engine_type* engine;
    struct longgetopt optctx;
    int allzones = 0;
    int longindex;
    char* zonename = NULL;
    zone_type *zone;
    int force_serial = 0;
    long serial = 0;
    char* signtime = NULL;
    int opt;

    engine = getglobalcontext(context);
    /* Skip the "sign" command itself, then parse options */
    ++argv;
    --argc;
    for(opt = longgetopt(argc, argv, "az:s:t:", signoptions, &longindex, &optctx); opt != -1;
        opt = longgetopt(argc, argv, NULL,      signoptions, &longindex, &optctx)) {
        switch(opt) {
            case 'a':
                allzones = 1;
                break;
            case 'z':
                zonename = optctx.optarg;
                break;
            case 's':
                getlong(optctx.optarg, NULL, &serial);
                force_serial = 1;
                break;
            case 't':
                signtime = optctx.optarg;
                break;
            default:
                client_printf_err(context->sockfd, "unknown arguments\n");
                return -1;
        }
    }
    if(optctx.optind < argc)
        zonename = argv[optctx.optind];
    if(!allzones && (zonename == NULL || *zonename == '\0')) {
        client_printf_err(context->sockfd, "No zone name provided to zone sign command.\n");
        return -1;
    }
    if(allzones) {
        pthread_mutex_lock(&engine->zonelist->zl_lock);
        ldns_rbnode_t* node;
        for (node = ldns_rbtree_first(engine->zonelist->zones); node != LDNS_RBTREE_NULL && node != NULL; node = ldns_rbtree_next(node)) {
            zone = (zone_type*)node->data;
            forceread(engine, zone, 0, 0, context->sockfd);
        }
        pthread_mutex_unlock(&engine->zonelist->zl_lock);
        engine_wakeup_workers(engine);
        client_printf(context->sockfd, "All zones scheduled for immediate re-sign.\n");
    } else {
        pthread_mutex_lock(&engine->zonelist->zl_lock);
        zone = zonelist_lookup_zone_by_name(engine->zonelist, zonename, LDNS_RR_CLASS_IN);
        /* If this zone is just added, don't update (it might not have a task
         * yet).
         */
        if (zone && zone->zl_status == ZONE_ZL_ADDED) {
            zone = NULL;
        }
        pthread_mutex_unlock(&engine->zonelist->zl_lock);

        if (!zone) {
            client_printf(context->sockfd, "Error: Zone %s not found.\n", zonename);
            return 1;
        }

        forceread(engine, zone, force_serial, serial, context->sockfd);
        engine_wakeup_workers(engine);
        client_printf(context->sockfd, "Zone %s scheduled for immediate re-sign.\n", zonename);
        ods_log_verbose("zone %s scheduled for immediate re-sign", zonename);
    }
    return 0;
}

/**
 * Unlink backup file.
 *
 */
static void
unlink_backup_file(const char* filename, const char* extension)
{
    char* tmpname = ods_build_path(filename, extension, 0, 1);
    if (tmpname) {
        ods_log_debug("[%s] unlink file %s", cmdh_str, tmpname);
        unlink(tmpname);
        free((void*)tmpname);
    }
}

/**
 * Handle the 'clear' command.
 *
 */
static int
cmdhandler_handle_cmd_clear(cmdhandler_ctx_type* context, char *cmd)
{
    int sockfd = context->sockfd;
    engine_type* engine;
    char buf[ODS_SE_MAXLINE];
    zone_type* zone = NULL;
    uint32_t inbserial = 0;
    uint32_t intserial = 0;
    uint32_t outserial = 0;
    engine = getglobalcontext(context);
    unlink_backup_file(cmdargument(cmd, NULL, ""), ".inbound");
    unlink_backup_file(cmdargument(cmd, NULL, ""), ".backup");
    unlink_backup_file(cmdargument(cmd, NULL, ""), ".axfr");
    unlink_backup_file(cmdargument(cmd, NULL, ""), ".ixfr");
    pthread_mutex_lock(&engine->zonelist->zl_lock);
    zone = zonelist_lookup_zone_by_name(engine->zonelist, cmdargument(cmd, NULL, ""),
        LDNS_RR_CLASS_IN);
    pthread_mutex_unlock(&engine->zonelist->zl_lock);
    if (zone) {
        pthread_mutex_lock(&zone->zone_lock);
        inbserial = zone->db->inbserial;
        intserial = zone->db->intserial;
        outserial = zone->db->outserial;
        namedb_cleanup(zone->db);
        ixfr_cleanup(zone->ixfr);
        signconf_cleanup(zone->signconf);

        zone->db = namedb_create((void*)zone);
        zone->ixfr = ixfr_create();
        zone->signconf = signconf_create();

        if (!zone->signconf || !zone->ixfr || !zone->db) {
            ods_fatal_exit("[%s] unable to clear zone %s: failed to recreate"
            "signconf, ixfr of db structure (out of memory?)", cmdh_str, cmdargument(cmd, NULL, ""));
            return 1;
        }
        /* restore serial management */
        zone->db->inbserial = inbserial;
        zone->db->intserial = intserial;
        zone->db->outserial = outserial;
        zone->db->have_serial = 1;

        /* If a zone does not have a task we probably never read a signconf
         * for it. Skip reschedule step */
        schedule_scheduletask(engine->taskq, TASK_FORCESIGNCONF, zone->name, zone, &zone->zone_lock, schedule_IMMEDIATELY);
        pthread_mutex_unlock(&zone->zone_lock);

        (void)snprintf(buf, ODS_SE_MAXLINE, "Internal zone information about "
            "%s cleared", cmdargument(cmd, NULL, ""));
        ods_log_info("[%s] internal zone information about %s cleared",
            cmdh_str, cmdargument(cmd, NULL, ""));
    } else {
        (void)snprintf(buf, ODS_SE_MAXLINE, "Cannot clear zone %s, zone not "
            "found", cmdargument(cmd, NULL, ""));
        ods_log_warning("[%s] cannot clear zone %s, zone not found",
            cmdh_str, cmdargument(cmd, NULL, ""));
    }
    client_printf(sockfd, "%s", buf);
    return 0;
}


/**
 * Handle the 'queue' command.
 *
 */
static int
cmdhandler_handle_cmd_queue(cmdhandler_ctx_type* context, char *cmd)
{
    int sockfd = context->sockfd;
    engine_type* engine;
    char* strtime = NULL;
    char buf[ODS_SE_MAXLINE];
    char* taskdesc;
    size_t i = 0;
    time_t now = 0;
    ldns_rbnode_t* node = LDNS_RBTREE_NULL;
    task_type* task = NULL;
    engine = getglobalcontext(context);
    if (!engine->taskq || !engine->taskq->tasks) {
        (void)snprintf(buf, ODS_SE_MAXLINE, "There are no tasks scheduled.\n");
        client_printf(sockfd, "%s", buf);
        return 0;
    }
    /* current time */
    now = time_now();
    strtime = ctime(&now);
    (void)snprintf(buf, ODS_SE_MAXLINE, "It is now %s",
        strtime?strtime:"(null)");
    client_printf(sockfd, "%s", buf);
    /* current work */
    pthread_mutex_lock(&engine->taskq->schedule_lock);
    /* how many tasks */
    (void)snprintf(buf, ODS_SE_MAXLINE, "\nThere are %i tasks scheduled.\n",
        (int) engine->taskq->tasks->count);
    client_printf(sockfd, "%s", buf);
    /* list tasks */
    node = ldns_rbtree_first(engine->taskq->tasks);
    while (node && node != LDNS_RBTREE_NULL) {
        task = (task_type*) node->data;
        for (i=0; i < ODS_SE_MAXLINE; i++) {
            buf[i] = 0;
        }
        taskdesc = schedule_describetask(task);
        client_printf(sockfd, "%s", taskdesc);
        free(taskdesc);
        node = ldns_rbtree_next(node);
    }
    pthread_mutex_unlock(&engine->taskq->schedule_lock);
    return 0;
}


/**
 * Handle the 'flush' command.
 *
 */
static int
cmdhandler_handle_cmd_flush(cmdhandler_ctx_type* context, char *cmd)
{
    int sockfd = context->sockfd;
    engine_type* engine;
    char buf[ODS_SE_MAXLINE];
    engine = getglobalcontext(context);
    ods_log_assert(engine->taskq);
    schedule_flush(engine->taskq);
    engine_wakeup_workers(engine);
    (void)snprintf(buf, ODS_SE_MAXLINE, "All tasks scheduled immediately.\n");
    client_printf(sockfd, "%s", buf);
    ods_log_verbose("[%s] all tasks scheduled immediately", cmdh_str);
    return 0;
}


/**
 * Handle the 'reload' command.
 *
 */
static int
cmdhandler_handle_cmd_reload(cmdhandler_ctx_type* context, char *cmd)
{
    int sockfd = context->sockfd;
    engine_type* engine;
    char buf[ODS_SE_MAXLINE];
    engine = getglobalcontext(context);
    ods_log_error("signer instructed to reload due to explicit command");
    engine->need_to_reload = 1;
    pthread_mutex_lock(&engine->signal_lock);
    pthread_cond_signal(&engine->signal_cond);
    pthread_mutex_unlock(&engine->signal_lock);
    (void)snprintf(buf, ODS_SE_MAXLINE, "Reloading engine.\n");
    client_printf(sockfd, "%s", buf);
    return 0;
}


/**
 * Handle the 'stop' command.
 *
 */
static int
cmdhandler_handle_cmd_stop(cmdhandler_ctx_type* context, char *cmd)
{
    int sockfd = context->sockfd;
    engine_type* engine;
    char buf[ODS_SE_MAXLINE];
    engine = getglobalcontext(context);
    engine->need_to_exit = 1;
    pthread_mutex_lock(&engine->signal_lock);
    pthread_cond_signal(&engine->signal_cond);
    pthread_mutex_unlock(&engine->signal_lock);
    (void)snprintf(buf, ODS_SE_MAXLINE, ODS_SE_STOP_RESPONSE);
    client_printf(sockfd, "%s", buf);
    return 0;
}


/**
 * Handle the 'start' command.
 *
 */
static int
cmdhandler_handle_cmd_start(cmdhandler_ctx_type* context, char *cmd)
{
    int sockfd = context->sockfd;
    char buf[ODS_SE_MAXLINE];
    (void)snprintf(buf, ODS_SE_MAXLINE, "Engine already running.\n");
    client_printf(sockfd, "%s", buf);
    return 0;
}


/**
 * Handle the 'running' command.
 *
 */
static int
cmdhandler_handle_cmd_running(cmdhandler_ctx_type* context, char *cmd)
{
    int sockfd = context->sockfd;
    char buf[ODS_SE_MAXLINE];
    (void)snprintf(buf, ODS_SE_MAXLINE, "Engine running.\n");
    client_printf(sockfd, "%s", buf);
    return 0;
}


/**
 * Handle the 'verbosity' command.
 *
 */
static int
cmdhandler_handle_cmd_verbosity(cmdhandler_ctx_type* context, char *cmd)
{
    int sockfd = context->sockfd;
    char buf[ODS_SE_MAXLINE];
    int val;
    val = atoi(cmdargument(cmd, NULL, "1"));
    ods_log_setverbosity(val);
    (void)snprintf(buf, ODS_SE_MAXLINE, "Verbosity level set to %i.\n", val);
    client_printf(sockfd, "%s", buf);
    return 0;
}


/**
 * Handle erroneous command.
 *
 */
static void
cmdhandler_handle_cmd_error(int sockfd, cmdhandler_ctx_type* context, char* str)
{
    char buf[ODS_SE_MAXLINE];
    (void)snprintf(buf, ODS_SE_MAXLINE, "Error: %s.\n", str?str:"(null)");
    client_printf(sockfd, "%s", buf);
}


/**
 * Handle the 'time leap' command.
 *
 */
static int
cmdhandler_handle_cmd_timeleap(cmdhandler_ctx_type* context, char *cmd)
{
    int sockfd = context->sockfd;
    struct tm strtime_struct;
    char strtime[64]; /* at least 26 according to docs plus a long integer */
    time_t now = time_now();
    time_t time_leap = 0;
    time_t next_leap = 0;
    struct tm tm;
    int taskcount;
    engine_type* engine = getglobalcontext(context);

    /* skip "time" and "leap" */
    while(isspace(*cmd)) ++cmd;
    cmd = &cmd[4];
    while(isspace(*cmd)) ++cmd;
    cmd = &cmd[4];
    while(isspace(*cmd)) ++cmd;

    if (strptime(cmd, "%Y-%m-%d-%H:%M:%S", &tm)) {
        tm.tm_isdst = -1;
        time_leap = mktime(&tm);
        client_printf(sockfd, "Using %s parameter value as time to leap to\n", cmd);
    } else {
        client_printf_err(sockfd, "Time leap: Error - could not convert '%s' to a time. Format is YYYY-MM-DD-HH:MM:SS \n", cmd);
        return -1;
    }
    if (!engine->taskq || !engine->taskq->tasks) {
        client_printf(sockfd, "There are no tasks scheduled.\n");
        return 1;
    }
    schedule_info(engine->taskq, &next_leap, NULL, &taskcount);
    now = time_now();
    strftime(strtime, sizeof (strtime), "%c", localtime_r(&now, &strtime_struct));
    client_printf(sockfd, "There are %i tasks scheduled.\nIt is now       %s (%ld seconds since epoch)\n", taskcount, strtime, (long) now);
    set_time_now(time_leap);
    strftime(strtime, sizeof (strtime), "%c", localtime_r(&time_leap, &strtime_struct));
    client_printf(sockfd, "Leaping to time %s (%ld seconds since epoch)\n", (strtime[0] ? strtime : "(null)"), (long) time_leap);
    ods_log_info("Time leap: Leaping to time %s\n", strtime);
    client_printf(sockfd, "Waking up workers\n");
    engine_wakeup_workers(engine);
    return 0;
}

struct cmd_func_block helpCmdDef       = { "help",       NULL, NULL, NULL, &cmdhandler_handle_cmd_help, NULL };
struct cmd_func_block zonesCmdDef      = { "zones",      NULL, NULL, NULL, &cmdhandler_handle_cmd_zones, NULL };
struct cmd_func_block signCmdDef       = { "sign",       NULL, NULL, NULL, NULL, &cmdhandler_handle_cmd_sign };
struct cmd_func_block clearCmdDef      = { "clear",      NULL, NULL, NULL, &cmdhandler_handle_cmd_clear, NULL };
struct cmd_func_block queueCmdDef      = { "queue",      NULL, NULL, NULL, &cmdhandler_handle_cmd_queue, NULL };
struct cmd_func_block flushCmdDef      = { "flush",      NULL, NULL, NULL, &cmdhandler_handle_cmd_flush, NULL };
struct cmd_func_block updateCmdDef     = { "update",     NULL, NULL, NULL, &cmdhandler_handle_cmd_update, NULL };
struct cmd_func_block stopCmdDef       = { "stop",       NULL, NULL, NULL, &cmdhandler_handle_cmd_stop, NULL };
struct cmd_func_block startCmdDef      = { "start",      NULL, NULL, NULL, &cmdhandler_handle_cmd_start, NULL };
struct cmd_func_block reloadCmdDef     = { "reload",     NULL, NULL, NULL, &cmdhandler_handle_cmd_reload, NULL };
struct cmd_func_block retransferCmdDef = { "retransfer", NULL, NULL, NULL, &cmdhandler_handle_cmd_retransfer, NULL };
struct cmd_func_block runningCmdDef    = { "running",    NULL, NULL, NULL, &cmdhandler_handle_cmd_running, NULL };
struct cmd_func_block verbosityCmdDef  = { "verbosity",  NULL, NULL, NULL, &cmdhandler_handle_cmd_verbosity, NULL };
struct cmd_func_block timeleapCmdDef   = { "time leap",  NULL, NULL, NULL, &cmdhandler_handle_cmd_timeleap, NULL };

struct cmd_func_block* signcommands[] = {
    &helpCmdDef,
    &zonesCmdDef,
    &signCmdDef,
    &clearCmdDef,
    &queueCmdDef,
    &flushCmdDef,
    &updateCmdDef,
    &stopCmdDef,
    &startCmdDef,
    &reloadCmdDef,
    &retransferCmdDef,
    &runningCmdDef,
    &verbosityCmdDef,
    &timeleapCmdDef,
    NULL
};
struct cmd_func_block** signercommands = signcommands;

engine_type*
getglobalcontext(cmdhandler_ctx_type* context)
{
    return (engine_type*) context->globalcontext;
}
