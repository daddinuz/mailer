/*
 * Author: daddinuz
 * email:  daddinuz@gmail.com
 *
 * Copyright (c) 2018 Davide Di Carlo
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <error/error.h>

#if !(defined(__GNUC__) || defined(__clang__))
#define __attribute__(...)
#endif

#define MAILER_VERSION_MAJOR        0
#define MAILER_VERSION_MINOR        1
#define MAILER_VERSION_PATCH        0
#define MAILER_VERSION_SUFFIX       ""
#define MAILER_VERSION_IS_RELEASE   0
#define MAILER_VERSION_HEX          0x000100

extern const Error Mailer_Error_ConnectionFailed;
extern const Error Mailer_Error_ConnectionTimedOut;
extern const Error Mailer_Error_ConnectionSSLFailed;
extern const Error Mailer_Error_AuthenticationFailed;
extern const Error Mailer_Error_UnableToResolveHost;
extern const Error Mailer_Error_UnableToResolveProxy;
extern const Error Mailer_Error_UnableToSendData;

void Mailer_initialize(void);
void Mailer_terminate(void);

struct Mailer_Builder;
struct Mailer_Client;

struct Mailer_Builder *Mailer_Builder_new(const char *serverUrl, const char *senderEmail)
__attribute__((__warn_unused_result__, __nonnull__));

struct Mailer_Builder *Mailer_Builder_setSSL(struct Mailer_Builder *self, bool enabled)
__attribute__((__nonnull__));

struct Mailer_Builder *Mailer_Builder_setAuth(struct Mailer_Builder *self, const char *username, const char *password)
__attribute__((__nonnull__));

struct Mailer_Client *Mailer_Builder_build(struct Mailer_Builder **ref)
__attribute__((__warn_unused_result__, __nonnull__));

void Mailer_Builder_delete(struct Mailer_Builder *self);

Error Mailer_Client_send(struct Mailer_Client *self, const char *receiverEmail, const char *subject, const char *body)
__attribute__((__warn_unused_result__, __nonnull__));

void Mailer_Client_delete(struct Mailer_Client *self);

#ifdef __cplusplus
}
#endif
