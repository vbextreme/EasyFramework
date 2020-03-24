/*
 * this is a fork of:
 * 
 * GENANN - Minimal C Artificial Neural Network
 *
 * Copyright (c) 2015-2018 Lewis Van Winkle
 *
 * http://CodePlea.com
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgement in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 *
 */


#ifndef _EF_GNN_H_
#define _EF_GNN_H_

#include <ef/type.h>

#ifndef GNN_RANDOM
/**We use the following for uniform random numbers between 0 and 1.
 * If you have a better function, redefine this macro. */
#define GNN_RANDOM() (((double)rand())/RAND_MAX)
#endif

typedef struct gnn gnn_s;

typedef double (*gnnAct_f)(const gnn_s *ann, double a);

typedef struct gnn {
	/** used to check if already runned before train*/
	int runned;

    /** How many inputs, outputs, and hidden neurons. */
    int inputs, hidden_layers, hidden, outputs;

    /** Which activation function to use for hidden neurons. Default: gennann_act_sigmoid_cached*/
    gnnAct_f activation_hidden;

    /** Which activation function to use for output. Default: gennann_act_sigmoid_cached*/
    gnnAct_f activation_output;

    /** Total number of weights, and size of weights buffer. */
    int total_weights;

    /** Total number of neurons + inputs and size of output buffer. */
    int total_neurons;

    /** All weights (total_weights long). */
    double *weight;

    /** Stores input array and output of each neuron (total_neurons long). */
    double *output;

    /** Stores delta of each hidden and output neuron (total_neurons - inputs long). */
    double *delta;

} gnn_s;

/** Creates and returns a new ann. */
gnn_s *gnn_init(int inputs, int hidden_layers, int hidden, int outputs);

/** Creates ANN from file saved with gnn_write. */
gnn_s *gnn_read(FILE *in);

/** Sets weights randomly. Called by init. */
void gnn_randomize(gnn_s *ann);

/** Returns a new copy of ann. */
gnn_s *gnn_copy(gnn_s const *ann);

/** Frees the memory used by an ann. */
void gnn_free(gnn_s *ann);

/** Runs the feedforward algorithm to calculate the ann's output. */
double const *gnn_run(gnn_s const *ann, double const *inputs);

/** Does a single backprop update, gnn_run before train. */
void gnn_train(gnn_s const *ann, double const *desired_outputs, double learning_rate);

/** gnn_run and gnn_train*/
void gnn_training(gnn_s const *ann, double const *inputs, double const *desired_outputs, double learning_rate);

/** Saves the ann. */
void gnn_write(gnn_s const *ann, FILE *out);

void gnn_init_sigmoid_lookup(const gnn_s *ann);
double gnn_act_sigmoid(const gnn_s *ann, double a);
double gnn_act_sigmoid_cached(const gnn_s *ann, double a);
double gnn_act_threshold(const gnn_s *ann, double a);
double gnn_act_linear(const gnn_s *ann, double a);


#endif /*GENANN_H*/
