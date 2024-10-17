/* Implementation of the MLModel structure in C */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>
#include <time.h>

/* Assuming the Vector and Space structures and functions are defined as in previous implementations */
/* Include the definitions of Vector and Space here or in a separate header file */

/* Define the MLModel structure */
typedef struct MLModel {
    int size;
    int levels;
    char *vtype; // "binary" or "bipolar"
    Space *space;
    char **classes;
    int classes_count;
    char *version;
    // You can add more fields as needed
} MLModel;

/* Function prototypes */
MLModel *create_mlmodel(int size, int levels, const char *vtype);
void free_mlmodel(MLModel *model);
void fit_mlmodel(MLModel *model, double **points, int num_points, int num_features, char **labels, int num_labels, int seed);
void predict_mlmodel(MLModel *model, int *test_indices, int num_test_indices, char **predictions, int *retraining_iterations, double *model_error_rate);
void cross_val_predict_mlmodel(MLModel *model, double **points, int num_points, int num_features, char **labels, int num_labels, int cv);
void auto_tune_mlmodel(MLModel *model, double **points, int num_points, int num_features, char **labels, int num_labels, int *size_range, int size_range_length, int *levels_range, int levels_range_length, int cv);
void stepwise_regression_mlmodel(MLModel *model, double **points, int num_points, int num_features, char **features, int num_features_list, char **labels, int num_labels, const char *method, int cv);

/* Additional helper functions */
Vector *get_vector_from_space(Space *space, const char *name);

/* Function implementations */

/* Create a new MLModel */
MLModel *create_mlmodel(int size, int levels, const char *vtype) {
    if (size < 10000) {
        fprintf(stderr, "Vectors size must be greater than or equal to 10000\n");
        exit(EXIT_FAILURE);
    }
    if (levels < 2) {
        fprintf(stderr, "The number of levels must be greater than or equal to 2\n");
        exit(EXIT_FAILURE);
    }
    if (strcmp(vtype, "binary") != 0 && strcmp(vtype, "bipolar") != 0) {
        fprintf(stderr, "Vector type can be binary or bipolar only\n");
        exit(EXIT_FAILURE);
    }
    MLModel *model = (MLModel *)malloc(sizeof(MLModel));
    if (!model) {
        perror("Failed to allocate memory for MLModel");
        exit(EXIT_FAILURE);
    }
    model->size = size;
    model->levels = levels;
    model->vtype = strdup(vtype);
    model->space = create_space(size, vtype);
    model->classes = NULL;
    model->classes_count = 0;
    model->version = strdup("0.1.17"); // Assuming version
    return model;
}

/* Free an MLModel */
void free_mlmodel(MLModel *model) {
    if (model) {
        free_space(model->space);
        free(model->vtype);
        for (int i = 0; i < model->classes_count; i++) {
            free(model->classes[i]);
        }
        free(model->classes);
        free(model->version);
        free(model);
    }
}

/* Fit the MLModel */
void fit_mlmodel(MLModel *model, double **points, int num_points, int num_features, char **labels, int num_labels, int seed) {
    if (num_points < 3) {
        fprintf(stderr, "Not enough data points\n");
        exit(EXIT_FAILURE);
    }
    if (labels && num_points != num_labels) {
        fprintf(stderr, "The number of data points does not match the number of class labels\n");
        exit(EXIT_FAILURE);
    }
    if (labels) {
        // Collect unique classes
        model->classes = (char **)malloc(num_labels * sizeof(char *));
        model->classes_count = 0;
        for (int i = 0; i < num_labels; i++) {
            bool exists = false;
            for (int j = 0; j < model->classes_count; j++) {
                if (strcmp(labels[i], model->classes[j]) == 0) {
                    exists = true;
                    break;
                }
            }
            if (!exists) {
                model->classes[model->classes_count++] = strdup(labels[i]);
            }
        }
        if (model->classes_count < 2) {
            fprintf(stderr, "The number of unique class labels must be > 1\n");
            exit(EXIT_FAILURE);
        }
    }
    // Initialize random number generator
    if (seed != -1) {
        srand(seed);
    } else {
        srand(time(NULL));
    }
    // Create level vectors
    int index_vector_size = model->size;
    int *index_vector = (int *)malloc(index_vector_size * sizeof(int));
    for (int i = 0; i < index_vector_size; i++) {
        index_vector[i] = i;
    }
    int next_level = (int)((model->size / 2) / model->levels);
    int change = model->size / 2;
    // Initialize base vector
    int *base_vector = (int *)malloc(model->size * sizeof(int));
    for (int i = 0; i < model->size; i++) {
        base_vector[i] = strcmp(model->vtype, "bipolar") == 0 ? -1 : 0;
    }
    // Find min and max values in points
    double min_value = INFINITY;
    double max_value = -INFINITY;
    for (int i = 0; i < num_points; i++) {
        for (int j = 0; j < num_features; j++) {
            if (points[i][j] < min_value) {
                min_value = points[i][j];
            }
            if (points[i][j] > max_value) {
                max_value = points[i][j];
            }
        }
    }
    double gap = (max_value - min_value) / model->levels;
    // Create level vectors
    for (int level_count = 0; level_count < model->levels; level_count++) {
        char level_name[50];
        sprintf(level_name, "level_%d", level_count);
        if (level_count == 0) {
            // Flip bits
            for (int i = 0; i < change; i++) {
                int index = rand() % index_vector_size;
                base_vector[index] *= -1;
            }
        } else {
            for (int i = 0; i < next_level; i++) {
                int index = rand() % index_vector_size;
                base_vector[index] *= -1;
            }
        }
        // Create vector
        Vector *level_vector = create_vector(level_name, model->size, model->vtype, -1, false);
        memcpy(level_vector->vector, base_vector, model->size * sizeof(int));
        insert_vector(model->space, level_vector);
    }
    // Encode data points
    for (int point_idx = 0; point_idx < num_points; point_idx++) {
        Vector *sum_vector = NULL;
        for (int feature_idx = 0; feature_idx < num_features; feature_idx++) {
            double value = points[point_idx][feature_idx];
            int level_count = 0;
            if (value == min_value) {
                level_count = 0;
            } else if (value == max_value) {
                level_count = model->levels - 1;
            } else {
                for (int level_position = 0; level_position < model->levels; level_position++) {
                    double left_bound = min_value + (level_position - 1) * gap;
                    double right_bound = min_value + level_position * gap;
                    if (left_bound <= value && value < right_bound) {
                        level_count = level_position;
                        break;
                    }
                }
            }
            char level_name[50];
            sprintf(level_name, "level_%d", level_count);
            Vector *level_vector = get_vector_from_space(model->space, level_name);
            Vector *rolled_vector = permute_vector(level_vector, feature_idx);
            if (!sum_vector) {
                sum_vector = rolled_vector;
            } else {
                Vector *temp = bundle_vectors(sum_vector, rolled_vector);
                free_vector(sum_vector);
                sum_vector = temp;
                free_vector(rolled_vector);
            }
        }
        char point_name[50];
        sprintf(point_name, "point_%d", point_idx);
        sum_vector->name = strdup(point_name);
        insert_vector(model->space, sum_vector);
        if (labels) {
            // Add tag (class label)
            add_tag(sum_vector, labels[point_idx]);
        }
    }
    free(index_vector);
    free(base_vector);
}

