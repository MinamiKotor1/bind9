/*
 * Copyright (C) Internet Systems Consortium, Inc. ("ISC")
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, you can obtain one at https://mozilla.org/MPL/2.0/.
 *
 * See the COPYRIGHT file distributed with this work for additional
 * information regarding copyright ownership.
 */

/* RFC 3845 */

#ifndef RDATA_GENERIC_NSEC_47_C
#define RDATA_GENERIC_NSEC_47_C

/*
 * The attributes do not include DNS_RDATATYPEATTR_SINGLETON
 * because we must be able to handle a parent/child NSEC pair.
 */
#define RRTYPE_NSEC_ATTRIBUTES                                      \
	(DNS_RDATATYPEATTR_DNSSEC | DNS_RDATATYPEATTR_ZONECUTAUTH | \
	 DNS_RDATATYPEATTR_ATCNAME)

static inline isc_result_t
fromtext_nsec(ARGS_FROMTEXT) {
	isc_token_t token;
	dns_name_t name;
	isc_buffer_t buffer;

	REQUIRE(type == dns_rdatatype_nsec);

	UNUSED(type);
	UNUSED(rdclass);
	UNUSED(callbacks);

	/*
	 * Next domain.
	 */
	RETERR(isc_lex_getmastertoken(lexer, &token, isc_tokentype_string,
				      false));
	dns_name_init(&name, NULL);
	buffer_fromregion(&buffer, &token.value.as_region);
	if (origin == NULL) {
		origin = dns_rootname;
	}
	RETTOK(dns_name_fromtext(&name, &buffer, origin, options, target));

	return (typemap_fromtext(lexer, target, false));
}

static inline isc_result_t
totext_nsec(ARGS_TOTEXT) {
	isc_region_t sr;
	dns_name_t name;

	REQUIRE(rdata->type == dns_rdatatype_nsec);
	REQUIRE(rdata->length != 0);

	UNUSED(tctx);

	dns_name_init(&name, NULL);
	dns_rdata_toregion(rdata, &sr);
	dns_name_fromregion(&name, &sr);
	isc_region_consume(&sr, name_length(&name));
	RETERR(dns_name_totext(&name, false, target));
	/*
	 * Don't leave a trailing space when there's no typemap present.
	 */
	if (sr.length > 0) {
		RETERR(str_totext(" ", target));
	}
	return (typemap_totext(&sr, NULL, target));
}

static /* inline */ isc_result_t
fromwire_nsec(ARGS_FROMWIRE) {
	isc_region_t sr;
	dns_name_t name;

	REQUIRE(type == dns_rdatatype_nsec);

	UNUSED(type);
	UNUSED(rdclass);

	dns_decompress_setmethods(dctx, DNS_COMPRESS_NONE);

	dns_name_init(&name, NULL);
	RETERR(dns_name_fromwire(&name, source, dctx, options, target));

	isc_buffer_activeregion(source, &sr);
	RETERR(typemap_test(&sr, false));
	RETERR(mem_tobuffer(target, sr.base, sr.length));
	isc_buffer_forward(source, sr.length);
	return (ISC_R_SUCCESS);
}

static inline isc_result_t
towire_nsec(ARGS_TOWIRE) {
	isc_region_t sr;
	dns_name_t name;
	dns_offsets_t offsets;

	REQUIRE(rdata->type == dns_rdatatype_nsec);
	REQUIRE(rdata->length != 0);

	dns_compress_setmethods(cctx, DNS_COMPRESS_NONE);
	dns_name_init(&name, offsets);
	dns_rdata_toregion(rdata, &sr);
	dns_name_fromregion(&name, &sr);
	isc_region_consume(&sr, name_length(&name));
	RETERR(dns_name_towire(&name, cctx, target));

	return (mem_tobuffer(target, sr.base, sr.length));
}

static inline int
compare_nsec(ARGS_COMPARE) {
	isc_region_t r1;
	isc_region_t r2;

	REQUIRE(rdata1->type == rdata2->type);
	REQUIRE(rdata1->rdclass == rdata2->rdclass);
	REQUIRE(rdata1->type == dns_rdatatype_nsec);
	REQUIRE(rdata1->length != 0);
	REQUIRE(rdata2->length != 0);

	dns_rdata_toregion(rdata1, &r1);
	dns_rdata_toregion(rdata2, &r2);
	return (isc_region_compare(&r1, &r2));
}

