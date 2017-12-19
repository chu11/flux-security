#ifndef _UTIL_KV_H
#define _UTIL_KV_H

/* Simple serialization:
 *   key=value\0key=value\0...key=value\0
 */

#include <stdarg.h>
#include <stdbool.h>

/* Create/destroy/copy kv object.
 */
struct kv *kv_create (void);
void kv_destroy (struct kv *kv);
struct kv *kv_copy (const struct kv *kv);

/* Return true if kv1 is identical to kv2 (including entry order)
 */
bool kv_equal (const struct kv *kv1, const struct kv *kv2);

/* Remove 'key' from kv object.
 *   EINVAL - invalid argument
 *   ENOENT - key not found
 */
int kv_delete (struct kv *kv, const char *key);

/* Add key=val to kv object.
 * Return 0 on success, -1 on failure with errno set:
 *   EINVAL - invalid argument
 *   ENOMEM - out of memory
 */
int kv_put (struct kv *kv, const char *key, const char *val);

/* Find key in kv object and set val (if 'val' is non-NULL).
 * Return 0 on success, -1 on failure wtih errno set:
 *   EINVAL - invalid argument
 *   ENOENT - key not found
 */
int kv_get (const struct kv *kv, const char *key, const char **val);

/* Access internal binary encoding.
 * Return 0 on success, -1 on failure with errno set.
 */
int kv_raw_encode (const struct kv *kv, const char **buf, int *len);

/* Create kv object from binary encoding.
 * Return kv object on success, NULL on failure with errno set.
 */
struct kv *kv_raw_decode (const char *buf, int len);

/* Iteration example:
 *
 *   const char *key = NULL;
 *
 *   while ((key = kv_next (kv, key))) {
 *       const char *val = kv_val (key);
 *       ...
 *   }
 *
 * kv_delete() may not be called on kv during iteration.
 */
const char *kv_next (const struct kv *kv, const char *key);
const char *kv_val (const char *key);

#endif /* !_UTIL_KV_H */

/*
 * vi:tabstop=4 shiftwidth=4 expandtab
 */
