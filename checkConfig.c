#include "parseComments.c"
#include <unistd.h>

void checkConfig(struct json_object * configFile) {
    int configReady = 1;
    if (access(json_object_get_string(json_object_object_get(configFile, "outDirectory")), F_OK) != 0) {
        printf("FATAL: Out directory \"%s\" not found.\n", json_object_get_string(json_object_object_get(configFile, "outDirectory")));
        configReady = 0;
    }

    if (access(json_object_get_string(json_object_object_get(configFile, "subredditsFile")), F_OK) != 0) {
        printf("FATAL: Subreddits file \"%s\" not found.\n", json_object_get_string(json_object_object_get(configFile, "subredditsFile")));
        configReady = 0;
    }

    if (access(json_object_get_string(json_object_object_get(configFile, "tokenFile")), F_OK) == 0) {
        struct json_object * tokenFile;
        tokenFile = json_object_from_file(json_object_get_string(json_object_object_get(configFile, "tokenFile")));
        
        if (json_object_get_string_len(json_object_object_get(tokenFile, "access_token")) == 0) {
            printf("FATAL: Value \"access_token\" does not exist in token file \"%s\".\n", json_object_get_string(json_object_object_get(configFile, "tokenFile")));
            configReady = 0;
        }

        if (json_object_get_string_len(json_object_object_get(tokenFile, "refresh_token")) == 0) {
            printf("FATAL: Value \"refresh_token\" does not exist in token file \"%s\".\n", json_object_get_string(json_object_object_get(configFile, "tokenFile")));
            configReady = 0;
        }

        if (json_object_get_int(json_object_object_get(tokenFile, "request_time")) == 0) {
            printf("FATAL: value \"request_time\" does not exist in token file \"%s\".\n", json_object_get_string(json_object_object_get(configFile, "tokenFile")));
            configReady = 0;
        }

        json_object_put(tokenFile);

    } else {
        printf("FATAL: Token file \"%s\" not found.\n", json_object_get_string(json_object_object_get(configFile, "tokenFile")));
        configReady = 0;
    }

    if (access(json_object_get_string(json_object_object_get(configFile, "loginFile")), F_OK) == 0) {
        struct json_object * loginFile;
        loginFile = json_object_from_file(json_object_get_string(json_object_object_get(configFile, "loginFile")));

        if (json_object_get_string_len(json_object_object_get(loginFile, "username")) == 0) {
            printf("FATAL: value \"username\" does not exist in login file \"%s\".\n", json_object_get_string(json_object_object_get(configFile, "loginFile")));
            configReady = 0;
        }

        if (json_object_get_string_len(json_object_object_get(loginFile, "password")) == 0) {
            printf("FATAL: value \"password\" does not exist in login file \"%s\".\n", json_object_get_string(json_object_object_get(configFile, "loginFile")));
            configReady = 0;
        }

        if (json_object_get_string_len(json_object_object_get(loginFile, "useragent")) == 0) {
            printf("FATAL: value \"useragent\" does not exist in login file \"%s\".\n", json_object_get_string(json_object_object_get(configFile, "loginFile")));
            configReady = 0;
        }

        json_object_put(loginFile);

    } else {
        printf("FATAL: Login file \"%s\" not found.\n", json_object_get_string(json_object_object_get(configFile, "loginFile")));
        configReady = 0;
    }

    if (configReady) {
        struct json_object  * testDownload;
        testDownload = httpRequest("https://oauth.reddit.com", configFile);
        if (testDownload == NULL) {
            printf("FATAL: All configuration elements are present, but at least one credential is invalid.\n");
            configReady = 0;
        }
        json_object_put(testDownload);
    }

    if (!configReady) {
        printf("Fatal error(s) detected. Aborting.\n");
        json_object_put(configFile);
        exit(0);
    } else {
        printf("No configuration errors detected.\n");
    }
}