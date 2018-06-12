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

#include <assert.h>
#include <curl/curl.h>
#include <panic/panic.h>
#include <alligator/alligator.h>
#include "mailer.h"

static bool initialized = false;  // TODO: make thread local

const Error Mailer_Error_ConnectionFailed = Error_new("Connection failed");
const Error Mailer_Error_ConnectionTimedOut = Error_new("Connection timed out");
const Error Mailer_Error_ConnectionSSLFailed = Error_new("Connection SSL failed");
const Error Mailer_Error_AuthenticationFailed = Error_new("Authentication failed");
const Error Mailer_Error_UnableToResolveHost = Error_new("Unable to resolve host");
const Error Mailer_Error_UnableToResolveProxy = Error_new("Unable to resolve proxy");
const Error Mailer_Error_UnableToSendData = Error_new("Unable to send data");

void Mailer_initialize(void) {
    assert(!initialized);
    const CURLcode e = curl_global_init(CURL_GLOBAL_ALL);
    if (CURLE_OK != e) {
        Panic_terminate("Unable to initialize CURL\n%s\n", curl_easy_strerror(e));
    }
    initialized = true;
}

void Mailer_terminate(void) {
    assert(initialized);
    curl_global_cleanup();
}

struct Mailer_Builder {
    struct Mailer_Client *client;
};

struct Mailer_Client {
    const char *serverUrl;
    const char *senderEmail;
    const char *username;
    const char *password;
    bool sslEnabled;
};

struct Mailer_Builder *Mailer_Builder_new(const char *serverUrl, const char *senderEmail) {
    assert(serverUrl);
    assert(senderEmail);
    struct Mailer_Builder *builder = Option_unwrap(Alligator_malloc(sizeof(*builder)));
    struct Mailer_Client *client = builder->client = Option_unwrap(Alligator_calloc(1, sizeof(*client)));
    client->serverUrl = serverUrl;
    client->senderEmail = senderEmail;
    return builder;
}

struct Mailer_Builder *Mailer_Builder_setSSL(struct Mailer_Builder *self, bool enabled) {
    assert(self);
    self->client->sslEnabled = enabled;
    return self;
}

struct Mailer_Builder *Mailer_Builder_setAuth(struct Mailer_Builder *self, const char *username, const char *password) {
    assert(self);
    assert(username);
    assert(password);
    self->client->username = username;
    self->client->password = password;
    return self;
}

struct Mailer_Client *Mailer_Builder_build(struct Mailer_Builder **ref) {
    assert(ref);
    assert(*ref);
    struct Mailer_Builder *builder = *ref;
    struct Mailer_Client *client = builder->client;
    Alligator_free(builder);
    *ref = NULL;
    return client;
}

void Mailer_Builder_delete(struct Mailer_Builder *self) {
    if (self) {
        Alligator_free(self->client);
        Alligator_free(self);
    }
}

Error Mailer_Client_send(struct Mailer_Client *self, const char *receiverEmail, const char *subject, const char *body) {
    assert(self);
    assert(receiverEmail);
    assert(body);

    Error error = Ok;
    FILE *file = NULL;
    CURL *curl = NULL;
    struct curl_slist *recipients = NULL;

    file = tmpfile();
    if (NULL == file) {
        Panic_terminate("Unable to open temporary file\n");
    }

    fprintf(file, "From: %s\r\nTo: %s\r\nSubject: %s\r\n\r\n%s\r\n", self->senderEmail, receiverEmail, subject, body);
    fflush(file);
    rewind(file);

    recipients = curl_slist_append(recipients, receiverEmail);
    if (NULL == recipients) {
        Panic_terminate("%s\n", Error_explain(OutOfMemory));
    }

    curl = curl_easy_init();
    if (NULL == curl) {
        Panic_terminate("%s\n", Error_explain(OutOfMemory));
    }

#ifdef MAILER_DEBUG
        curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
#endif

    curl_easy_setopt(curl, CURLOPT_URL, self->serverUrl);

    if (self->sslEnabled) {
        curl_easy_setopt(curl, CURLOPT_USE_SSL, CURLUSESSL_ALL);
    }

    if (NULL != self->username) {
        assert(NULL != self->password);
        curl_easy_setopt(curl, CURLOPT_USERNAME, self->username);
        curl_easy_setopt(curl, CURLOPT_PASSWORD, self->password);
    }

    curl_easy_setopt(curl, CURLOPT_MAIL_FROM, self->senderEmail);
    curl_easy_setopt(curl, CURLOPT_MAIL_RCPT, recipients);

    curl_easy_setopt(curl, CURLOPT_READDATA, file);
    curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);

    const CURLcode e = curl_easy_perform(curl);
    switch (e) {
        case CURLE_OK:
            error = Ok;
            break;
        case CURLE_COULDNT_CONNECT:
            error = Mailer_Error_ConnectionFailed;
            break;
        case CURLE_OPERATION_TIMEDOUT:
            error = Mailer_Error_ConnectionTimedOut;
            break;
        case CURLE_SSL_CONNECT_ERROR:
            error = Mailer_Error_ConnectionSSLFailed;
            break;
        case CURLE_LOGIN_DENIED:
            error = Mailer_Error_AuthenticationFailed;
            break;
        case CURLE_COULDNT_RESOLVE_HOST:
            error = Mailer_Error_UnableToResolveHost;
            break;
        case CURLE_COULDNT_RESOLVE_PROXY:
            error = Mailer_Error_UnableToResolveProxy;
            break;
        case CURLE_SEND_ERROR:
            error = Mailer_Error_UnableToSendData;
            break;
        default:
            Panic_terminate("curl_easy_perform(): error (%d) %s\n", e, curl_easy_strerror(e));  // TODO enumerate errors
    }

    curl_slist_free_all(recipients);
    curl_easy_cleanup(curl);
    fclose(file);
    return error;
}

void Mailer_Client_delete(struct Mailer_Client *self) {
    if (self) {
        Alligator_free(self);
    }
}
