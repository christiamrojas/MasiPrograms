#ifndef HTTP_H
#define HTTP_H

#include "config.h"
#include "main_definitions.h"


#define HOST_NAME   "masi.smartiotgroup.com"
#define URL         "/api/v1.0/vme/writeMultiValues"
#define HOST_PORT   443

extern char fingerprint[65];

int http_post_request(char *dataBuffer);


#endif