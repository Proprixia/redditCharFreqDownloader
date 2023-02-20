// Outputs the path array passed into it (used for telemetry).
#define OUTPUTPATHARRAY(pathArray) { \
    printf("["); \
    for (int i = 0; i < (int) (pathArray->size / sizeof(int)); i++) { \
        printf("%d", (int) * ((int *) (pathArray->contents + (i * sizeof(int))))); \
        if (i < (int) (pathArray->size / sizeof(int) - 1)) { \
            printf(", "); \
        } \
    } \
    printf("]\n"); \
}

// Creates and allocates a memory struct with name memStructName.
#define CREATEMEMSTRUCT(memStructName, type) { \
    memStructName = malloc(sizeof(struct memory)); \
    memStructName->size = sizeof(type); \
    memStructName->contents = malloc(memStructName->size); \
    memset(memStructName->contents, 0, memStructName->size); \
}

// Appends the text of "node" to "commentsText".
#define COPYTOCOMMENTSTEXT() { \
    if (json_object_is_type(node, json_type_string)) { \
        stats.numComments += 1; \
        commentsText->size += (json_object_get_string_len(node) + strlen("\n")); \
        commentsText->contents = realloc(commentsText->contents, commentsText->size); \
        strncat((char *) commentsText->contents, json_object_get_string(node), commentsText->size); \
        strncat((char *) commentsText->contents, "\n", commentsText->size); \
    } \
}

// Concatenates the memory struct src to memory struct dest.
#define MEMSTRUCTCAT(dest, src) { \
    dest->size += src->size; \
    dest->contents = realloc(dest->contents, dest->size); \
    memset(dest->contents + dest->size - src->size, '\0', src->size); \
    strncat(dest->contents, src->contents, src->size); \
}

// Uses the intArray and attribute to create json_object node.
#define NODEFROMARRAYANDATTRIBUTE(intArray, attribute) { \
    pathStr = genPathStr(intArray, attribute); \
    json_pointer_get(pageJSON, pathStr->contents, &node); \
    free(pathStr->contents); \
    free(pathStr); \
}

#define CATSTRTOMEMORYSTRUCT(dest, src) { \
    dest->size += (strlen((char *) src)); \
    dest->contents = realloc(dest->contents, dest->size); \
    strncat(dest->contents, src, dest->size); \
}

#define FREEMEMSTRUCT(memStruct) { \
    free(memStruct->contents); \
    free(memStruct); \
}

#define GETINTARRAYIDX(array, idx) ( \
    (int) * (int *) (array->contents + (idx * sizeof(int))) \
)
