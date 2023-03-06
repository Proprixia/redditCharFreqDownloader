#include <json-c/json.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <curl/curl.h>
#include "macros.c"
#include <unistd.h>

struct memory {
    void * contents;
    int size;
};

size_t downloadToMem(void * incoming, size_t size, size_t nmembs, struct memory * memoryBuffer) {
    CATSTRTOMEMORYSTRUCT(memoryBuffer, (char *) incoming)
    return (size * nmembs);
}

void baseRequest(struct memory * pageDL, char * targetURL, char * postFields, struct curl_slist * httpHeaders, struct json_object * configFile) {
    CURL * request;
    CURLcode res;
    struct json_object * loginInfo;

    loginInfo = json_object_from_file(json_object_get_string(json_object_object_get(configFile, "loginFile")));

    curl_global_init(CURL_GLOBAL_ALL);
    request = curl_easy_init();

    curl_easy_setopt(request, CURLOPT_USERNAME, json_object_get_string(json_object_object_get(loginInfo, "username"))); 
    curl_easy_setopt(request, CURLOPT_PASSWORD, json_object_get_string(json_object_object_get(loginInfo, "password"))); 
    if (postFields != NULL) {
        curl_easy_setopt(request, CURLOPT_POSTFIELDS, postFields);
    }
    if (httpHeaders != NULL) {
        curl_easy_setopt(request, CURLOPT_HTTPHEADER, httpHeaders);
    }
    curl_easy_setopt(request, CURLOPT_USERAGENT, json_object_get_string(json_object_object_get(loginInfo, "useragent"))); 
    curl_easy_setopt(request, CURLOPT_URL, targetURL);
    curl_easy_setopt(request, CURLOPT_WRITEFUNCTION, downloadToMem);
    curl_easy_setopt(request, CURLOPT_WRITEDATA, pageDL);

    for (int i = 0; i < 16; i++) {
        res = curl_easy_perform(request);
        if (res == CURLE_OK) {
            break;
        } else {
            printf("Download error %d\n", res);
            pageDL->size = 1;
            pageDL->contents = realloc(pageDL->contents, pageDL->size);
        }
    }

    curl_easy_cleanup(request);
    curl_global_cleanup();
    json_object_put(loginInfo);
}

struct memory * getToken(struct json_object * configFile) {
    int request_time;
    struct json_object * tokenRoot;
    struct memory * access_token;

    tokenRoot = json_object_from_file(json_object_get_string(json_object_object_get(configFile, "tokenFile")));
    request_time = json_object_get_int(json_object_object_get(tokenRoot, "request_time"));

    CREATEMEMSTRUCT(access_token, char);
    if ((request_time + 86400) > time(NULL)) {
        CATSTRTOMEMORYSTRUCT(access_token, json_object_get_string(json_object_object_get(tokenRoot, "access_token")));
    } else {
        char targetURL[] = "https://www.reddit.com/api/v1/access_token";
        struct memory * pageDL;
        struct memory * postFields;
        struct json_object * tokenDLRoot;

        CREATEMEMSTRUCT(pageDL, char);
        CREATEMEMSTRUCT(postFields, char);
        CATSTRTOMEMORYSTRUCT(postFields, "grant_type=refresh_token&refresh_token=");
        CATSTRTOMEMORYSTRUCT(postFields, json_object_get_string(json_object_object_get(tokenRoot, "refresh_token")));

        baseRequest(pageDL, targetURL, postFields->contents, NULL, configFile);
        
        if (pageDL->size != 1) {
            tokenDLRoot = json_tokener_parse(pageDL->contents);
            CATSTRTOMEMORYSTRUCT(access_token, json_object_get_string(json_object_object_get(tokenDLRoot, "access_token")));
        } else{
            printf("Unable to obtain access token.\n");
            exit(0);
        }

        json_object_object_add(tokenRoot, "access_token", json_object_new_string(access_token->contents));
        json_object_object_add(tokenRoot, "request_time", json_object_new_int(time(NULL)));

        json_object_to_file("token.json", tokenRoot);
        FREEMEMSTRUCT(postFields);
        FREEMEMSTRUCT(pageDL);
    }

    json_object_put(tokenRoot);
    return access_token;
}

struct json_object * httpRequest(char * targetURL, struct json_object * configFile) {
    printf("%s\n", targetURL);
    struct curl_slist * httpHeader;
    struct memory * accessToken;
    struct memory * authHeader;
    struct memory * pageDL;

    accessToken = getToken(configFile);

    CREATEMEMSTRUCT(authHeader, char);
    CATSTRTOMEMORYSTRUCT(authHeader, "Authorization: bearer ");
    MEMSTRUCTCAT(authHeader, accessToken);

    httpHeader = NULL;
    httpHeader = curl_slist_append(httpHeader, authHeader->contents);

    CREATEMEMSTRUCT(pageDL, char);

    baseRequest(pageDL, targetURL, NULL, httpHeader, configFile);

    if (pageDL->size == 1) {
        printf("Error in httpRequest().\n");
        FREEMEMSTRUCT(pageDL);
        FREEMEMSTRUCT(accessToken);
        FREEMEMSTRUCT(authHeader);
        curl_slist_free_all(httpHeader);
        return NULL;
    } else {
        curl_slist_free_all(httpHeader);
        FREEMEMSTRUCT(accessToken);
        FREEMEMSTRUCT(authHeader);
    
        struct json_tokener * deepTokener = json_tokener_new_ex(256);
        struct json_object * pageJSON = json_tokener_parse_ex(deepTokener, (char *) pageDL->contents, pageDL->size);
        json_tokener_free(deepTokener);
    
        FREEMEMSTRUCT(pageDL);
        return pageJSON;
    }
}  
