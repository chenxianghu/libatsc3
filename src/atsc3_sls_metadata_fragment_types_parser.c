/*
 * atsc3_sls_metadata_fragment_types_parser.c
 *
 *  Created on: Apr 2, 2019
 *      Author: jjustman
 */

#include "atsc3_mime_multipart_related.h"
#include "atsc3_sls_metadata_fragment_types_parser.h"

#define	ATSC3_ROUTE_USD_TYPE	 		"application/route-usd+xml"
#define ATSC3_ROUTE_S_TSID_TYPE			"application/route-s-tsid+xml"
#define ATSC3_ROUTE_MPD_TYPE			"application/dash+xml"
#define ATSC3_SLS_HELD_FRAGMENT_TYPE	"application/atsc-held+xml"


atsc3_sls_metadata_fragments_t* atsc3_sls_metadata_fragment_types_parse_from_mime_multipart_related_instance(atsc3_mime_multipart_related_instance_t* atsc3_mime_multipart_related_instance) {
	if(!atsc3_mime_multipart_related_instance->atsc3_mime_multipart_related_payload_v.count) {
		return NULL;

	}
	atsc3_sls_metadata_fragments_t* atsc3_sls_metadata_fragments = calloc(1, sizeof(atsc3_sls_metadata_fragments_t));
	atsc3_mime_multipart_related_payload_t* atsc3_mime_multipart_related_payload = NULL;

	for(int i=0; i < atsc3_mime_multipart_related_instance->atsc3_mime_multipart_related_payload_v.count; i++) {
		atsc3_mime_multipart_related_payload = atsc3_mime_multipart_related_instance->atsc3_mime_multipart_related_payload_v.data[i];

		__MIME_PARSER_DEBUG("type     : %s", atsc3_mime_multipart_related_payload->content_type);
		__MIME_PARSER_DEBUG("location : %s", atsc3_mime_multipart_related_payload->content_location);
		__MIME_PARSER_DEBUG("payload  :\n%s", atsc3_mime_multipart_related_payload->payload);

		//USBD fragment creation
		if(!strncmp(ATSC3_ROUTE_USD_TYPE, atsc3_mime_multipart_related_payload->content_type, strlen(ATSC3_ROUTE_USD_TYPE))) {
			atsc3_sls_metadata_fragments->atsc3_route_user_service_bundle_description = atsc3_route_user_service_bundle_description_parse_from_payload(atsc3_mime_multipart_related_payload->payload, atsc3_mime_multipart_related_payload->content_location);
			atsc3_route_usb_dump(atsc3_sls_metadata_fragments->atsc3_route_user_service_bundle_description);
		}


		//S-TSID fragment creation
		if(!strncmp(ATSC3_ROUTE_S_TSID_TYPE, atsc3_mime_multipart_related_payload->content_type, strlen(ATSC3_ROUTE_S_TSID_TYPE))) {
			atsc3_sls_metadata_fragments->atsc3_route_s_tsid = atsc3_route_s_tsid_parse_from_payload(atsc3_mime_multipart_related_payload->payload, atsc3_mime_multipart_related_payload->content_location);
			atsc3_route_s_tsid_dump(atsc3_sls_metadata_fragments->atsc3_route_s_tsid);
		}

		//ROUTE MPD fragment creation
		if(!strncmp(ATSC3_ROUTE_MPD_TYPE, atsc3_mime_multipart_related_payload->content_type, strlen(ATSC3_ROUTE_MPD_TYPE))) {
			atsc3_sls_metadata_fragments->atsc3_route_mpd = atsc3_route_mpd_parse_from_payload(atsc3_mime_multipart_related_payload->payload, atsc3_mime_multipart_related_payload->content_location);
			atsc3_route_mpd_dump(atsc3_sls_metadata_fragments->atsc3_route_mpd);
		}


		//HELD fragment creation
		if(!strncmp(ATSC3_SLS_HELD_FRAGMENT_TYPE, atsc3_mime_multipart_related_payload->content_type, strlen(ATSC3_SLS_HELD_FRAGMENT_TYPE))) {
			atsc3_sls_metadata_fragments->atsc3_sls_held_fragment = atsc3_sls_held_fragment_parse_from_payload(atsc3_mime_multipart_related_payload->payload, atsc3_mime_multipart_related_payload->content_location);
			//dump to make sure we got everything
			atsc3_sls_held_fragment_dump(atsc3_sls_metadata_fragments->atsc3_sls_held_fragment);
		}

	}
	return atsc3_sls_metadata_fragments;
}

