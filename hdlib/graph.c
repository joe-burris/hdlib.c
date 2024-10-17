/* Implementation of the Graph structure in C */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>
#include <time.h>

/* Assuming the Vector and Space structures and functions are defined as in the previous code */
/* Include the definitions of Vector and Space here or in a separate header file */

/* Define the Edge structure */
typedef struct Edge {
    char *node1_name;
    char *node2_name;
    double weight; // Use -1 for unweighted graphs
} Edge;

/* Define the Graph structure */
typedef struct Graph {
    int size;
    char *vtype; // "binary" or "bipolar"
    bool directed;
    bool weighted;
    int nodes_counter;
    int edges_counter;
    Space *space;
    int seed;
    // Random number generator seed
    // You can add more fields as needed
} Graph;

/* Function prototypes */
Graph *create_graph(int size, bool directed, bool weighted, int seed);
void free_graph(Graph *graph);
void add_edge(Graph *graph, const char *node1_name, const char *node2_name, double weight);
void build_node_memory(Graph *graph, const char *node_name);
void build_weight_memory(Graph *graph, double start, double end, double step);
void fit_graph(Graph *graph, Edge **edges, int edge_count);
bool edge_exists(Graph *graph, const char *node1_name, const char *node2_name, double weight, double threshold, double *distance);
double error_rate(Graph *graph, Edge **edges, int edge_count, double threshold, Edge ***false_positives, Edge ***false_negatives, int *fp_count, int *fn_count);
void error_mitigation(Graph *graph, Edge **edges, int edge_count, double threshold, int max_iter, double prev_error_rate);

/* Additional helper functions */
Vector *get_vector_from_space(Space *space, const char *name);

/* Function implementations */

/* Create a new Graph */
Graph *create_graph(int size, bool directed, bool weighted, int seed) {
    if (size < 10000) {
        fprintf(stderr, "Vectors size must be greater than or equal to 10000\n");
        exit(EXIT_FAILURE);
    }
    Graph *graph = (Graph *)malloc(sizeof(Graph));
    if (!graph) {
        perror("Failed to allocate memory for Graph");
        exit(EXIT_FAILURE);
    }
    graph->size = size;
    graph->vtype = strdup("bipolar"); // Default type
    graph->directed = directed;
    graph->weighted = weighted;
    graph->nodes_counter = 0;
    graph->edges_counter = 0;
    graph->seed = seed;
    graph->space = create_space(size, graph->vtype);
    if (seed != -1) {
        srand(seed);
    } else {
        srand(time(NULL));
    }
    return graph;
}

/* Free a Graph */
void free_graph(Graph *graph) {
    if (graph) {
        free_space(graph->space);
        free(graph->vtype);
        free(graph);
    }
}

/* Get a Vector from the Space by name */
Vector *get_vector_from_space(Space *space, const char *name) {
    for (int i = 0; i < space->vector_count; i++) {
        if (strcmp(space->vectors[i]->name, name) == 0) {
            return space->vectors[i];
        }
    }
    return NULL;
}

/* Add an edge to the graph */
void add_edge(Graph *graph, const char *node1_name, const char *node2_name, double weight) {
    // Check if nodes exist; if not, create them
    Vector *node1 = get_vector_from_space(graph->space, node1_name);
    if (!node1) {
        node1 = create_vector(node1_name, graph->size, graph->vtype, -1, false);
        insert_vector(graph->space, node1);
        graph->nodes_counter++;
    }
    Vector *node2 = get_vector_from_space(graph->space, node2_name);
    if (!node2) {
        node2 = create_vector(node2_name, graph->size, graph->vtype, -1, false);
        insert_vector(graph->space, node2);
        graph->nodes_counter++;
    }
    // Implement edge storage in the graph
    // For simplicity, we can store edges in the vectors' tags or create a separate edge list
    // Here, we'll assume each Vector has a list of neighbors (children)
    // Add node2 to node1's children
    // You need to implement this functionality in your Vector structure
    // For example:
    add_child(node1, node2_name);
    if (graph->weighted) {
        // Store the weight
        add_weight(node1, node2_name, weight);
    }
    graph->edges_counter++;
    if (!graph->directed) {
        // Add node1 to node2's children
        add_child(node2, node1_name);
        if (graph->weighted) {
            add_weight(node2, node1_name, weight);
        }
        graph->edges_counter++;
    }
}

