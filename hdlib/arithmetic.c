/* Implementation of arithmetic operators in C */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdbool.h>
#include <time.h>

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

/* Function prototypes */
Vector *create_vector(const char *name, int size, const char *vtype, int seed, bool warning);
void free_vector(Vector *vec);
void print_vector(Vector *vec);
Vector *bind_vectors(Vector *vec1, Vector *vec2);
Vector *bundle_vectors(Vector *vec1, Vector *vec2);
Vector *subtract_vectors(Vector *vec1, Vector *vec2);
Vector *permute_vector(Vector *vec, int rotate_by);

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
        if ((i + 1) % 10 == 0 && i != vec->size - 1) {
            printf("\n");
        }
    }
    printf("]\n");
}

/* Bind two vectors */
Vector *bind_vectors(Vector *vec1, Vector *vec2) {
    if (vec1->size != vec2->size) {
        fprintf(stderr, "Vectors must have the same size\n");
        exit(EXIT_FAILURE);
    }
    if (strcmp(vec1->vtype, vec2->vtype) != 0) {
        fprintf(stderr, "Vector types are not compatible\n");
        exit(EXIT_FAILURE);
    }

    Vector *result = create_vector("BindResult", vec1->size, vec1->vtype, -1, false);
    for (int i = 0; i < vec1->size; i++) {
        result->vector[i] = vec1->vector[i] * vec2->vector[i];
    }

    /* Merge tags */
    // Assuming tags are merged; implement tag handling as needed.

    return result;
}

/* Bundle two vectors */
Vector *bundle_vectors(Vector *vec1, Vector *vec2) {
    if (vec1->size != vec2->size) {
        fprintf(stderr, "Vectors must have the same size\n");
        exit(EXIT_FAILURE);
    }
    if (strcmp(vec1->vtype, vec2->vtype) != 0) {
        fprintf(stderr, "Vector types are not compatible\n");
        exit(EXIT_FAILURE);
    }

    Vector *result = create_vector("BundleResult", vec1->size, vec1->vtype, -1, false);
    for (int i = 0; i < vec1->size; i++) {
        result->vector[i] = vec1->vector[i] + vec2->vector[i];
    }

    /* Merge tags */
    // Assuming tags are merged; implement tag handling as needed.

    return result;
}

/* Subtract one vector from another */
Vector *subtract_vectors(Vector *vec1, Vector *vec2) {
    if (vec1->size != vec2->size) {
        fprintf(stderr, "Vectors must have the same size\n");
        exit(EXIT_FAILURE);
    }
    if (strcmp(vec1->vtype, vec2->vtype) != 0) {
        fprintf(stderr, "Vector types are not compatible\n");
        exit(EXIT_FAILURE);
    }

    Vector *result = create_vector("SubtractResult", vec1->size, vec1->vtype, -1, false);
    for (int i = 0; i < vec1->size; i++) {
        result->vector[i] = vec1->vector[i] - vec2->vector[i];
    }

    /* Copy tags from vec1 */
    // Implement tag copying as needed.

    return result;
}

/* Permute a vector */
Vector *permute_vector(Vector *vec, int rotate_by) {
    Vector *result = create_vector("PermuteResult", vec->size, vec->vtype, -1, false);

    for (int i = 0; i < vec->size; i++) {
        int new_index = (i + rotate_by) % vec->size;
        result->vector[new_index] = vec->vector[i];
    }

    /* Copy tags */
    // Implement tag copying as needed.

    return result;
}

/* Example usage */
int main() {
    /* Create two vectors */
    Vector *vec1 = create_vector("Vector1", 10000, "bipolar", 1, false);
    Vector *vec2 = create_vector("Vector2", 10000, "bipolar", 2, false);

    /* Bind vectors */
    Vector *bound_vec = bind_vectors(vec1, vec2);
    printf("Bound Vector:\n");
    print_vector(bound_vec);

    /* Bundle vectors */
    Vector *bundled_vec = bundle_vectors(vec1, vec2);
    printf("Bundled Vector:\n");
    print_vector(bundled_vec);

    /* Subtract vectors */
    Vector *subtracted_vec = subtract_vectors(vec1, vec2);
    printf("Subtracted Vector:\n");
    print_vector(subtracted_vec);

    /* Permute vector */
    Vector *permuted_vec = permute_vector(vec1, 2);
    printf("Permuted Vector:\n");
    print_vector(permuted_vec);

    /* Clean up */
    free_vector(vec1);
    free_vector(vec2);
    free_vector(bound_vec);
    free_vector(bundled_vec);
    free_vector(subtracted_vec);
    free_vector(permuted_vec);

    return 0;
}
