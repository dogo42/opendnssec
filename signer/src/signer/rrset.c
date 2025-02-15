/*
 * Copyright (c) 2009 NLNet Labs. All rights reserved.
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

/**
 * RRset.
 *
 */

#include "config.h"
#include "file.h"
#include "hsm.h"
#include "log.h"
#include "util.h"
#include "compat.h"
#include "signer/rrset.h"
#include "signer/zone.h"

static const char* rrset_str = "rrset";

/**
 * Log RR.
 *
 */
void
log_rr(ldns_rr* rr, const char* pre, int level)
{
    char* str = NULL;
    size_t i = 0;

    if (ods_log_get_level() < level) {
        return;
    }
    str = ldns_rr2str(rr);
    if (!str) {
        ods_log_error("[%s] %s: Error converting RR to string", rrset_str,
            pre?pre:"");
        return;
    }
    str[(strlen(str))-1] = '\0';
    /* replace tabs with white space */
    for (i=0; i < strlen(str); i++) {
        if (str[i] == '\t') {
            str[i] = ' ';
        }
    }
    if (level == LOG_EMERG) {
        ods_fatal_exit("[%s] %s: %s", rrset_str, pre?pre:"", str);
    } else if (level == LOG_ALERT) {
        ods_log_alert("[%s] %s: %s", rrset_str, pre?pre:"", str);
    } else if (level == LOG_CRIT) {
        ods_log_crit("[%s] %s: %s", rrset_str, pre?pre:"", str);
    } else if (level == LOG_ERR) {
        ods_log_error("[%s] %s: %s", rrset_str, pre?pre:"", str);
    } else if (level == LOG_WARNING) {
        ods_log_warning("[%s] %s: %s", rrset_str, pre?pre:"", str);
    } else if (level == LOG_NOTICE) {
        ods_log_info("[%s] %s: %s", rrset_str, pre?pre:"", str);
    } else if (level == LOG_INFO) {
        ods_log_verbose("[%s] %s: %s", rrset_str, pre?pre:"", str);
    } else if (level == LOG_DEBUG) {
        ods_log_debug("[%s] %s: %s", rrset_str, pre?pre:"", str);
    } else if (level == LOG_DEEEBUG) {
        ods_log_deeebug("[%s] %s: %s", rrset_str, pre?pre:"", str);
    } else {
        ods_log_deeebug("[%s] %s: %s", rrset_str, pre?pre:"", str);
    }
    free((void*)str);
}


/**
 * Log RRset.
 *
 */
void
log_rrset(ldns_rdf* dname, ldns_rr_type type, const char* pre, int level)
{
    char* str = NULL;
    size_t i = 0;

    if (ods_log_get_level() < level) {
        return;
    }
    str = ldns_rdf2str(dname);
    if (!str) {
        return;
    }
    str[(strlen(str))-1] = '\0';
    /* replace tabs with white space */
    for (i=0; i < strlen(str); i++) {
        if (str[i] == '\t') {
            str[i] = ' ';
        }
    }
    if (level == LOG_EMERG) {
        ods_fatal_exit("[%s] %s: <%s,%s>", rrset_str, pre?pre:"", str,
            rrset_type2str(type));
    } else if (level == LOG_ALERT) {
        ods_log_alert("[%s] %s: <%s,%s>", rrset_str, pre?pre:"", str,
            rrset_type2str(type));
    } else if (level == LOG_CRIT) {
        ods_log_crit("[%s] %s: <%s,%s>", rrset_str, pre?pre:"", str,
            rrset_type2str(type));
    } else if (level == LOG_ERR) {
        ods_log_error("[%s] %s: <%s,%s>", rrset_str, pre?pre:"", str,
            rrset_type2str(type));
    } else if (level == LOG_WARNING) {
        ods_log_warning("[%s] %s: <%s,%s>", rrset_str, pre?pre:"", str,
            rrset_type2str(type));
    } else if (level == LOG_NOTICE) {
        ods_log_info("[%s] %s: <%s,%s>", rrset_str, pre?pre:"", str,
            rrset_type2str(type));
    } else if (level == LOG_INFO) {
        ods_log_verbose("[%s] %s: <%s,%s>", rrset_str, pre?pre:"", str,
            rrset_type2str(type));
    } else if (level == LOG_DEBUG) {
        ods_log_debug("[%s] %s: <%s,%s>", rrset_str, pre?pre:"", str,
            rrset_type2str(type));
    } else if (level == LOG_DEEEBUG) {
        ods_log_deeebug("[%s] %s: <%s,%s>", rrset_str, pre?pre:"", str,
            rrset_type2str(type));
    } else {
        ods_log_deeebug("[%s] %s: <%s,%s>", rrset_str, pre?pre:"", str,
            rrset_type2str(type));
    }
    free((void*)str);
}