/* Build node memory */
void build_node_memory(Graph *graph, const char *node_name) {
    // Retrieve the node
    Vector *node = get_vector_from_space(graph->space, node_name);
    if (!node) {
        fprintf(stderr, "Node '%s' is not in the graph space\n", node_name);
        exit(EXIT_FAILURE);
    }
    // Initialize node memory
    Vector *node_memory = create_vector("NodeMemory", graph->size, graph->vtype, -1, false);
    // Assuming node has a list of children and weights
    for (int i = 0; i < node->children_count; i++) {
        const char *neighbor_name = node->children[i];
        Vector *neighbor = get_vector_from_space(graph->space, neighbor_name);
        Vector *temp_vector = NULL;
        if (graph->weighted) {
            double weight = get_weight(node, neighbor_name);
            char weight_vector_name[50];
            sprintf(weight_vector_name, "__weight__%.2f", weight);
            Vector *weight_vector = get_vector_from_space(graph->space, weight_vector_name);
            if (!weight_vector) {
                // Build weight vector if it doesn't exist
                // You need to implement build_weight_memory function
                fprintf(stderr, "Weight vector '%s' not found\n", weight_vector_name);
                exit(EXIT_FAILURE);
            }
            temp_vector = bind_vectors(weight_vector, neighbor);
        } else {
            temp_vector = neighbor;
        }
        // Bundle temp_vector into node_memory
        for (int j = 0; j < graph->size; j++) {
            node_memory->vector[j] += temp_vector->vector[j];
        }
    }
    // Store node_memory in the node (you may need to add a field to Vector)
    node->memory = node_memory;
}

/* Build weight memory */
void build_weight_memory(Graph *graph, double start, double end, double step) {
    int levels = (int)((end - start) / step);
    int change = graph->size / 2;
    int next_level = graph->size / (2 * levels);

    int *base_vector = (int *)malloc(graph->size * sizeof(int));
    for (int i = 0; i < graph->size; i++) {
        base_vector[i] = -1; // Initialize to -1 for bipolar
    }
    for (double w = start; w < end; w += step) {
        char weight_vector_name[50];
        sprintf(weight_vector_name, "__weight__%.2f", w);
        Vector *weight_vector = create_vector(weight_vector_name, graph->size, graph->vtype, -1, false);
        // Flip bits to create different weight vectors
        for (int i = 0; i < next_level; i++) {
            int index = rand() % graph->size;
            base_vector[index] *= -1;
        }
        memcpy(weight_vector->vector, base_vector, graph->size * sizeof(int));
        insert_vector(graph->space, weight_vector);
    }
    free(base_vector);
}

