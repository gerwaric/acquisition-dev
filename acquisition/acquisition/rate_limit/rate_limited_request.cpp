#include <acquisition/rate_limit/rate_limited_request.h>

// Total number of rate-limited requests that have been created.
unsigned long RateLimitedRequest::request_count = 0;