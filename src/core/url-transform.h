#ifndef _URL_TRANSFORM_H_
#define _URL_TRANSFORM_H_

/**
 * If -1 is returned, memory allocate fail.
 * If  0 is returned, not transformed.
 * If  1 is returned, transformed.
 */
int transform_url(const char * url, char ** outP);

#endif
