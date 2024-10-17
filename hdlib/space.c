/* Implementation of hyperdimensional Vector and Space in C */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <errno.h>
#include <uuid/uuid.h>
#include <stdbool.h>

/* Define the Vector structure */
typedef struct Vector {
    char *name;
    int size;
    int *vector;
    char *vtype; // "binary" or "bipolar"
    char **tags;
    int tags_count;
    int seed;
    bool warning;
} Vector;

/* Define the Space structure */
typedef struct Space {
    Vector **vectors;
    int vector_count;
    int size;
    char *vtype; // "binary" or "bipolar"
    char **tags;
    int tags_count;
} Space;

/* Function prototypes */
Vector *create_vector(const char *name, int size, const char *vtype, int seed, bool warning);
void free_vector(Vector *vec);
void print_vector(Vector *vec);
Space *create_space(int size, const char *vtype);
void free_space(Space *space);
void insert_vector(Space *space, Vector *vec);
void print_space(Space *space);
double vector_distance(Vector *vec1, Vector *vec2, const char *method);
void normalize_vector(Vector *vec);
Vector *bind_vectors(Vector *vec1, Vector *vec2);
Vector *bundle_vectors(Vector *vec1, Vector *vec2);
Vector *subtract_vectors(Vector *vec1, Vector *vec2);
void permute_vector(Vector *vec, int rotate_by);

/* Function implementations */

/* Create a new Vector */
Vector *create_vector(const char *name, int size, const char *vtype, int seed, bool warning) {
    if (size < 10000) {
        fprintf(stderr, "Vector size must be greater than or equal to 10000\n");
        exit(EXIT_FAILURE);
    }

    Vector *vec = (Vector *)malloc(sizeof(Vector));
    if (!vec) {
        perror("Failed to allocate memory for Vector");
        exit(EXIT_FAILURE);
    }

    vec->name = strdup(name);
    vec->size = size;
    vec->vtype = strdup(vtype);
    vec->seed = seed;
    vec->warning = warning;
    vec->tags = NULL;
    vec->tags_count = 0;

    /* Initialize the vector */
    vec->vector = (int *)malloc(size * sizeof(int));
    if (!vec->vector) {
        perror("Failed to allocate memory for vector elements");
        free(vec);
        exit(EXIT_FAILURE);
    }

    /* Seed the random number generator */
    if (seed != -1) {
        srand(seed);
    } else {
        srand(time(NULL));
    }

    /* Generate random vector */
    for (int i = 0; i < size; i++) {
        int rand_value = rand() % 2;
        if (strcmp(vtype, "binary") == 0) {
            vec->vector[i] = rand_value;
        } else if (strcmp(vtype, "bipolar") == 0) {
            vec->vector[i] = rand_value == 0 ? -1 : 1;
        } else {
            fprintf(stderr, "Vector type can be binary or bipolar only\n");
            free_vector(vec);
            exit(EXIT_FAILURE);
        }
    }

    return vec;
}

/* Free a Vector */
void free_vector(Vector *vec) {
    if (vec) {
        free(vec->name);
        free(vec->vtype);
        free(vec->vector);
        if (vec->tags) {
            for (int i = 0; i < vec->tags_count; i++) {
                free(vec->tags[i]);
            }
            free(vec->tags);
        }
        free(vec);
    }
}

/* Print a Vector */
void print_vector(Vector *vec) {
    printf("Vector Name: %s\n", vec->name);
    printf("Size: %d\n", vec->size);
    printf("Type: %s\n", vec->vtype);
    printf("Tags: ");
    for (int i = 0; i < vec->tags_count; i++) {
        printf("%s ", vec->tags[i]);
    }
    printf("\nVector Elements: [");
    for (int i = 0; i < vec->size; i++) {
        printf("%d ", vec->vector[i]);
    }
    printf("]\n");
}

/* Create a new Space */
Space *create_space(int size, const char *vtype) {
    if (size < 10000) {
        fprintf(stderr, "Size of vectors in space must be greater than or equal to 10000\n");
        exit(EXIT_FAILURE);
    }

    Space *space = (Space *)malloc(sizeof(Space));
    if (!space) {
        perror("Failed to allocate memory for Space");
        exit(EXIT_FAILURE);
    }

    space->vectors = NULL;
    space->vector_count = 0;
    space->size = size;
    space->vtype = strdup(vtype);
    space->tags = NULL;
    space->tags_count = 0;

    return space;
}

/* Free a Space */
void free_space(Space *space) {
    if (space) {
        for (int i = 0; i < space->vector_count; i++) {
            free_vector(space->vectors[i]);
        }
        free(space->vectors);
        free(space->vtype);
        if (space->tags) {
            for (int i = 0; i < space->tags_count; i++) {
                free(space->tags[i]);
            }
            free(space->tags);
        }
        free(space);
    }
}

/* Insert a Vector into a Space */
void insert_vector(Space *space, Vector *vec) {
    if (space->size != vec->size) {
        fprintf(stderr, "Space and vectors with different size are not compatible\n");
        exit(EXIT_FAILURE);
    }
    if (strcmp(space->vtype, vec->vtype) != 0) {
        fprintf(stderr, "Attempting to insert a %s vector into a %s space: failed\n", vec->vtype, space->vtype);
        exit(EXIT_FAILURE);
    }

    /* Check if vector name already exists */
    for (int i = 0; i < space->vector_count; i++) {
        if (strcmp(space->vectors[i]->name, vec->name) == 0) {
            fprintf(stderr, "Vector \"%s\" already in space\n", vec->name);
            exit(EXIT_FAILURE);
        }
    }

    /* Insert the vector */
    space->vector_count++;
    space->vectors = (Vector **)realloc(space->vectors, space->vector_count * sizeof(Vector *));
    if (!space->vectors) {
        perror("Failed to allocate memory for vectors in space");
        exit(EXIT_FAILURE);
    }
    space->vectors[space->vector_count - 1] = vec;
}

