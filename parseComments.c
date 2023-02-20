#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "net.c"
#include "macros.c"

struct subredditStats {
    int numComments;
};

// Returns the number of digits in a passed integer.
int getNumDigits (int numIn) {
    int numDigits = 1;
    int num = numIn;

    while (abs(num) >= 10) {
        num = (int) floor(abs(num) / 10);
        numDigits ++;
    }
    if (numIn < 0) {
        numDigits ++;
    }

    return numDigits;
}

// Converts the passed path int array (in memory struct form) and attribute string into a string with the necessary path, such that it can be queried to json_c.
struct memory * genPathStr(struct memory * pathArray, const char * attribute) {
    char pathFmtStr1[] = "/1/data/children/%d/data";
    char pathFtmStrPost1[] = "/replies/data/children/%d/data";
    int numDigits;
    struct memory * addition;
    struct memory * pathStr;
    
    CREATEMEMSTRUCT(addition, char);
    CREATEMEMSTRUCT(pathStr, char);

    free(addition->contents);

    for (int i = 0; i < (int) floor(pathArray->size / sizeof(int)); i++) {
        // Formats the addition string to include the next integer in the pathArray integer array.
        numDigits = getNumDigits(GETINTARRAYIDX(pathArray, i));
        if (i == 0) {
            addition->size = strlen(pathFmtStr1) - (strlen("%d") - 1) + numDigits;
            addition->contents = malloc(addition->size); 
            snprintf(addition->contents, addition->size, pathFmtStr1, GETINTARRAYIDX(pathArray, i)); 
            } else { 
                addition->size = strlen(pathFtmStrPost1) - (strlen("%d") - 1) + numDigits; addition->contents = malloc(addition->size); 
                snprintf(addition->contents, addition->size, pathFtmStrPost1, GETINTARRAYIDX(pathArray, i)); 
            }

            // Appends the most recent addition to pathStr.
            MEMSTRUCTCAT(pathStr, addition);
            free(addition->contents);
    }

    // Append the attribute string to pathStr 
    CATSTRTOMEMORYSTRUCT(pathStr, attribute);

    free(addition);
    return pathStr;
}

