/* Implementation of utility functions in C */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>

/* Function prototypes */
void load_dataset(const char *filepath, const char *sep, char ***samples, char ***features, double ***content, char ***classes, int *num_samples, int *num_features);
int *percentage_split(char **labels, int num_labels, double percentage, int seed, int *num_selected_indices);

/* Helper functions */
void handle_file_not_found(const char *filepath);
void free_dataset(char **samples, char **features, double **content, char **classes, int num_samples, int num_features);

int main() {
    /* Example usage of load_dataset */
    char **samples = NULL;
    char **features = NULL;
    double **content = NULL;
    char **classes = NULL;
    int num_samples = 0;
    int num_features = 0;

    const char *filepath = "dataset.txt";
    const char *sep = "\t";

    load_dataset(filepath, sep, &samples, &features, &content, &classes, &num_samples, &num_features);

    /* Print loaded data */
    printf("Samples:\n");
    for (int i = 0; i < num_samples; i++) {
        printf("%s\n", samples[i]);
    }

    printf("Features:\n");
    for (int i = 0; i < num_features; i++) {
        printf("%s\t", features[i]);
    }
    printf("\n");

    printf("Content:\n");
    for (int i = 0; i < num_samples; i++) {
        for (int j = 0; j < num_features; j++) {
            printf("%.2f\t", content[i][j]);
        }
        printf("\n");
    }

    printf("Classes:\n");
    for (int i = 0; i < num_samples; i++) {
        printf("%s\n", classes[i]);
    }

    /* Example usage of percentage_split */
    int num_selected_indices = 0;
    int *selected_indices = percentage_split(classes, num_samples, 20.0, 0, &num_selected_indices);

    printf("Selected indices:\n");
    for (int i = 0; i < num_selected_indices; i++) {
        printf("%d\n", selected_indices[i]);
    }

    /* Free allocated memory */
    free_dataset(samples, features, content, classes, num_samples, num_features);
    free(selected_indices);

    return 0;
}

/* Function implementations */

/* Load the input numerical dataset */
void load_dataset(const char *filepath, const char *sep, char ***samples, char ***features, double ***content, char ***classes, int *num_samples, int *num_features) {
    FILE *file = fopen(filepath, "r");
    if (!file) {
        handle_file_not_found(filepath);
        exit(EXIT_FAILURE);
    }

    char line[1024];
    char *token;
    int line_num = 0;
    *num_samples = 0;
    *num_features = 0;
    int content_capacity = 10;
    int features_capacity = 10;

    /* Initialize arrays */
    *samples = NULL;
    *features = NULL;
    *content = NULL;
    *classes = NULL;

    /* Read the first line (features) */
    if (fgets(line, sizeof(line), file)) {
        line[strcspn(line, "\r\n")] = '\0'; // Remove newline characters
        char *line_copy = strdup(line);
        char *rest = line_copy;

        /* Skip the first column (Sample ID) */
        token = strtok_r(rest, sep, &rest);

        /* Read feature names */
        *features = (char **)malloc(features_capacity * sizeof(char *));
        int feature_idx = 0;
        while ((token = strtok_r(NULL, sep, &rest))) {
            if (strcmp(token, "") == 0) {
                continue;
            }
            if (strcmp(token, "#") == 0) {
                break;
            }
            if (feature_idx >= features_capacity) {
                features_capacity *= 2;
                *features = (char **)realloc(*features, features_capacity * sizeof(char *));
            }
            (*features)[feature_idx++] = strdup(token);
        }
        *num_features = feature_idx;
        free(line_copy);
    } else {
        fprintf(stderr, "Empty file or unable to read the file: %s\n", filepath);
        fclose(file);
        exit(EXIT_FAILURE);
    }

    /* Read the rest of the file */
    while (fgets(line, sizeof(line), file)) {
        line_num++;
        line[strcspn(line, "\r\n")] = '\0'; // Remove newline characters
        if (line[0] == '#' || strlen(line) == 0) {
            continue;
        }
        char *line_copy = strdup(line);
        char *rest = line_copy;

        /* Allocate memory for samples and classes */
        *samples = (char **)realloc(*samples, (*num_samples + 1) * sizeof(char *));
        *classes = (char **)realloc(*classes, (*num_samples + 1) * sizeof(char *));

        /* Read Sample ID */
        token = strtok_r(rest, sep, &rest);
        (*samples)[*num_samples] = strdup(token);

        /* Allocate memory for content row */
        if (*content == NULL) {
            *content = (double **)malloc(content_capacity * sizeof(double *));
        }
        if (*num_samples >= content_capacity) {
            content_capacity *= 2;
            *content = (double **)realloc(*content, content_capacity * sizeof(double *));
        }
        (*content)[*num_samples] = (double *)malloc(*num_features * sizeof(double));

        /* Read numerical data */
        int feature_idx = 0;
        while ((token = strtok_r(NULL, sep, &rest))) {
            if (feature_idx < *num_features) {
                char *endptr;
                double value = strtod(token, &endptr);
                if (*endptr != '\0') {
                    fprintf(stderr, "The input dataset must contain numbers only! Error at line %d\n", line_num);
                    fclose(file);
                    exit(EXIT_FAILURE);
                }
                (*content)[*num_samples][feature_idx++] = value;
            } else {
                /* Read Class label */
                (*classes)[*num_samples] = strdup(token);
                break;
            }
        }
        if (feature_idx != *num_features) {
            fprintf(stderr, "Mismatch in the number of features at line %d\n", line_num);
            fclose(file);
            exit(EXIT_FAILURE);
        }
        (*num_samples)++;
        free(line_copy);
    }
    fclose(file);
}