/* Print a Space */
void print_space(Space *space) {
    printf("Space Size: %d\n", space->size);
    printf("Vector Type: %s\n", space->vtype);
    printf("Number of Vectors: %d\n", space->vector_count);
    printf("Vectors:\n");
    for (int i = 0; i < space->vector_count; i++) {
        printf("  %s\n", space->vectors[i]->name);
    }
}

/* Calculate distance between two vectors */
double vector_distance(Vector *vec1, Vector *vec2, const char *method) {
    if (vec1->size != vec2->size) {
        fprintf(stderr, "Vectors must have the same size\n");
        exit(EXIT_FAILURE);
    }
    if (strcmp(vec1->vtype, vec2->vtype) != 0) {
        fprintf(stderr, "Vectors must be of the same type\n");
        exit(EXIT_FAILURE);
    }

    if (strcmp(method, "cosine") == 0) {
        double dot_product = 0.0;
        double norm_a = 0.0;
        double norm_b = 0.0;
        for (int i = 0; i < vec1->size; i++) {
            dot_product += vec1->vector[i] * vec2->vector[i];
            norm_a += vec1->vector[i] * vec1->vector[i];
            norm_b += vec2->vector[i] * vec2->vector[i];
        }
        return 1.0 - (dot_product / (sqrt(norm_a) * sqrt(norm_b)));
    } else if (strcmp(method, "hamming") == 0) {
        int count = 0;
        for (int i = 0; i < vec1->size; i++) {
            if (vec1->vector[i] != vec2->vector[i]) {
                count++;
            }
        }
        return (double)count;
    } else if (strcmp(method, "euclidean") == 0) {
        double sum = 0.0;
        for (int i = 0; i < vec1->size; i++) {
            double diff = vec1->vector[i] - vec2->vector[i];
            sum += diff * diff;
        }
        return sqrt(sum);
    } else {
        fprintf(stderr, "Distance method \"%s\" is not supported\n", method);
        exit(EXIT_FAILURE);
    }
}

/* Normalize a vector */
void normalize_vector(Vector *vec) {
    if (strcmp(vec->vtype, "binary") != 0 && strcmp(vec->vtype, "bipolar") != 0) {
        fprintf(stderr, "Vector type is not supported\n");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < vec->size; i++) {
        if (vec->vector[i] > 0) {
            vec->vector[i] = 1;
        } else {
            vec->vector[i] = strcmp(vec->vtype, "binary") == 0 ? 0 : -1;
        }
    }
}

/* Bind two vectors */
Vector *bind_vectors(Vector *vec1, Vector *vec2) {
    if (vec1->size != vec2->size) {
        fprintf(stderr, "Vectors must have the same size\n");
        exit(EXIT_FAILURE);
    }
    Vector *result = create_vector(vec1->name, vec1->size, vec1->vtype, -1, false);
    for (int i = 0; i < vec1->size; i++) {
        result->vector[i] = vec1->vector[i] * vec2->vector[i];
    }
    return result;
}

/* Bundle two vectors */
Vector *bundle_vectors(Vector *vec1, Vector *vec2) {
    if (vec1->size != vec2->size) {
        fprintf(stderr, "Vectors must have the same size\n");
        exit(EXIT_FAILURE);
    }
    Vector *result = create_vector(vec1->name, vec1->size, vec1->vtype, -1, false);
    for (int i = 0; i < vec1->size; i++) {
        result->vector[i] = vec1->vector[i] + vec2->vector[i];
    }
    return result;
}

/* Subtract one vector from another */
Vector *subtract_vectors(Vector *vec1, Vector *vec2) {
    if (vec1->size != vec2->size) {
        fprintf(stderr, "Vectors must have the same size\n");
        exit(EXIT_FAILURE);
    }
    Vector *result = create_vector(vec1->name, vec1->size, vec1->vtype, -1, false);
    for (int i = 0; i < vec1->size; i++) {
        result->vector[i] = vec1->vector[i] - vec2->vector[i];
    }
    return result;
}

/* Permute a vector */
void permute_vector(Vector *vec, int rotate_by) {
    int *temp = (int *)malloc(vec->size * sizeof(int));
    if (!temp) {
        perror("Failed to allocate memory for permutation");
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i < vec->size; i++) {
        temp[(i + rotate_by) % vec->size] = vec->vector[i];
    }
    memcpy(vec->vector, temp, vec->size * sizeof(int));
    free(temp);
}

/* Example usage */
int main() {
    /* Create two vectors */
    Vector *vec1 = create_vector("Vector1", 10000, "bipolar", 1, false);
    Vector *vec2 = create_vector("Vector2", 10000, "bipolar", 2, false);

    /* Print vectors */
    /* print_vector(vec1); */
    /* print_vector(vec2); */

    /* Calculate distance */
    double dist = vector_distance(vec1, vec2, "cosine");
    printf("Cosine distance between vec1 and vec2: %f\n", dist);

    /* Bind vectors */
    Vector *bound_vec = bind_vectors(vec1, vec2);
    /* Normalize the bound vector */
    normalize_vector(bound_vec);
    /* print_vector(bound_vec); */

    /* Create a space and insert vectors */
    Space *space = create_space(10000, "bipolar");
    insert_vector(space, vec1);
    insert_vector(space, vec2);
    insert_vector(space, bound_vec);

    /* Print space */
    print_space(space);

    /* Clean up */
    free_space(space);

    return 0;
}