struct subredditStats parseComments(struct json_object * pageJSON, struct memory * commentsText) {
    struct subredditStats stats = {0};
    stats.numComments = 0;
    // Comment parsing algorithm: appends the text of every comment in the post (or at the first 11 deep) to the commentsText string.
    if (json_object_is_type(pageJSON, json_type_array)) {
    if (json_object_array_length(pageJSON) > 1) {
        int depth;
        int numComments;
        int risingUp;
        int rootDirLen;
        int workingDirLen; 
        struct json_object * node;
        struct memory * pathArray;
        struct memory * pathStr;

        CREATEMEMSTRUCT(pathArray, int);

        // Determines the number of tier 1 comments on the post.
        json_pointer_get(pageJSON, "/1/data/children", &node);
        if (json_object_is_type(node, json_type_array)) {
        rootDirLen = json_object_array_length(node);
        workingDirLen = rootDirLen;
        depth = 0;
        risingUp = 0;

        while (1) {
            // Finds the replies object to determine if there are replies to the current node.
            NODEFROMARRAYANDATTRIBUTE(pathArray, "/replies");

            if ((json_object_is_type(node, json_type_object)) && (risingUp == 0)) { // If the current node has unexplored children, move the cursor to its oldest child.
                depth += 1;

                // Determines the number of siblings the curent node's child, the new node, has.
                NODEFROMARRAYANDATTRIBUTE(pathArray, "/replies/data/children");
                if (json_object_is_type(node, json_type_array)) {
                    workingDirLen = json_object_array_length(node);

                    pathArray->size += sizeof(int);
                    pathArray->contents = realloc(pathArray->contents, pathArray->size);
                    *((int *) (pathArray->contents + (pathArray->size - sizeof(int)))) = 0;
                    risingUp = 0;
                } else {
                    json_object_put(pageJSON);
                    FREEMEMSTRUCT(pathArray);
                    break;
                }
            } else {
                // If the current node has one or more younger siblings, output the current node and move the cursor to its oldest younger sibling
                if((int) * (int *) (pathArray->contents + (depth * sizeof(int))) + 1 < workingDirLen) {
                    // Concatenate the body of the current node to the commentsText->contents string.
                    NODEFROMARRAYANDATTRIBUTE(pathArray, "/body");
                    COPYTOCOMMENTSTEXT();
                    numComments += 1;

                    // Change the cursor to point to the current node's oldest sibling.
                    * (int *) (pathArray->contents + (depth * sizeof(int))) += 1;
                    risingUp = 0;
                } else { // If the current node has neighter a younger sibling, nor an unexplored child, output the current node and move the cursor to its parent.
                    // Concatenate the body of the current node to the commentsText->contents string.
                    NODEFROMARRAYANDATTRIBUTE(pathArray, "/body");
                    COPYTOCOMMENTSTEXT();
                    numComments += 1;

                    depth --;
                    if (depth < 0) { // Triggers when cursor was on a tier one comment with no unexplored children and no younger siblings. In other words, the last comment to be procesed.
                        break;
                    }
                    
                    // Moves the cursor to the current node's parent.
                    pathArray->size -= sizeof(int);
                    pathArray->contents = realloc(pathArray->contents, pathArray->size);
                    risingUp = 1;

                    // Determines the number of nodes in the branch that the cursor is moving to.
                    if (depth == 0) {
                        workingDirLen = rootDirLen;
                    } else {
                        int lastElementTemp = (int) * (int *) (pathArray->contents + (pathArray->size - sizeof(int)));
                        pathArray->size -= sizeof(int);
                        pathArray->contents = realloc(pathArray->contents, pathArray->size);

                        NODEFROMARRAYANDATTRIBUTE(pathArray, "/replies/data/children");
                        workingDirLen = json_object_array_length(node);

                        pathArray->size += sizeof(int);
                        pathArray->contents = realloc(pathArray->contents, pathArray->size);
                        *((int *) (pathArray->contents + (pathArray->size - sizeof(int)))) = lastElementTemp;
                    }
                }
            }
        }
        free(pathArray->contents);
        free(pathArray);

        printf("rootDirLen-1=%d\n", rootDirLen - 1);
        json_pointer_getf(pageJSON, &node, "/1/data/children/%d/kind", rootDirLen - 1);
        if (strncmp(json_object_get_string(node), "more", 4) == 0) {
            printf("Getting more.\n");
            json_pointer_getf(pageJSON, &node, "/1/data/children/%d/data/children", rootDirLen - 1);
            
            if (json_object_is_type(node, json_type_array)) {
                printf("Json is type array!\n");
                int numMoreComments = json_object_array_length(node);
                struct memory * moreCommentIDs;
                CREATEMEMSTRUCT(moreCommentIDs, char);
            
                for (int i = 0; i < numMoreComments; i++) {
                    json_pointer_getf(pageJSON, &node, "/1/data/children/%d/data/children/%d", rootDirLen - 1, i);
                    CATSTRTOMEMORYSTRUCT(moreCommentIDs, json_object_get_string(node));
                }

                moreCommentIDs->size -= 1;
                moreCommentIDs->contents = realloc(moreCommentIDs->contents, moreCommentIDs->size);
                
                struct memory * commentIDsParameter;
                CREATEMEMSTRUCT(commentIDsParameter, char);
                commentIDsParameter->size = (int) (moreCommentIDs->size / 7) * 8;
                commentIDsParameter->contents = realloc(commentIDsParameter->contents, commentIDsParameter->size);
                memset(commentIDsParameter->contents, '\0', commentIDsParameter->size);
                int i = 0;
                for (; i < moreCommentIDs->size; i++) {
                    strncat((char *) commentIDsParameter->contents, (char *) (moreCommentIDs->contents + i), 1);
                    if ((i % 7 == 6) && (i != (moreCommentIDs->size - 1))) {
                        strcat((char *) commentIDsParameter->contents, ",");
                    }
                }

                FREEMEMSTRUCT(moreCommentIDs);

                struct memory * moreCommentsURL;
                CREATEMEMSTRUCT(moreCommentsURL, char);
                CATSTRTOMEMORYSTRUCT(moreCommentsURL, "https://oauth.reddit.com/api/morechildren?api_type=json&raw_json=1&link_id=");
                json_pointer_getf(pageJSON, &node, "/1/data/children/%d/data/parent_id", rootDirLen - 1);
                CATSTRTOMEMORYSTRUCT(moreCommentsURL, json_object_get_string(node));
                CATSTRTOMEMORYSTRUCT(moreCommentsURL, "&children=");
                
                struct json_tokener * deepTokener = json_tokener_new_ex(256);
                printf("numMoreComments: %d\n", numMoreComments);

                int newDataSize;

                for (int j = 0; j < (int) (ceil(numMoreComments / 100) + 1); j++) {
                    printf("j = %d\ncommentIDsParameter->size = %d\n", j, commentIDsParameter->size);

                    if (j < (int) (ceil(numMoreComments / 100))) {
                        newDataSize = 800;
                    } else {
                        newDataSize = (int) ceil(((commentIDsParameter->size / 8) % 100) * 8);
                    }
                    printf("newDataSize = %d\n", newDataSize);

                    moreCommentsURL->size += newDataSize;
                    moreCommentsURL->contents = realloc(moreCommentsURL->contents, moreCommentsURL->size);
                    strncat((char *) moreCommentsURL->contents, (char *) commentIDsParameter->contents + (j * 800), newDataSize);
                    if (* (char *) (moreCommentsURL->contents + moreCommentsURL->size - 2) == ',') {
                        * (char *) (moreCommentsURL->contents + moreCommentsURL->size - 2) = '\0';
                    }

                    struct json_object * moreCommentsPageJSON = httpRequest(moreCommentsURL->contents);

                    if (moreCommentsPageJSON != NULL) {
                        json_pointer_get(moreCommentsPageJSON, "/json/data/things", &node);
                        if (json_object_is_type(node, json_type_array)) {
                            int moreCommentsArrayLen = json_object_array_length(node);

                            for (int i = 0; i < moreCommentsArrayLen; i++) {
                                json_pointer_getf(moreCommentsPageJSON, &node, "/json/data/things/%d/kind", i);
                                if (strncmp(json_object_get_string(node), "t1", 2) == 0) {
                                    json_pointer_getf(moreCommentsPageJSON, &node, "/json/data/things/%d/data/body", i);
                                    COPYTOCOMMENTSTEXT();
                                }
                            }

                            json_object_put(moreCommentsPageJSON);
                        } else {
                            if(json_object_is_type(moreCommentsPageJSON, json_type_object)) {
                                json_object_put(moreCommentsPageJSON);
                            }
                            break;
                        }
                    }

                    moreCommentsURL->size -= newDataSize;
                    moreCommentsURL->contents = realloc(moreCommentsURL->contents, moreCommentsURL->size);
                    memset(moreCommentsURL->contents + moreCommentsURL->size - 1, '\0', 1);
                }
                json_tokener_free(deepTokener);

                FREEMEMSTRUCT(commentIDsParameter);
                FREEMEMSTRUCT(moreCommentsURL);
            }
            }
        }
    }
    }
    return stats;
}