/* Predict using the MLModel */
void predict_mlmodel(MLModel *model, int *test_indices, int num_test_indices, char **predictions, int *retraining_iterations, double *model_error_rate) {
    if (num_test_indices == 0) {
        fprintf(stderr, "No test indices have been provided\n");
        exit(EXIT_FAILURE);
    }
    // Retrieve test and training vectors
    Vector **test_vectors = (Vector **)malloc(num_test_indices * sizeof(Vector *));
    int test_count = 0;
    Vector **training_vectors = NULL;
    int training_count = 0;
    for (int i = 0; i < model->space->vector_count; i++) {
        Vector *vec = model->space->vectors[i];
        if (strncmp(vec->name, "point_", 6) == 0) {
            int point_idx = atoi(vec->name + 6);
            bool is_test = false;
            for (int j = 0; j < num_test_indices; j++) {
                if (point_idx == test_indices[j]) {
                    test_vectors[test_count++] = vec;
                    is_test = true;
                    break;
                }
            }
            if (!is_test) {
                training_vectors = (Vector **)realloc(training_vectors, (training_count + 1) * sizeof(Vector *));
                training_vectors[training_count++] = vec;
            }
        }
    }
    if (test_count != num_test_indices) {
        fprintf(stderr, "Unable to retrieve all the test vectors from the space\n");
        exit(EXIT_FAILURE);
    }
    // Build class vectors
    Vector **class_vectors = (Vector **)malloc(model->classes_count * sizeof(Vector *));
    for (int class_idx = 0; class_idx < model->classes_count; class_idx++) {
        Vector *class_vector = NULL;
        for (int i = 0; i < training_count; i++) {
            if (has_tag(training_vectors[i], model->classes[class_idx])) {
                if (!class_vector) {
                    class_vector = copy_vector(training_vectors[i]);
                } else {
                    Vector *temp = bundle_vectors(class_vector, training_vectors[i]);
                    free_vector(class_vector);
                    class_vector = temp;
                }
            }
        }
        if (class_vector) {
            char class_name[50];
            sprintf(class_name, "class_%d", class_idx);
            class_vector->name = strdup(class_name);
            add_tag(class_vector, model->classes[class_idx]);
            class_vectors[class_idx] = class_vector;
        } else {
            fprintf(stderr, "No training vectors for class '%s'\n", model->classes[class_idx]);
            exit(EXIT_FAILURE);
        }
    }
    // Predict test vectors
    for (int i = 0; i < num_test_indices; i++) {
        Vector *test_vector = test_vectors[i];
        char *closest_class = NULL;
        double closest_dist = INFINITY;
        for (int j = 0; j < model->classes_count; j++) {
            Vector *class_vector = class_vectors[j];
            double distance = vector_distance(test_vector, class_vector, "cosine");
            if (distance < closest_dist) {
                closest_dist = distance;
                closest_class = model->classes[j];
            }
        }
        predictions[i] = strdup(closest_class);
    }
    // Free allocated memory
    for (int i = 0; i < model->classes_count; i++) {
        free_vector(class_vectors[i]);
    }
    free(class_vectors);
    free(test_vectors);
    free(training_vectors);
}
