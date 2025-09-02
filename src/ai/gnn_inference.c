#include "gnn_inference.h"

float gnn_predict(const float state_vec[INPUT_SIZE]) {
    if (!state_vec) return 0.0f;
    
    // Placeholder: simple sum for now
    float sum = 0.0f;
    for (int i = 0; i < INPUT_SIZE; i++) {
        sum += state_vec[i];
    }
    
    return sum / INPUT_SIZE;
}
