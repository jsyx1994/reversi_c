#!/usr/bin/python
# -*- coding: utf-8 -*-
# Imports

import numpy as np
import tensorflow as tf
import time
# Just disables the warning, doesn't enable AVX/FMA

tf.logging.set_verbosity(tf.logging.INFO)

def cnn_model_fn(features, labels, mode):  # labels:[1,65] probability
    """Model function for CNN."""
  # Input Layer
  # [batch_size, image_width, image_height, channels]
  # [-1       , 8          , 8           , 9       ]
    start = time.time();
    input_layer = tf.reshape(features, [-1, 8, 8, 9])

  # Convolutional Layer #1
  # 8-3+1=6
    filters = 32
    conv1 = tf.layers.conv2d(inputs=input_layer,
                             filters=filters,
                             kernel_size=[3, 3], 
                             padding='valid',
                             activation=tf.nn.relu)

  # [-1,6,6,filters*9]

  # Pooling Layer #1

    pool1 = tf.layers.max_pooling2d(inputs=conv1,
                                    pool_size=[2, 2],
                                    strides=2)

  # [-1,3,3,filters*9]
  # enough layer
  # Dense Layer

    pool1_flat = tf.reshape(pool1, [-1, 3 * 3 * filters])
    dense = tf.layers.dense(inputs=pool1_flat,
                            units=1024,
                            activation=tf.nn.relu)

  # Logits Layer

  #0.03
    logits = tf.layers.dense(inputs=dense, units=65)


    pred_prob= tf.nn.softmax(logits[:,:-1], name='softmax_tensor')
    pred_winner = logits[:,-1]



    predictions = {'classes': tf.argmax(input=logits[:,:-1], axis=1),
                   'probabilities': pred_prob,
                   'winner' :pred_winner}  # Generate predictions (for PREDICT and EVAL mode)
                                            # Add `softmax_tensor` to the graph. It is used for PREDICT and by the
                                            # `logging_hook`.
 #0.04
        
    if mode == tf.estimator.ModeKeys.PREDICT:
        predict = tf.estimator.EstimatorSpec(mode=mode,
                predictions=predictions)
        end = time.time()
        print("In model:",end-start)
        return predictions

  # Calculate Loss (for both TRAIN and EVAL modes)
  # should not include the last column, cauz it represent winner,not probability


    desired_prob = labels[:,:-1]
    desired_winner = labels[:,-1]

    loss = -tf.reduce_mean(desired_prob*tf.log(pred_prob))  + tf.losses.mean_squared_error(desired_winner , pred_winner)
  # Configure the Training Op (for TRAIN mode)

    if mode == tf.estimator.ModeKeys.TRAIN:
        optimizer = tf.train.AdamOptimizer(learning_rate=0.001)
        train_op = optimizer.minimize(loss=loss,
                global_step=tf.train.get_global_step())
        return tf.estimator.EstimatorSpec(mode=mode, loss=loss,
                train_op=train_op)


  # Add evaluation metrics (for EVAL mode)

def train_input_fn(features, labels, batch_size):
    data = tf.data.Dataset.from_tensor_slices((features, labels))
    data = data.repeat().batch(batch_size)
    return data.make_one_shot_iterator().get_next()

def predict_input_fn(features):
    start = time.time()

    data = tf.data.Dataset.from_tensor_slices(features)
    data = data.repeat(1).batch(1)

    end = time.time()
    print("input_function use time(s):",end-start)
    return data.make_one_shot_iterator().get_next()

def predict_class(position):
    reversi_classifier = tf.estimator.Estimator(model_fn=cnn_model_fn,
            model_dir='/tmp/reversi_convnet_model')
    position = tf.reshape(tensor = position, shape = (-1,64,9))
    
    sess =  tf.InteractiveSession()

    #tf.saved_model.loader.load(sess,tags = ["tag"] ,export_dir = '/tmp/reversi_convnet_model')
    z = cnn_model_fn(position,None,tf.estimator.ModeKeys.PREDICT)


    sess.run(tf.global_variables_initializer())

    start = time.time()
    result = sess.run(z)
    end = time.time()
    print("first predict use time(s):",end - start)
    print(result)
    '''
    x = reversi_classifier.predict(input_fn= lambda : predict_input_fn(position))


    start = time.time()
    y = next(x)
    end = time.time()
    print("first predict use time(s):",end - start)
    start = time.time()
    next(x)
    end = time.time()
    print("seconde predict use time(s):",end-start)

    y["probabilities"]= list(y["probabilities"])
    '''

    return z

def main(batch_size,winner):# main function for training

  # Load training data

    train_data = np.genfromtxt('/Users/mro/Desktop/cnnInput',
                               delimiter=' ', dtype=np.float32)
    train_data = train_data.reshape(-1, 64, 9)
    train_labels = np.genfromtxt('/Users/mro/Desktop/cnnOut',
                                 dtype=np.float32)
    #add winner to end of the ndarray
    print("Winner is ",winner)
    if winner == 1:
        train_labels = np.column_stack((train_labels,np.ones(train_labels.shape[0],dtype=np.float32)))
    elif winner == 0:
        train_labels = np.column_stack((train_labels,np.zeros(train_labels.shape[0],dtype=np.float32)))

  # and eval data
  # eval_data = mnist.test.images  # Returns np.array
  # eval_labels = np.asarray(mnist.test.labels, dtype=np.int32)

  # Create the Estimator
    reversi_classifier = tf.estimator.Estimator(model_fn=cnn_model_fn,
            model_dir='/tmp/reversi_convnet_model')
   # reversi_classifier = tf.estimator.Estimator(model_fn=cnn_model_fn, model_dir='/tmp/reversi_convnet_model')

  # Set up logging for predictions
  # Log the values in the "Softmax" tensor with label "probabilities"
    tensors_to_log = {'probabilities': 'softmax_tensor'}
    logging_hook = tf.train.LoggingTensorHook(tensors=tensors_to_log,
            every_n_iter=50)

    saver = tf.train.Saver()
    check_point_path = saver.last_checkpoint('/tmp/reversi_convnet_model')
    print(check_point_path)
    #predict 
  # Train the model
    predict_input = [0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0]
    #predict_class(predict_input)
    reversi_classifier.train(input_fn=lambda : train_input_fn(train_data, train_labels,batch_size), steps=2000, hooks=[logging_hook])
#main(64,1)

