/**
 * @file features.h
 * @brief Time-domain feature extraction for 1-second vibration + temperature window
 */

#ifndef FEATURES_H
#define FEATURES_H

#include "kanshi_config.h"
#include "circular_buffer.h"

/**
 * Extract 12 features from a window of samples.
 * Output is normalised to approximately [-1, +1] for the model.
 */
void extract_features(const sensor_sample_t *win, UW n, float features[NUM_FEATURES]);

#endif /* FEATURES_H */