/**
 * Get the string-format of RRtype.
 *
 */
const char*
rrset_type2str(ldns_rr_type type)
{
    if (type == LDNS_RR_TYPE_IXFR) {
        return "IXFR";
    } else if (type == LDNS_RR_TYPE_AXFR) {
        return "AXFR";
    } else if (type == LDNS_RR_TYPE_MAILB) {
        return "MAILB";
    } else if (type == LDNS_RR_TYPE_MAILA) {
        return "MAILA";
    } else if (type == LDNS_RR_TYPE_ANY) {
        return "ANY";
    } else {
        const ldns_rr_descriptor* descriptor = ldns_rr_descript(type);
        if (descriptor && descriptor->_name) {
            return descriptor->_name;
        }
    }
    return "TYPE???";
}

static int
memberdestroy(void* dummy, void* member)
{
    rrsig_type* sig = (rrsig_type*) member;
    (void)dummy;
    free((void*) sig->key_locator);
    sig->key_locator = NULL;
    /* The rrs may still be in use by IXFRs so cannot do ldns_rr_free(sig->rr); */
    ldns_rr_free(sig->rr);
    sig->owner = NULL;
    sig->rr = NULL;
    return 0;
}


/**
 * Create RRset.
 *
 */
rrset_type*
rrset_create(zone_type* zone, ldns_rr_type type)
{
    rrset_type* rrset = NULL;
    if (!type || !zone) {
        return NULL;
    }
    CHECKALLOC(rrset = (rrset_type*) malloc(sizeof(rrset_type)));
    if (!rrset) {
        ods_log_error("[%s] unable to create RRset %u: allocator_alloc() "
            "failed", rrset_str, (unsigned) type);
        return NULL;
    }
    rrset->next = NULL;
    rrset->rrs = NULL;
    rrset->domain = NULL;
    rrset->zone = zone;
    rrset->rrtype = type;
    rrset->rr_count = 0;
    collection_create_array(&rrset->rrsigs, sizeof(rrsig_type), rrset->zone->rrstore);
    rrset->needs_signing = 0;
    return rrset;
}

collection_class
rrset_store_initialize()
{
    collection_class klass;
    collection_class_allocated(&klass, NULL, memberdestroy);
    return klass;
}


/**
 * Lookup RR in RRset.
 *
 */
rr_type*
rrset_lookup_rr(rrset_type* rrset, ldns_rr* rr)
{
    ldns_status lstatus = LDNS_STATUS_OK;
    int cmp = 0;
    size_t i = 0;

    if (!rrset || !rr || rrset->rr_count <= 0) {
       return NULL;
    }
    for (i=0; i < rrset->rr_count; i++) {
        lstatus = util_dnssec_rrs_compare(rrset->rrs[i].rr, rr, &cmp);
        if (lstatus != LDNS_STATUS_OK) {
            ods_log_error("[%s] unable to lookup RR: compare failed (%s)",
                rrset_str, ldns_get_errorstr_by_id(lstatus));
            return NULL;
        }
        if (!cmp) { /* equal */
            return &rrset->rrs[i];
        }
    }
    return NULL;
}

/**
 * What TTL should new RR's in this RRS get?
 */
uint32_t
rrset_lookup_ttl(rrset_type* rrset, uint32_t default_ttl)
{
    for (int i = 0; i < rrset->rr_count; i++) {
        if (!rrset->rrs[i].is_added) continue;
        return ldns_rr_ttl(rrset->rrs[i].rr);
    }
    return default_ttl;
}


/**
 * Count the number of RRs in this RRset that have is_added.
 *
 */
size_t
rrset_count_rr_is_added(rrset_type* rrset)
{
    size_t i = 0;
    size_t count = 0;
    if (!rrset) {
        return 0;
    }
    for (i=0; i < rrset->rr_count; i++) {
        if (rrset->rrs[i].is_added) {
            count++;
        }
    }
    return count;
}


/**
 * Add RR to RRset.
 *
 */
