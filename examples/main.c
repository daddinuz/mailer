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

/**
 * Send emails with your gmail account.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <mailer.h>
#include <panic/panic.h>

static void prompt(const char *message, char *buffer, const size_t size) {
    printf("%s", message);
    if (NULL == fgets(buffer, (int) size, stdin)) {
        memset(buffer, 0, size);
    } else {
        buffer[strlen(buffer) - 1] = 0;
    }
}

static void promptMultiline(const char *message, char *buffer, const size_t size) {
    size_t read = 0;
    printf("%s", message);
    do {
        fflush(NULL);
        if (fgets(buffer + read, (int) (size - read), stdin) == NULL) {
            puts("");
            fflush(NULL);
            buffer[read] = 0;
            return;
        }
        read += strlen(buffer + read);
        printf("(CTRL+D to finish)> ");
    } while (read + 1 < size);
    Panic_terminate("Too many characters to read\n");
}

int main() {
    char username[128] = "";
    char password[64] = "";
    char recipient[128] = "";
    char subject[64] = "";
    char body[2048] = "";
    struct Mailer_Builder *builder = NULL;
    struct Mailer_Client *client = NULL;

    Mailer_initialize();
    atexit(Mailer_terminate);

    prompt("username: ", username, sizeof(username));
    strncpy(password, getpass("password: "), sizeof(password));
    prompt("recipient: ", recipient, sizeof(recipient));
    prompt("subject: ", subject, sizeof(subject));
    promptMultiline("body: ", body, sizeof(body));

    builder = Mailer_Builder_new("smtp://smtp.gmail.com:587", username);
    Mailer_Builder_setAuth(builder, username, password);
    Mailer_Builder_setSSL(builder, true);

    printf("Sending... ");
    fflush(NULL);
    client = Mailer_Builder_build(&builder);
    const Error e = Mailer_Client_send(client, recipient, subject, body);
    if (Ok != e) {
        Panic_terminate("Error: %s\n", Error_explain(e));
    }
    Mailer_Client_delete(client);
    puts("sent!");

    return 0;
}