static inline isc_result_t
fromstruct_nsec(ARGS_FROMSTRUCT) {
	dns_rdata_nsec_t *nsec = source;
	isc_region_t region;

	REQUIRE(type == dns_rdatatype_nsec);
	REQUIRE(nsec != NULL);
	REQUIRE(nsec->common.rdtype == type);
	REQUIRE(nsec->common.rdclass == rdclass);
	REQUIRE(nsec->typebits != NULL || nsec->len == 0);

	UNUSED(type);
	UNUSED(rdclass);

	dns_name_toregion(&nsec->next, &region);
	RETERR(isc_buffer_copyregion(target, &region));

	region.base = nsec->typebits;
	region.length = nsec->len;
	RETERR(typemap_test(&region, false));
	return (mem_tobuffer(target, nsec->typebits, nsec->len));
}

static inline isc_result_t
tostruct_nsec(ARGS_TOSTRUCT) {
	isc_region_t region;
	dns_rdata_nsec_t *nsec = target;
	dns_name_t name;

	REQUIRE(rdata->type == dns_rdatatype_nsec);
	REQUIRE(nsec != NULL);
	REQUIRE(rdata->length != 0);

	nsec->common.rdclass = rdata->rdclass;
	nsec->common.rdtype = rdata->type;
	ISC_LINK_INIT(&nsec->common, link);

	dns_name_init(&name, NULL);
	dns_rdata_toregion(rdata, &region);
	dns_name_fromregion(&name, &region);
	isc_region_consume(&region, name_length(&name));
	dns_name_init(&nsec->next, NULL);
	name_duporclone(&name, mctx, &nsec->next);

	nsec->len = region.length;
	nsec->typebits = mem_maybedup(mctx, region.base, region.length);
	if (nsec->typebits == NULL) {
		goto cleanup;
	}

	nsec->mctx = mctx;
	return (ISC_R_SUCCESS);

cleanup:
	if (mctx != NULL) {
		dns_name_free(&nsec->next, mctx);
	}
	return (ISC_R_NOMEMORY);
}

static inline void
freestruct_nsec(ARGS_FREESTRUCT) {
	dns_rdata_nsec_t *nsec = source;

	REQUIRE(nsec != NULL);
	REQUIRE(nsec->common.rdtype == dns_rdatatype_nsec);

	if (nsec->mctx == NULL) {
		return;
	}

	dns_name_free(&nsec->next, nsec->mctx);
	if (nsec->typebits != NULL) {
		isc_mem_free(nsec->mctx, nsec->typebits);
	}
	nsec->mctx = NULL;
}

static inline isc_result_t
additionaldata_nsec(ARGS_ADDLDATA) {
	REQUIRE(rdata->type == dns_rdatatype_nsec);

	UNUSED(rdata);
	UNUSED(owner);
	UNUSED(add);
	UNUSED(arg);

	return (ISC_R_SUCCESS);
}

static inline isc_result_t
digest_nsec(ARGS_DIGEST) {
	isc_region_t r;

	REQUIRE(rdata->type == dns_rdatatype_nsec);

	dns_rdata_toregion(rdata, &r);
	return ((digest)(arg, &r));
}

static inline bool
checkowner_nsec(ARGS_CHECKOWNER) {
	REQUIRE(type == dns_rdatatype_nsec);

	UNUSED(name);
	UNUSED(type);
	UNUSED(rdclass);
	UNUSED(wildcard);

	return (true);
}

static inline bool
checknames_nsec(ARGS_CHECKNAMES) {
	REQUIRE(rdata->type == dns_rdatatype_nsec);

	UNUSED(rdata);
	UNUSED(owner);
	UNUSED(bad);

	return (true);
}

static inline int
casecompare_nsec(ARGS_COMPARE) {
	isc_region_t region1;
	isc_region_t region2;
	dns_name_t name1;
	dns_name_t name2;
	int order;

	REQUIRE(rdata1->type == rdata2->type);
	REQUIRE(rdata1->rdclass == rdata2->rdclass);
	REQUIRE(rdata1->type == dns_rdatatype_nsec);
	REQUIRE(rdata1->length != 0);
	REQUIRE(rdata2->length != 0);

	dns_name_init(&name1, NULL);
	dns_name_init(&name2, NULL);

	dns_rdata_toregion(rdata1, &region1);
	dns_rdata_toregion(rdata2, &region2);

	dns_name_fromregion(&name1, &region1);
	dns_name_fromregion(&name2, &region2);

	order = dns_name_rdatacompare(&name1, &name2);
	if (order != 0) {
		return (order);
	}

	isc_region_consume(&region1, name_length(&name1));
	isc_region_consume(&region2, name_length(&name2));

	return (isc_region_compare(&region1, &region2));
}
#endif /* RDATA_GENERIC_NSEC_47_C */
