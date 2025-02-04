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
        "..",
        "lol/.",
    };

    char expectedResults[][CONST] = {
        "lol",
        "lol",
        "lol",
        "lol",
        "",
        "",
        "lol",
        "lol",
        "a/lol",
        "lol",
        "..qwe",
        "",
        "home/user/docs",
        "home/user/docs/",
        "home/user/docs",
        "home/user/pictures",
        "home/user/docs",
        "home/user2/pictures",
        "home/user/docs",
        "home/user/",
        "",
        "",
        "",
        "home/results",
        "home/results/",
        "",
        "",
        "lol/"
    };

    for (int i = 0; i < sizeof(testcases) / CONST; i++) {
        printf("Case %d:\nTestcase: %s\n", i + 1, testcases[i]);
        printf("Expected: %s\n", expectedResults[i]);
        normalizePath(CONST, testcases[i]);
        printf("Received: %s\n", testcases[i]);
        assert(strncmp(testcases[i], expectedResults[i], CONST) == 0);
        printf("Correct\n\n");
    }

    char* endsWithChar1 = "";
    char* endsWithChar2 = "/";
    char* endsWithChar3 = "aboba/";

    // printf("'%s': %d\n", endsWithChar1, endsWithChar(endsWithChar1, '/'));
    // printf("'%s': %d\n", endsWithChar2, endsWithChar(endsWithChar2, '/'));
    // printf("'%s': %d\n", endsWithChar3, endsWithChar(endsWithChar3, '/'));

    char getExtension_test[][CONST] = {
        "home/tag/", "home/tag", "home/tag.pdf", "/", "", "lol.pdf/",
        "lol.pdf/."
    };

    char* getExtension_result[CONST] = {
        NULL, NULL, ".pdf", NULL, NULL, NULL, "."
    };

    for (int i = 0; i < sizeof(getExtension_test) / CONST; i++) {
        printf("Case %d:\n", i + 1);
        printf("Testcase: %s\n", getExtension_test[i]);
        printf("Expected: %s\n", getExtension_result[i]);
        char* str = getExtension(getExtension_test[i]);
        if (str) printf("Received: %s\n", str);
        else printf("Received: NULL\n");
    }
    printf("\n\n\n");

    char* str = "";
    printf("'%s' isDir: %d\n", str, isDirectory(str));

    str = ".";
    printf("'%s' isDir: %d\n", str, isDirectory(str));

    str = "..";
    printf("'%s' isDir: %d\n", str, isDirectory(str));

    str = "/";
    printf("'%s' isDir: %d\n", str, isDirectory(str));

    str = "/tmp";
    printf("'%s' isDir: %d\n", str, isDirectory(str));

    str = "test";
    printf("'%s' isDir: %d\n", str, isDirectory(str));

    str = "stringlib.c";
    printf("'%s' isDir: %d\n", str, isDirectory(str));

    str = "not_real";
    printf("'%s' isDir: %d\n", str, isDirectory(str));

    return 0;
}