rr_type*
rrset_add_rr(rrset_type* rrset, ldns_rr* rr)
{
    rr_type* rrs_old = NULL;

    ods_log_assert(rrset);
    ods_log_assert(rr);
    ods_log_assert(rrset->rrtype == ldns_rr_get_type(rr));

    rrs_old = rrset->rrs;
    CHECKALLOC(rrset->rrs = (rr_type*) malloc((rrset->rr_count + 1) * sizeof(rr_type)));
    if (!rrset->rrs) {
        ods_fatal_exit("[%s] fatal unable to add RR: allocator_alloc() failed",
            rrset_str);
    }
    if (rrs_old) {
        memcpy(rrset->rrs, rrs_old, (rrset->rr_count) * sizeof(rr_type));
    }
    free(rrs_old);
    rrset->rr_count++;
    rrset->rrs[rrset->rr_count - 1].owner = rrset->domain;
    rrset->rrs[rrset->rr_count - 1].rr = rr;
    rrset->rrs[rrset->rr_count - 1].exists = 0;
    rrset->rrs[rrset->rr_count - 1].is_added = 1;
    rrset->rrs[rrset->rr_count - 1].is_removed = 0;
    rrset->needs_signing = 1;
    log_rr(rr, "+RR", LOG_DEEEBUG);
    return &rrset->rrs[rrset->rr_count -1];
}


/**
 * Delete RR from RRset.
 *
 */
void
rrset_del_rr(rrset_type* rrset, uint16_t rrnum)
{
    rr_type* rrs_orig = NULL;

    ods_log_assert(rrset);
    ods_log_assert(rrnum < rrset->rr_count);

    log_rr(rrset->rrs[rrnum].rr, "-RR", LOG_DEEEBUG);
    rrset->rrs[rrnum].owner = NULL; /* who owns owner? */
    ldns_rr_free(rrset->rrs[rrnum].rr);
    while (rrnum < rrset->rr_count-1) {
        rrset->rrs[rrnum] = rrset->rrs[rrnum+1];
        rrnum++;
    }
    memset(&rrset->rrs[rrset->rr_count-1], 0, sizeof(rr_type));
    rrs_orig = rrset->rrs;
    CHECKALLOC(rrset->rrs = (rr_type*) malloc((rrset->rr_count - 1) * sizeof(rr_type)));
    if(!rrset->rrs) {
        ods_fatal_exit("[%s] fatal unable to delete RR: allocator_alloc() failed",
            rrset_str);
    }
    memcpy(rrset->rrs, rrs_orig, (rrset->rr_count -1) * sizeof(rr_type));
    free(rrs_orig);
    rrset->rr_count--;
    rrset->needs_signing = 1;
}

/**
 * Apply differences at RRset.
 *
 */
void
rrset_diff(rrset_type* rrset, unsigned is_ixfr, unsigned more_coming)
{
    zone_type* zone = NULL;
    uint16_t i = 0;
    uint8_t del_sigs = 0;
    if (!rrset) {
        return;
    }
    zone = (zone_type*) rrset->zone;
    /* CAUTION: both iterator and condition (implicit) are changed
     * within the loop. */
    for (i=0; i < rrset->rr_count; i++) {
        if (rrset->rrs[i].is_added) {
            if (!rrset->rrs[i].exists) {
                /* ixfr +RR */
                if (zone->db->is_initialized) {
                    pthread_mutex_lock(&zone->ixfr->ixfr_lock);
                    ixfr_add_rr(zone->ixfr, rrset->rrs[i].rr);
                    pthread_mutex_unlock(&zone->ixfr->ixfr_lock);
                }
                del_sigs = 1;
            }
            rrset->rrs[i].exists = 1;
            if ((rrset->rrtype == LDNS_RR_TYPE_DNSKEY) && more_coming) {
                continue;
            }
            rrset->rrs[i].is_added = 0;
        } else if (!is_ixfr || rrset->rrs[i].is_removed) {
            if (rrset->rrs[i].exists && zone->db->is_initialized) {
                /* ixfr -RR */
                pthread_mutex_lock(&zone->ixfr->ixfr_lock);
                ixfr_del_rr(zone->ixfr, rrset->rrs[i].rr);
                pthread_mutex_unlock(&zone->ixfr->ixfr_lock);
            }
            rrset->rrs[i].exists = 0;
            rrset_del_rr(rrset, i);
            del_sigs = 1;
            i--;
        }
    }
    if (del_sigs) {
        rrset_drop_rrsigs(zone, rrset);
    }
}

/**
 * Remove signatures, deallocate storage and add then to the outgoing IFXR for that zone.
 *
 */
void
rrset_drop_rrsigs(zone_type* zone, rrset_type* rrset)
{
    rrsig_type* rrsig;
    while((rrsig = collection_iterator(rrset->rrsigs))) {
        /* ixfr -RRSIG */
        if (zone->db->is_initialized) {
            pthread_mutex_lock(&zone->ixfr->ixfr_lock);
            ixfr_del_rr(zone->ixfr, rrsig->rr);
            pthread_mutex_unlock(&zone->ixfr->ixfr_lock);
        }
        collection_del_cursor(rrset->rrsigs);
    }
}

