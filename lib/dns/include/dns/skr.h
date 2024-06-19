/*
 * Copyright (C) Internet Systems Consortium, Inc. ("ISC")
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, you can obtain one at https://mozilla.org/MPL/2.0/.
 *
 * See the COPYRIGHT file distributed with this work for additional
 * information regarding copyright ownership.
 */

#pragma once

/*! \file dns/skr.h
 * \brief
 * A skr is a convenience type representing a Signed Key Response (SKR),
 * determining which DNSKEY, CDS, CDNSKEY and corresponding signatures
 * should be published at a given time. It is based on terminology used in
 * https://www.iana.org/dnssec/archive/files/draft-icann-dnssec-keymgmt-01.txt
 */

#include <isc/stdtime.h>

#include <dns/diff.h>
#include <dns/types.h>

ISC_LANG_BEGINDECLS

#define DNS_SKR_MAGIC	 ISC_MAGIC('S', 'K', 'R', '-')
#define DNS_SKR_VALID(t) ISC_MAGIC_VALID(t, DNS_SKR_MAGIC)

#define DNS_SKRBUNDLE_MAGIC    ISC_MAGIC('S', 'K', 'R', 'B')
#define DNS_SKRBUNDLE_VALID(t) ISC_MAGIC_VALID(t, DNS_SKRBUNDLE_MAGIC)

typedef struct dns_skrbundle dns_skrbundle_t;
typedef ISC_LIST(dns_skrbundle_t) dns_skrbundlelist_t;

/* Stores a Signed Key Response (SKR) */
struct dns_skr {
	unsigned int	    magic;
	isc_mem_t	   *mctx;
	char		   *filename;
	isc_time_t	    loadtime;
	dns_skrbundlelist_t bundles;
	isc_refcount_t	    references;
};

struct dns_skrbundle {
	unsigned int  magic;
	isc_stdtime_t inception;
	dns_diff_t    diff;
	ISC_LINK(dns_skrbundle_t) link;
};

isc_result_t
dns_skrbundle_getsig(dns_skrbundle_t *bundle, dst_key_t *key,
		     dns_rdatatype_t covering_type, dns_rdata_t *sigrdata);
/*%<
 * Retrieve the RRSIG rdata for 'covering_type' generated by 'key' from the
 * given 'bundle'.
 *
 * Requires:
 * \li   'bundle' is a valid bundle
 *
 * Returns:
 * \li   a possible error if we fail to convert the rdata to a struct
 * \li   ISC_R_SUCCESS if the signature is found
 * \li   ISC_R_NOTFOUND otherwise
 */

void
dns_skr_create(isc_mem_t *mctx, const char *filename, dns_name_t *origin,
	       dns_rdataclass_t rdclass, dns_skr_t **skrp);
/*%<
 * Create a SKR.
 *
 * Requires:
 * \li   mctx != NULL
 * \li   *skrp != NULL && *skrp == NULL
 */

isc_result_t
dns_skr_read(isc_mem_t *mctx, const char *filename, dns_name_t *origin,
	     dns_rdataclass_t rdclass, dns_ttl_t dnskeyttl, dns_skr_t **skrp);
/*%<
 * Read a SKR from 'filename'.
 *
 * Requires:
 * \li   mctx != NULL
 * \li   *skrp != NULL && *skrp == NULL
 */

dns_skrbundle_t *
dns_skr_lookup(dns_skr_t *skr, isc_stdtime_t time, uint32_t sigval);
/*%<
 * Look up the currently active bundle. The active bundle is the one which
 * inception time is prior to 'time' and the next bundle inception is after
 " 'time'. In case of the last bundle in the SKR, 'time' is expected to be
 * lower than the last bundle inception time plus 'sigval'.
 *
 * Requires:
 * \li   'skr' is a valid SKR
 *
 * Returns:
 * \li   The currently active bundle, or NULL if no such bundle is found.
 */

void
dns_skr_attach(dns_skr_t *source, dns_skr_t **targetp);
/*%<
 * Attach '*targetp' to 'source'.
 *
 * Requires:
 *
 *\li   'source' is a valid SKR.
 *
 *\li   'targetp' points to a NULL dns_skr_t *.
 *
 * Ensures:
 *
 *\li   *targetp is attached to source.
 */

void
dns_skr_detach(dns_skr_t **skrp);
/*%<
 * Detach SKR.
 *
 * Requires:
 *
 *\li   'skrp' points to a valid dns_skr_t *
 *
 * Ensures:
 *
 *\li   *skrp is NULL.
 */

void
dns_skr_destroy(dns_skr_t *skr);
/*%<
 * Destroy a SKR.
 *
 * Requires:
 * \li   'skr' is a valid SKR
 */

ISC_LANG_ENDDECLS