/* Fit the graph */
void fit_graph(Graph *graph, Edge **edges, int edge_count) {
    if (edge_count == 0) {
        fprintf(stderr, "Must provide at least one edge\n");
        exit(EXIT_FAILURE);
    }
    // Add edges to the graph
    for (int i = 0; i < edge_count; i++) {
        Edge *edge = edges[i];
        if (graph->weighted && edge->weight == -1) {
            fprintf(stderr, "Graph is weighted but edge weight is missing\n");
            exit(EXIT_FAILURE);
        }
        if (!graph->weighted && edge->weight != -1) {
            fprintf(stderr, "Graph is unweighted but edge weight is specified\n");
            exit(EXIT_FAILURE);
        }
        add_edge(graph, edge->node1_name, edge->node2_name, edge->weight);
    }
    // Build weight memory if weighted
    if (graph->weighted) {
        build_weight_memory(graph, 0.0, 1.0, 0.01);
    }
    // Build node memories
    for (int i = 0; i < graph->space->vector_count; i++) {
        Vector *node = graph->space->vectors[i];
        if (strcmp(node->name, "__graph__") != 0 && strncmp(node->name, "__weight__", 9) != 0) {
            build_node_memory(graph, node->name);
        }
    }
    // Build the graph vector
    Vector *graph_vector = create_vector("__graph__", graph->size, graph->vtype, -1, false);
    for (int i = 0; i < graph->space->vector_count; i++) {
        Vector *node = graph->space->vectors[i];
        if (strcmp(node->name, "__graph__") != 0 && strncmp(node->name, "__weight__", 9) != 0) {
            Vector *temp_vector = bind_vectors(node, node->memory);
            // Bundle into graph_vector
            for (int j = 0; j < graph->size; j++) {
                graph_vector->vector[j] += temp_vector->vector[j];
            }
            free_vector(temp_vector);
        }
    }
    // Normalize if undirected
    if (!graph->directed) {
        for (int i = 0; i < graph->size; i++) {
            graph_vector->vector[i] /= 2;
        }
    }
    // Insert graph vector into space
    insert_vector(graph->space, graph_vector);
}

/* Check if an edge exists */
bool edge_exists(Graph *graph, const char *node1_name, const char *node2_name, double weight, double threshold, double *distance) {
    Vector *graph_vector = get_vector_from_space(graph->space, "__graph__");
    if (!graph_vector) {
        fprintf(stderr, "There is no graph in the space\n");
        exit(EXIT_FAILURE);
    }
    Vector *node1 = get_vector_from_space(graph->space, node1_name);
    Vector *node2 = get_vector_from_space(graph->space, node2_name);
    if (!node1 || !node2) {
        fprintf(stderr, "Nodes '%s' or '%s' are not in the space\n", node1_name, node2_name);
        exit(EXIT_FAILURE);
    }
    // Retrieve node1 memory from graph
    Vector *node1_memory = bind_vectors(node1, graph_vector);
    if (graph->directed) {
        // Permute back if directed
        permute_vector(node1_memory, -1);
    }
    Vector *temp_vector = NULL;
    if (graph->weighted) {
        char weight_vector_name[50];
        sprintf(weight_vector_name, "__weight__%.2f", weight);
        Vector *weight_vector = get_vector_from_space(graph->space, weight_vector_name);
        if (!weight_vector) {
            fprintf(stderr, "Weight vector '%s' not found\n", weight_vector_name);
            exit(EXIT_FAILURE);
        }
        temp_vector = bind_vectors(weight_vector, node2);
    } else {
        temp_vector = node2;
    }
    // Compute distance
    *distance = vector_distance(node1_memory, temp_vector, "cosine");
    free_vector(node1_memory);
    if (graph->weighted) {
        free_vector(temp_vector);
    }
    return (*distance < threshold);
}

/* Compute error rate */
double error_rate(Graph *graph, Edge **edges, int edge_count, double threshold, Edge ***false_positives, Edge ***false_negatives, int *fp_count, int *fn_count) {
    *fp_count = 0;
    *fn_count = 0;
    int fp_capacity = 10;
    int fn_capacity = 10;
    *false_positives = (Edge **)malloc(fp_capacity * sizeof(Edge *));
    *false_negatives = (Edge **)malloc(fn_capacity * sizeof(Edge *));
    for (int i = 0; i < edge_count; i++) {
        Edge *edge = edges[i];
        double distance = 0.0;
        bool exists = edge_exists(graph, edge->node1_name, edge->node2_name, edge->weight, threshold, &distance);
        bool actual_exists = false;
        // Determine if the edge actually exists in the graph's stored edges
        Vector *node1 = get_vector_from_space(graph->space, edge->node1_name);
        if (node1) {
            actual_exists = has_child(node1, edge->node2_name);
        }
        if (exists && !actual_exists) {
            // False positive
            if (*fp_count >= fp_capacity) {
                fp_capacity *= 2;
                *false_positives = (Edge **)realloc(*false_positives, fp_capacity * sizeof(Edge *));
            }
            (*false_positives)[(*fp_count)++] = edge;
        } else if (!exists && actual_exists) {
            // False negative
            if (*fn_count >= fn_capacity) {
                fn_capacity *= 2;
                *false_negatives = (Edge **)realloc(*false_negatives, fn_capacity * sizeof(Edge *));
            }
            (*false_negatives)[(*fn_count)++] = edge;
        }
    }
    return (double)(*fp_count + *fn_count) / edge_count;
}