/**
 * Add RRSIG to RRset.
 *
 */
void
rrset_add_rrsig(rrset_type* rrset, ldns_rr* rr,
    const char* locator, uint32_t flags)
{
    rrsig_type rrsig;
    ods_log_assert(rrset);
    ods_log_assert(rr);
    ods_log_assert(ldns_rr_get_type(rr) == LDNS_RR_TYPE_RRSIG);
    rrsig.owner = rrset->domain;
    rrsig.rr = rr;
    rrsig.key_locator = locator;
    rrsig.key_flags = flags;
    collection_add(rrset->rrsigs, &rrsig);
}

/**
 * Transmogrify the RRset to a RRlist.
 *
 */
static ldns_rr_list*
rrset2rrlist(rrset_type* rrset)
{
    ldns_rr_list* rr_list = NULL;
    int ret = 0;
    size_t i = 0;
    rr_list = ldns_rr_list_new();
    for (i=0; i < rrset->rr_count; i++) {
        if (!rrset->rrs[i].exists) {
            log_rr(rrset->rrs[i].rr, "RR does not exist", LOG_WARNING);
            continue;
        }
        ret = (int) ldns_rr_list_push_rr(rr_list, rrset->rrs[i].rr);
        if (!ret) {
            ldns_rr_list_free(rr_list);
            return NULL;
        }
        if (rrset->rrtype == LDNS_RR_TYPE_CNAME ||
            rrset->rrtype == LDNS_RR_TYPE_DNAME) {
            /* singleton types */
            return rr_list;
        }
    }
    ldns_rr_list_sort(rr_list);
    return rr_list;
}


/**
 * Calculate the signature validation period.
 *
 */
static void
rrset_sigvalid_period(signconf_type* sc, ldns_rr_type rrtype, time_t signtime,
    time_t* inception, time_t* expiration)
{
    time_t jitter = 0;
    time_t offset = 0;
    time_t validity = 0;
    time_t random_jitter = 0;
    if (!sc || !rrtype || !signtime) {
        return;
    }
    jitter = duration2time(sc->sig_jitter);
    if (jitter) {
        random_jitter = ods_rand(jitter*2);
    }
    offset = duration2time(sc->sig_inception_offset);
    switch (rrtype) {
        case LDNS_RR_TYPE_NSEC:
        case LDNS_RR_TYPE_NSEC3:
            validity = duration2time(sc->sig_validity_denial);
            break;
        case LDNS_RR_TYPE_DNSKEY:
            if (sc->sig_validity_keyset != NULL && duration2time(sc->sig_validity_keyset) > 0) {
                validity = duration2time(sc->sig_validity_keyset);
            } else {
                validity = duration2time(sc->sig_validity_default);
            }
            break;
        default:
            validity = duration2time(sc->sig_validity_default);
    }
    *inception = signtime - offset;
    *expiration = (signtime + validity + random_jitter) - jitter;
}


struct rrsigkeymatching {
    rrsig_type* signature;
    key_type* key;
};

static int
rrsigkeyismatching(rrsig_type* rrsig, key_type* key)
{
    if(rrsig->key_flags == key->flags && !strcmp(rrsig->key_locator,key->locator)) {
        return 1;
    } else {
        return 0;
    }
}

static void 
rrsigkeymatching(signconf_type* signconf, int nrrsigs, rrsig_type** rrsigs, struct rrsigkeymatching** rrsigkeymatchingptr, int* nrrsigkeymatchingptr)
{
    int nmatches = 0;
    struct rrsigkeymatching* matches = malloc(sizeof(struct rrsigkeymatching) * (signconf->keys->count + nrrsigs));
    for(int i=0; i<nrrsigs; i++) {
        matches[nmatches].signature = rrsigs[i];
        matches[nmatches].key = NULL;
        ++nmatches;
    }
    for(int keyidx=0; keyidx<signconf->keys->count; keyidx++) {
        int matchidx;
        for(matchidx=0; matchidx<nmatches; matchidx++) {
            if(matches[matchidx].signature && rrsigkeyismatching(matches[matchidx].signature, &signconf->keys->keys[keyidx])) {
                matches[matchidx].key = &signconf->keys->keys[keyidx];
                break;
            }
        }
        if(matchidx==nmatches) {
            matches[nmatches].signature = NULL;
            matches[nmatches].key = &signconf->keys->keys[keyidx];
            ++nmatches;
        }
    }
    *rrsigkeymatchingptr = matches;
    *nrrsigkeymatchingptr = nmatches;
}


