#include "parseComments.c"

struct subredditStats getPost(char * url, struct memory * userText) {
    struct subredditStats stats;
    struct json_object * node;
    struct json_object * pageJSON;

    pageJSON = httpRequest(url);

    if (pageJSON != NULL) {
        json_pointer_get(pageJSON, "/0/data/children/0/data/selftext", &node);

        if (json_object_is_type(node, json_type_string)) {
            if (json_object_get_string_len(node) > 0) {
                CATSTRTOMEMORYSTRUCT(userText, json_object_get_string(node));
            }
    }

    stats = parseComments(pageJSON, userText);
    }

    json_object_put(pageJSON);

    return stats;
};

void getSubreddit(char * subredditName) {
    printf("%s\n", subredditName);
    char countStr[] = "&raw_json=1&count=100&after=";
    FILE * destFile;
    int dist;
    int subURLSizeBeforeModification;
    struct json_object * node;
    struct json_object * pageJSON;
    struct json_object * statsJSON;
    struct memory * pageURL;
    struct memory * permalink;
    struct memory * outputFileName;
    struct memory * statsFileName;
    struct memory * subredditURL;
    struct memory * userText;
    struct subredditStats postStats;
    struct subredditStats stats;

    stats.numComments = 0;

    CREATEMEMSTRUCT(subredditURL, char);
    CREATEMEMSTRUCT(permalink, char);
    CREATEMEMSTRUCT(userText, char);
    CREATEMEMSTRUCT(pageURL, char);

    CATSTRTOMEMORYSTRUCT(subredditURL, "https://oauth.reddit.com/r/");
    CATSTRTOMEMORYSTRUCT(subredditURL, (char *) subredditName);
    CATSTRTOMEMORYSTRUCT(subredditURL, "/top?sort=top&t=all&raw_json=1");

    subURLSizeBeforeModification = subredditURL->size;

    char pathStr[35];
    
    for (int j = 0; j < 4; j++) {
        pageJSON = httpRequest(subredditURL->contents);

        if (pageJSON != NULL) {
            
            if (json_object_object_get(pageJSON, "reason") == NULL) {
            json_pointer_get(pageJSON, "/data/dist", &node);
            if (json_object_is_type(node, json_type_int)) {
                dist = json_object_get_int(node);
                for (int i = 0; i < dist; i++) {
                    sprintf(pathStr, "/data/children/%d/data/permalink", i);

                    FREEMEMSTRUCT(pageURL);
                    CREATEMEMSTRUCT(pageURL, char);
                    * (char *) pageURL->contents = '\0';
                    CATSTRTOMEMORYSTRUCT(pageURL, "https://oauth.reddit.com");
                    json_pointer_get(pageJSON, pathStr, &node);

                    if (json_object_is_type(node, json_type_string)) {
                        CATSTRTOMEMORYSTRUCT(pageURL, json_object_get_string(node));
                        CATSTRTOMEMORYSTRUCT(pageURL, "?raw_json=1");

                        postStats = getPost(pageURL->contents, userText);

                        stats.numComments += postStats.numComments;
        
                    }
                }
            }

            json_pointer_get(pageJSON, "/data/after", &node);
            if (json_object_is_type(node, json_type_string)) {
                subredditURL->size = subURLSizeBeforeModification;
                subredditURL->contents = realloc(subredditURL->contents, subredditURL->size);
                * (char *) (subredditURL->contents + subredditURL->size - 1) = '\0';

                sprintf(countStr, "&raw_json=1&count=%d&after=", (j + 1) * 25);

                CATSTRTOMEMORYSTRUCT(subredditURL, countStr);
                CATSTRTOMEMORYSTRUCT(subredditURL, json_object_get_string(node));
            } else {
                json_object_put(pageJSON);
                break;
            }
            json_object_put(pageJSON);
        } else {
            json_object_put(pageJSON);
            break;
        }
        } else {
            json_object_put(pageJSON);
            break;
        }

    }

    CREATEMEMSTRUCT(outputFileName, char);
    CATSTRTOMEMORYSTRUCT(outputFileName, "./Out/");    
    CATSTRTOMEMORYSTRUCT(outputFileName, subredditName);
    destFile = fopen((char *) outputFileName->contents, "w");
    fputs(userText->contents, destFile);
    fclose(destFile);
    FREEMEMSTRUCT(outputFileName);

    statsJSON = json_object_new_object();
    json_object_object_add(statsJSON, "num_comments", json_object_new_int(stats.numComments));
    CREATEMEMSTRUCT(statsFileName, char);
    CATSTRTOMEMORYSTRUCT(statsFileName, "./Out/");
    CATSTRTOMEMORYSTRUCT(statsFileName, subredditName);
    CATSTRTOMEMORYSTRUCT(statsFileName, "Stats.json");
    json_object_to_file(statsFileName->contents, statsJSON);
    json_object_put(statsJSON);
    FREEMEMSTRUCT(statsFileName);

    FREEMEMSTRUCT(userText); 
    FREEMEMSTRUCT(permalink); 
    FREEMEMSTRUCT(pageURL); 
    FREEMEMSTRUCT(subredditURL);
}

int main() {
    FILE * subredditsFile;
    int fileSize;
    struct memory * fileText;
    struct memory * subredditName;

    subredditsFile = fopen("subreddits", "r");
    fseek(subredditsFile, 0, SEEK_END);
    fileSize = ftell(subredditsFile);
    rewind(subredditsFile);

    CREATEMEMSTRUCT(fileText, char);
    fileText->size = fileSize + 1;
    fileText->contents = realloc(fileText->contents, fileText->size);
    
    char c;
    for (int i = 0; i < fileText->size; i++) {
        c = fgetc(subredditsFile);
        * (char *) (fileText->contents + i) = c;
    }
    * (char *) (fileText->contents + fileText->size - 1) = '\0';

    CREATEMEMSTRUCT(subredditName, char);
    * (char *) subredditName->contents = '\0';
    for (int i = 0; i < fileText->size; i++) {
        if (* (char *) (fileText->contents + i) == '\n') {
            if (* (char *) (fileText->contents + i - 1) == '\n') {
                break;
            } else {
                getSubreddit((char *) subredditName->contents);
                subredditName->size = 1;
                subredditName->contents = realloc(subredditName->contents, subredditName->size);
            }
        } else {
            * (char *) (subredditName->contents + subredditName->size - 1) = * (char *) (fileText->contents + i);
            subredditName->size += 1;
            subredditName->contents = realloc(subredditName->contents, subredditName->size);
            * (char *) (subredditName->contents + subredditName->size - 1) = '\0';
        }
    }

    fclose(subredditsFile);
    FREEMEMSTRUCT(fileText);
    FREEMEMSTRUCT(subredditName);
}