/* Handle file not found error */
void handle_file_not_found(const char *filepath) {
    fprintf(stderr, "File not found: %s\n", filepath);
}

/* Free the allocated memory for the dataset */
void free_dataset(char **samples, char **features, double **content, char **classes, int num_samples, int num_features) {
    for (int i = 0; i < num_samples; i++) {
        free(samples[i]);
        free(content[i]);
        free(classes[i]);
    }
    free(samples);
    free(classes);
    free(content);
    for (int i = 0; i < num_features; i++) {
        free(features[i]);
    }
    free(features);
}

/* Given list of classes and a percentage, split the dataset and return indices of selected data points */
int *percentage_split(char **labels, int num_labels, double percentage, int seed, int *num_selected_indices) {
    if (percentage <= 0.0 || percentage > 100.0) {
        fprintf(stderr, "Percentage must be greater than 0 and lower than or equal to 100\n");
        exit(EXIT_FAILURE);
    }

    if (seed < 0) {
        fprintf(stderr, "The input seed must be a non-negative integer number\n");
        exit(EXIT_FAILURE);
    }

    /* Find unique labels */
    char **unique_labels = (char **)malloc(num_labels * sizeof(char *));
    int unique_count = 0;
    for (int i = 0; i < num_labels; i++) {
        bool exists = false;
        for (int j = 0; j < unique_count; j++) {
            if (strcmp(labels[i], unique_labels[j]) == 0) {
                exists = true;
                break;
            }
        }
        if (!exists) {
            unique_labels[unique_count++] = labels[i];
        }
    }

    if (unique_count < 2) {
        fprintf(stderr, "The list of class labels must contain at least two unique labels\n");
        free(unique_labels);
        exit(EXIT_FAILURE);
    }

    /* Initialize random number generator */
    srand(seed);

    /* Selection array */
    int *selection = NULL;
    int selection_capacity = 10;
    int selection_count = 0;
    selection = (int *)malloc(selection_capacity * sizeof(int));

    for (int i = 0; i < unique_count; i++) {
        char *label = unique_labels[i];

        /* Count occurrences of the label */
        int label_count = 0;
        for (int j = 0; j < num_labels; j++) {
            if (strcmp(labels[j], label) == 0) {
                label_count++;
            }
        }

        /* Calculate number of points to select */
        int select_points = (int)(percentage * label_count / 100.0);

        /* Collect indices of samples with the current label */
        int *indices = (int *)malloc(label_count * sizeof(int));
        int idx = 0;
        for (int j = 0; j < num_labels; j++) {
            if (strcmp(labels[j], label) == 0) {
                indices[idx++] = j;
            }
        }

        /* Randomly select indices */
        for (int k = 0; k < select_points; k++) {
            int rand_idx = rand() % label_count;
            int selected_idx = indices[rand_idx];

            /* Check if already selected */
            bool already_selected = false;
            for (int m = 0; m < selection_count; m++) {
                if (selection[m] == selected_idx) {
                    already_selected = true;
                    break;
                }
            }
            if (!already_selected) {
                if (selection_count >= selection_capacity) {
                    selection_capacity *= 2;
                    selection = (int *)realloc(selection, selection_capacity * sizeof(int));
                }
                selection[selection_count++] = selected_idx;
            }
        }
        free(indices);
    }
    free(unique_labels);

    /* Sort the selection */
    for (int i = 0; i < selection_count - 1; i++) {
        for (int j = i + 1; j < selection_count; j++) {
            if (selection[i] > selection[j]) {
                int temp = selection[i];
                selection[i] = selection[j];
                selection[j] = temp;
            }
        }
    }

    *num_selected_indices = selection_count;
    return selection;
}