/**
 * Sign RRset.
 *
 */
ods_status
rrset_sign(hsm_ctx_t* ctx, rrset_type* rrset, time_t signtime)
{
    ods_status status;
    zone_type* zone = NULL;
    uint32_t newsigs = 0;
    uint32_t reusedsigs = 0;
    ldns_rr* rrsig = NULL;
    rrsig_type* signature;
    ldns_rr_list* rr_list = NULL;
    ldns_rr_list* rr_list_clone = NULL;
    const char* locator = NULL;
    time_t inception = 0;
    time_t expiration = 0;
    size_t i = 0, j;
    domain_type* domain = NULL;
    ldns_rr_type dstatus = LDNS_RR_TYPE_FIRST;
    ldns_rr_type delegpt = LDNS_RR_TYPE_FIRST;
    uint8_t algorithm = 0;

    ods_log_assert(ctx);
    ods_log_assert(rrset);
    zone = (zone_type*) rrset->zone;
    ods_log_assert(zone);
    ods_log_assert(zone->signconf);
    /* Recycle signatures */
    if (rrset->rrtype == LDNS_RR_TYPE_NSEC ||
        rrset->rrtype == LDNS_RR_TYPE_NSEC3) {
        dstatus = LDNS_RR_TYPE_SOA;
        delegpt = LDNS_RR_TYPE_SOA;
    } else {
        domain = (domain_type*) rrset->domain;
        dstatus = domain_is_occluded(domain);
        delegpt = domain_is_delegpt(domain);
    }

    int nrrsigs;
    for(nrrsigs=0; (signature = collection_iterator(rrset->rrsigs)); nrrsigs++)
        ;
    rrsig_type** rrsigs = malloc(sizeof(rrsig_type*) * nrrsigs);
    for(i=0; (signature = collection_iterator(rrset->rrsigs)); i++) {
        assert(signature);
        rrsigs[i] = signature;
    }
    struct rrsigkeymatching* matchedsignatures = NULL;
    int nmatchedsignatures;
    rrsigkeymatching(zone->signconf, nrrsigs, rrsigs, &matchedsignatures, &nmatchedsignatures);

    rrset->needs_signing = 0;

    ods_log_assert(rrset->rrs);
    ods_log_assert(rrset->rrs[0].rr);

    /* Skip delegation, glue and occluded RRsets */
    if (dstatus != LDNS_RR_TYPE_SOA) {
        log_rrset(ldns_rr_owner(rrset->rrs[0].rr), rrset->rrtype,
            "skip signing occluded RRset", LOG_DEEEBUG);
        free(rrsigs);
        free(matchedsignatures);
        return ODS_STATUS_OK;
    }
    if (delegpt != LDNS_RR_TYPE_SOA && rrset->rrtype != LDNS_RR_TYPE_DS) {
        log_rrset(ldns_rr_owner(rrset->rrs[0].rr), rrset->rrtype,
            "skip signing delegation RRset", LOG_DEEEBUG);
        free(rrsigs);
        free(matchedsignatures);
        return ODS_STATUS_OK;
    }

    log_rrset(ldns_rr_owner(rrset->rrs[0].rr), rrset->rrtype,
        "sign RRset", LOG_DEEEBUG);
    ods_log_assert(dstatus == LDNS_RR_TYPE_SOA ||
        (delegpt == LDNS_RR_TYPE_SOA || rrset->rrtype == LDNS_RR_TYPE_DS));
    /* Transmogrify rrset */
    rr_list = rrset2rrlist(rrset);
    if (ldns_rr_list_rr_count(rr_list) <= 0) {
        /* Empty RRset, no signatures needed */
        ldns_rr_list_free(rr_list);
        free(rrsigs);
        free(matchedsignatures);
        return ODS_STATUS_OK;
    }
    /* Use rr_list_clone for signing, keep the original rr_list untouched for case preservation */
    rr_list_clone = ldns_rr_list_clone(rr_list);

    /* Further in the code the ORIG_TTL field for the signature will be set
     * to the TTL of the first RR in the list. We must make sure all RR's
     * have the same TTL when signing. We do not need to publish these TTLs.
     * We find the smallest TTL as other software seems to do this.
     **/
    uint32_t min_ttl = ldns_rr_ttl(ldns_rr_list_rr(rr_list_clone, 0));
    for (i = 1; i < ldns_rr_list_rr_count(rr_list_clone); i++) {
        uint32_t rr_ttl = ldns_rr_ttl(ldns_rr_list_rr(rr_list_clone, i));
        if (rr_ttl < min_ttl) min_ttl = rr_ttl;
    }
    for (i = 0; i < ldns_rr_list_rr_count(rr_list_clone); i++) {
        ldns_rr_set_ttl(ldns_rr_list_rr(rr_list_clone, i), min_ttl);
    }

    assert(zone->signconf);
    /* Calculate signature validity */
    rrset_sigvalid_period(zone->signconf, rrset->rrtype, signtime,
         &inception, &expiration);
    uint32_t refresh = 0;
    if (zone->signconf && zone->signconf->sig_refresh_interval) {
        refresh = (uint32_t) (signtime + duration2time(zone->signconf->sig_refresh_interval));
    }

    /* for each signature-key pair, determine whether the signature is valid and/or the key
     * should produce a signature.
     */
    for (int i = 0; i < nmatchedsignatures; i++) {
        if (matchedsignatures[i].signature) {
            assert(matchedsignatures[i].signature->rr);
            expiration = ldns_rdf2native_int32(ldns_rr_rrsig_expiration(matchedsignatures[i].signature->rr));
            inception = ldns_rdf2native_int32(ldns_rr_rrsig_inception(matchedsignatures[i].signature->rr));
        }
        if (matchedsignatures[i].key && matchedsignatures[i].key->ksk && !matchedsignatures[i].key->zsk && rrset->rrtype != LDNS_RR_TYPE_DNSKEY) {
            /* If KSK (not CSK) don't sign non-DNSKEY RRsets */
            matchedsignatures[i].key = NULL;
            matchedsignatures[i].signature = NULL;
        } else if (matchedsignatures[i].key && !matchedsignatures[i].key->ksk && !matchedsignatures[i].key->zsk && rrset->rrtype != LDNS_RR_TYPE_DNSKEY && !matchedsignatures[i].signature) {
            /* Drop keys that aren't ZSK or KSK and still have signatures */
            matchedsignatures[i].key = NULL;
        } else if (matchedsignatures[i].key && !matchedsignatures[i].key->ksk && !matchedsignatures[i].key->zsk && rrset->rrtype != LDNS_RR_TYPE_DNSKEY && !matchedsignatures[i].key->publish) {
            matchedsignatures[i].key = NULL;
            matchedsignatures[i].signature = NULL;
        } else if (matchedsignatures[i].key && !matchedsignatures[i].key->ksk && !matchedsignatures[i].key->zsk && rrset->rrtype == LDNS_RR_TYPE_DNSKEY) {
            matchedsignatures[i].key = NULL;
            matchedsignatures[i].signature = NULL;
        } else if (matchedsignatures[i].key && !matchedsignatures[i].key->ksk && matchedsignatures[i].key->zsk && rrset->rrtype == LDNS_RR_TYPE_DNSKEY) {
            /* If ZSK (not CSK) don't sign DNSKEY RRset */
            matchedsignatures[i].key = NULL;
            matchedsignatures[i].signature = NULL;
        } else if (matchedsignatures[i].key && matchedsignatures[i].key->ksk && matchedsignatures[i].key->locator == NULL) {
            /* If key has no locator, and should be pre-signed dnskey RR, skip */
            matchedsignatures[i].key = NULL;
        } else if (refresh <= (uint32_t) signtime) {
            /* If Refresh is disabled, drop all signatures */
            matchedsignatures[i].signature = NULL;
        } else if (matchedsignatures[i].signature && expiration < refresh && matchedsignatures[i].key && !matchedsignatures[i].key->ksk && !matchedsignatures[i].key->zsk) {
            /* Signature has expired but key not used for signing anymore */
            matchedsignatures[i].signature = NULL;
            matchedsignatures[i].key = NULL;
        } else if (matchedsignatures[i].signature && expiration < refresh) {
            /* Expiration - Refresh has passed */
            matchedsignatures[i].signature = NULL;
        } else if (matchedsignatures[i].signature && inception > (uint32_t) signtime) {
            /* Inception has not yet passed */
            matchedsignatures[i].signature = NULL;
        } else if (matchedsignatures[i].signature && !matchedsignatures[i].key) {
            /* Orphaned signatures */
            matchedsignatures[i].signature = NULL;
        } else if (dstatus != LDNS_RR_TYPE_SOA || (delegpt != LDNS_RR_TYPE_SOA && rrset->rrtype != LDNS_RR_TYPE_DS)) {
            /* Skip delegation, glue and occluded RRsets */
            matchedsignatures[i].key = NULL;
            matchedsignatures[i].signature = NULL;
        } else {
            ods_log_assert(dstatus == LDNS_RR_TYPE_SOA || (delegpt == LDNS_RR_TYPE_SOA || rrset->rrtype == LDNS_RR_TYPE_DS));
        }
    }
    /* At this time, each signature, key pair is valid, if there is a signature and a key, it is valid, if there is 
     * no key, there should be no signature, if there is no key, there should be no signature.  However for DNS
     * optimization, there needs to be no signature, if there is a signature for another key with the same algorithm
     * that is still valid.
     */
    for (int i = 0; i < nmatchedsignatures; i++) {
        if (!matchedsignatures[i].signature && matchedsignatures[i].key) {
            /* We now know this key doesn't sign the set, we will only
             * sign when there isn't already an active key for that algorithm
             */
            int j;
            for (j = 0; j < nmatchedsignatures; j++) {
                if (j != i) {
                    if (matchedsignatures[j].key && matchedsignatures[j].key->publish && matchedsignatures[j].signature && matchedsignatures[j].key->algorithm == matchedsignatures[i].key->algorithm) {
                        break;
                    }
                }
            }
            if (j < nmatchedsignatures) {
                matchedsignatures[i].key = NULL;
                matchedsignatures[i].signature = NULL;
            }
        }
    }

    /* For each of the existing signatures, if they are no longer present in the output, delete them
     * The rrsigs array is guaranteed to line up with the mathcedsignatures array
     */
    if (zone->db->is_initialized) {
        pthread_mutex_lock(&zone->ixfr->ixfr_lock);
        for(i=0; i<nrrsigs; i++) {
            if(matchedsignatures[i].signature == NULL) {
                if (rrsigs[i] != NULL) {
                    ixfr_del_rr(zone->ixfr, rrsigs[i]->rr);
                }
            }
        }
        pthread_mutex_unlock(&zone->ixfr->ixfr_lock);
    }
    for(i=0; i<nrrsigs; i++) {
        if(matchedsignatures[i].signature == NULL) {
            if (rrsigs[i] != NULL) {
                while((signature = collection_iterator(rrset->rrsigs))) {
                    if(signature == rrsigs[i]) {
                        collection_del_cursor(rrset->rrsigs);
                    }
                }
            }
        } else
           ++reusedsigs;
    }
    /* only at this time we have no need for the list anymore (just the list) */
    free(rrsigs);

    /* Calculate signature validity for new signatures */
    rrset_sigvalid_period(zone->signconf, rrset->rrtype, signtime, &inception, &expiration);
    /* for each missing signature (no signature, but with key in the tuplie list) produce a signature */
    for (int i = 0; i < nmatchedsignatures; i++) {
        if (!matchedsignatures[i].signature && matchedsignatures[i].key) {
            /* Sign the RRset with this key */
            ods_log_deeebug("[%s] signing RRset[%i] with key %s", rrset_str,
                rrset->rrtype, zone->signconf->keys->keys[i].locator);
            rrsig = lhsm_sign(ctx, rr_list_clone, matchedsignatures[i].key,
                zone->apex, inception, expiration);
            if (!rrsig) {
                ods_log_crit("[%s] unable to sign RRset[%i]: lhsm_sign() failed",
                        rrset_str, rrset->rrtype);
                free(matchedsignatures);
                ldns_rr_list_free(rr_list);
                ldns_rr_list_free(rr_list_clone);
                return ODS_STATUS_HSM_ERR;
            }
            /* Add signature */
            locator = strdup(matchedsignatures[i].key->locator);
            rrset_add_rrsig(rrset, rrsig, locator, matchedsignatures[i].key->flags);
            newsigs++;
            /* ixfr +RRSIG */
            if (zone->db->is_initialized) {
                pthread_mutex_lock(&zone->ixfr->ixfr_lock);
                ixfr_add_rr(zone->ixfr, rrsig);
                pthread_mutex_unlock(&zone->ixfr->ixfr_lock);
            }
        }
    }
    /* Add signatures for DNSKEY if have been configured to be added explicitjy */
    if(rrset->rrtype == LDNS_RR_TYPE_DNSKEY && zone->signconf->dnskey_signature) {
        for(i=0; zone->signconf->dnskey_signature[i]; i++) {
            rrsig = NULL;
            if ((status = rrset_getliteralrr(&rrsig, zone->signconf->dnskey_signature[i], duration2time(zone->signconf->dnskey_ttl), zone->apex)) != ODS_STATUS_OK) {
                    ods_log_error("[%s] unable to publish dnskeys for zone %s: "
                            "error decoding literal dnskey", rrset_str, zone->name);
                    ldns_rr_list_deep_free(rr_list_clone);
                    return status;
            }
            /* Add signature */
            rrset_add_rrsig(rrset, rrsig, NULL, 0);
            newsigs++;
            /* ixfr +RRSIG */
            if (zone->db->is_initialized) {
                pthread_mutex_lock(&zone->ixfr->ixfr_lock);
                ixfr_add_rr(zone->ixfr, rrsig);
                pthread_mutex_unlock(&zone->ixfr->ixfr_lock);
            }
        }
    }

    /* RRset signing completed */
    free(matchedsignatures);
    ldns_rr_list_free(rr_list);
    ldns_rr_list_deep_free(rr_list_clone);
    pthread_mutex_lock(&zone->stats->stats_lock);
    if (rrset->rrtype == LDNS_RR_TYPE_SOA) {
        zone->stats->sig_soa_count += newsigs;
    }
    zone->stats->sig_count += newsigs;
    zone->stats->sig_reuse += reusedsigs;
    pthread_mutex_unlock(&zone->stats->stats_lock);
    return ODS_STATUS_OK;
}