/* Error mitigation (simplified) */
void error_mitigation(Graph *graph, Edge **edges, int edge_count, double threshold, int max_iter, double prev_error_rate) {
    if (max_iter <= 0) {
        return;
    }
    Edge **false_positives = NULL;
    Edge **false_negatives = NULL;
    int fp_count = 0;
    int fn_count = 0;
    double current_error_rate = error_rate(graph, edges, edge_count, threshold, &false_positives, &false_negatives, &fp_count, &fn_count);
    if (prev_error_rate < 0 || current_error_rate < prev_error_rate) {
        // Adjust node memories based on false positives and negatives
        for (int i = 0; i < fp_count; i++) {
            Edge *edge = false_positives[i];
            // Reduce the signal of node2 in node1's memory
            // Implement this logic as per your data structures
        }
        for (int i = 0; i < fn_count; i++) {
            Edge *edge = false_negatives[i];
            // Increase the signal of node2 in node1's memory
            // Implement this logic as per your data structures
        }
        // Rebuild the graph vector
        fit_graph(graph, edges, edge_count);
        // Recurse
        error_mitigation(graph, edges, edge_count, threshold, max_iter - 1, current_error_rate);
    }
    free(false_positives);
    free(false_negatives);
}

/* Main function to demonstrate usage */
int main() {
    /* Create a graph */
    Graph *graph = create_graph(10000, false, false, -1);
    
    /* Define edges */
    int edge_count = 3;
    Edge **edges = (Edge **)malloc(edge_count * sizeof(Edge *));
    edges[0] = (Edge *)malloc(sizeof(Edge));
    edges[0]->node1_name = strdup("Node1");
    edges[0]->node2_name = strdup("Node2");
    edges[0]->weight = -1; // For unweighted graphs
    
    edges[1] = (Edge *)malloc(sizeof(Edge));
    edges[1]->node1_name = strdup("Node2");
    edges[1]->node2_name = strdup("Node3");
    edges[1]->weight = -1;
    
    edges[2] = (Edge *)malloc(sizeof(Edge));
    edges[2]->node1_name = strdup("Node3");
    edges[2]->node2_name = strdup("Node1");
    edges[2]->weight = -1;
    
    /* Fit the graph */
    fit_graph(graph, edges, edge_count);
    
    /* Check if an edge exists */
    double distance;
    bool exists = edge_exists(graph, "Node1", "Node2", -1, 0.7, &distance);
    printf("Edge between Node1 and Node2 exists: %s, distance: %f\n", exists ? "Yes" : "No", distance);
    
    /* Compute error rate */
    Edge **false_positives = NULL;
    Edge **false_negatives = NULL;
    int fp_count = 0;
    int fn_count = 0;
    double error = error_rate(graph, edges, edge_count, 0.7, &false_positives, &false_negatives, &fp_count, &fn_count);
    printf("Error rate: %f\n", error);
    
    /* Clean up */
    free_graph(graph);
    for (int i = 0; i < edge_count; i++) {
        free(edges[i]->node1_name);
        free(edges[i]->node2_name);
        free(edges[i]);
    }
    free(edges);
    
    return 0;
}
