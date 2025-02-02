#include <assert.h>
#include <string.h>
#include <stdio.h>

#include "stringlib.c"

#define CONST 8000

int main(void) {

    char testcases[][CONST] = {
        "lol",
        "lol?",
        "/lol?",
        "lol?query=something&haha=lol",
        "",
        "/",
        "/../../../lol",
        "/a/../b/../c/../lol",
        "/a/b/c/../../lol",
        "/./././lol?q=a",
        "/..qwe",
        ".././././../../",
        "/home/user/docs",
        "/home/user/docs/",
        "/home/user/./docs",
        "/home/user/docs/../pictures",
        "/home//user///docs",
        "/home/./user/../user2/docs/./../pictures",
        "/home/user/docs?query=123",
        "/home/user/docs/..",
        "/../..",
        "/.",
        "/..",
        "////home/results",
        "////home///results////",
        ".",
        ".."
    };

    char expectedResults[][CONST] = {
        "lol",
        "lol",
        "lol",
        "lol",
        "/",
        "/",
        "lol",
        "lol",
        "a/lol",
        "lol",
        "..qwe",
        "/",
        "home/user/docs",
        "home/user/docs/",
        "home/user/docs",
        "home/user/pictures",
        "home/user/docs",
        "home/user2/pictures",
        "home/user/docs",
        "home/user/",
        "/",
        "/",
        "/",
        "home/results",
        "home/results/",
        "/",
        "/"
    };

    for (int i = 0; i < sizeof(testcases) / 8000; i++) {
        printf("Case %d:\nTestcase: %s\n", i + 1, testcases[i]);
        printf("Expected: %s\n", expectedResults[i]);
        normalizePath(CONST, testcases[i]);
        printf("Received: %s\n", testcases[i]);
        assert(strncmp(testcases[i], expectedResults[i], CONST) == 0);
        printf("Correct\n\n");
    }

    return 0;
}