ods_status
rrset_getliteralrr(ldns_rr** dnskey, const char *resourcerecord, uint32_t ttl, ldns_rdf* apex)
{
    uint8_t dnskeystring[4096];
    ldns_status ldnsstatus;
    int len;
    if ((len = b64_pton(resourcerecord, dnskeystring, sizeof (dnskeystring) - 2)) < 0) {
        return ODS_STATUS_PARSE_ERR;
    }
    dnskeystring[len] = '\0';
    if ((ldnsstatus = ldns_rr_new_frm_str(dnskey, (const char*) dnskeystring, ttl, apex, NULL)) != LDNS_STATUS_OK) {
        return ODS_STATUS_PARSE_ERR;
    }
    return ODS_STATUS_OK;
}

/**
 * Print RRset.
 *
 */
void
rrset_print(FILE* fd, rrset_type* rrset, int skip_rrsigs,
    ods_status* status)
{
    rrsig_type* rrsig;
    uint16_t i = 0;
    ods_status result = ODS_STATUS_OK;

    if (!rrset || !fd) {
        ods_log_crit("[%s] unable to print RRset: rrset or fd missing",
            rrset_str);
        if (status) {
            *status = ODS_STATUS_ASSERT_ERR;
        }
    } else {
        for (i=0; i < rrset->rr_count; i++) {
            if (rrset->rrs[i].exists) {
                result = util_rr_print(fd, rrset->rrs[i].rr);
                if (rrset->rrtype == LDNS_RR_TYPE_CNAME ||
                    rrset->rrtype == LDNS_RR_TYPE_DNAME) {
                    /* singleton types */
                    break;
                }
                if (result != ODS_STATUS_OK) {
                    zone_type* zone = (zone_type*) rrset->zone;
                    log_rrset(ldns_rr_owner(rrset->rrs[i].rr), rrset->rrtype,
                        "error printing RRset", LOG_CRIT);
                    zone->adoutbound->error = 1;
                    break;
                }
            }
        }
        if (! skip_rrsigs) {
            result = ODS_STATUS_OK;
            while((rrsig = collection_iterator(rrset->rrsigs))) {
                if (result == ODS_STATUS_OK) {
                    result = util_rr_print(fd, rrsig->rr);
                    if (result != ODS_STATUS_OK) {
                        zone_type* zone = rrset->zone;
                        log_rrset(ldns_rr_owner(rrset->rrs[i].rr), rrset->rrtype,
                            "error printing RRset", LOG_CRIT);
                        zone->adoutbound->error = 1;
                    }
                }
            }
        }
        if (status) {
            *status = result;
        }
    }
}


/**
 * Clean up RRset.
 *
 */
void
rrset_cleanup(rrset_type* rrset)
{
    uint16_t i = 0;
    if (!rrset) {
       return;
    }
    rrset_cleanup(rrset->next);
    rrset->next = NULL;
    rrset->domain = NULL;
    for (i=0; i < rrset->rr_count; i++) {
        ldns_rr_free(rrset->rrs[i].rr);
        rrset->rrs[i].owner = NULL;
    }
    collection_destroy(&rrset->rrsigs);
    free(rrset->rrs);
    free(rrset);
}

/**
 * Backup RRset.
 *
 */
void
rrset_backup2(FILE* fd, rrset_type* rrset)
{
    rrsig_type* rrsig;
    char* str = NULL;
    if (!rrset || !fd) {
        return;
    }
    while((rrsig = collection_iterator(rrset->rrsigs))) {
        if ((str = ldns_rr2str(rrsig->rr))) {
            fprintf(fd, "%.*s; {locator %s flags %u}\n", (int)strlen(str)-1, str,
                    rrsig->key_locator, rrsig->key_flags);
            free(str);
        }
    }
}
