import tensorflow as tf
import numpy as np
import time
import os
'''
Convlutional  [3,3,9,32]
    |
    v
  pooling     maxpooling
    |
    v
fully connected  1024
    |
    v
  logits          65
    |
    v
  softmax         [:-1]
'''
CONV_FILTERS = 32

FULLY_CONNECT_LAYER_N = 1024

LEARNING_RATE = 1e-3

export_dir = '/tmp/low_level_reversi/model.ckpt'

sess = tf.InteractiveSession() # faster than session when running forward propagation



def weight_variable(shape):
    initial = tf.truncated_normal(shape, stddev = 0.1,dtype = tf.float32)
    return tf.Variable(initial)

def bias_variable(shape):
    initial = tf.constant(0.1, shape = shape,dtype  = tf.float32)
    return tf.Variable(initial)

def conv2d(input,filter):
    '''
    tf.nn.conv2d(
    input,
    filter,
    strides,
    padding,
    use_cudnn_on_gpu=True,
    data_format='NHWC',
    dilations=[1, 1, 1, 1],
    name=None
    )

    Given an input tensor of shape 
    [batch, in_height, in_width, in_channels] 

    and a filter / kernel tensor of shape 
    [filter_height, filter_width, in_channels, out_channels],

    this op performs the following:
    Flattens the filter to a 2-D matrix with shape 
    [filter_height * filter_width * in_channels, output_channels].

    Extracts image patches from the input tensor to form a virtual tensor of shape 
    [batch, out_height, out_width, filter_height * filter_width * in_channels].
    For each patch, right-multiplies the filter matrix and the image patch vector.

    Args:
input: A Tensor. Must be one of the following types: half, bfloat16, float32, float64. A 4-D tensor. The dimension order is interpreted according to the value of data_format, see below for details.
filter: A Tensor. Must have the same type as input. A 4-D tensor of shape [filter_height, filter_width, in_channels, out_channels]
strides: A list of ints. 1-D tensor of length 4. The stride of the sliding window for each dimension of input. The dimension order is determined by the value of data_format, see below for details.
padding: A string from: "SAME", "VALID". The type of padding algorithm to use.
use_cudnn_on_gpu: An optional bool. Defaults to True.
data_format: An optional string from: "NHWC", "NCHW". Defaults to "NHWC". Specify the data format of the input and output data. With the default format "NHWC", the data is stored in the order of: [batch, height, width, channels]. Alternatively, the format could be "NCHW", the data storage order of: [batch, channels, height, width].
dilations: An optional list of ints. Defaults to [1, 1, 1, 1]. 1-D tensor of length 4. The dilation factor for each dimension of input. If set to k > 1, there will be k-1 skipped cells between each filter element on that dimension. The dimension order is determined by the value of data_format, see above for details. Dilations in the batch and depth dimensions must be 1.
name: A name for the operation (optional).
    '''
    return tf.nn.conv2d(input = input,filter = filter,strides = [1,1,1,1],padding = "VALID")

def max_pooling_2x2(value):
    '''
    tf.nn.max_pool(
    value,
    ksize,
    strides,
    padding,
    data_format='NHWC',
    name=None
)
Defined in tensorflow/python/ops/nn_ops.py.

See the guide: Neural Network > Pooling

Performs the max pooling on the input.

Args:
value: A 4-D Tensor of the format specified by data_format.
ksize: A 1-D int Tensor of 4 elements. The size of the window for each dimension of the input tensor.
strides: A 1-D int Tensor of 4 elements. The stride of the sliding window for each dimension of the input tensor.
padding: A string, either 'VALID' or 'SAME'. The padding algorithm. See the comment here
data_format: A string. 'NHWC', 'NCHW' and 'NCHW_VECT_C' are supported.
name: Optional name for the operation.
Returns:
A Tensor of format specified by data_format. The max pooled output tensor.
    '''
    return tf.nn.max_pool(value = value,ksize = [1,2,2,1], strides = [1,2,2,1],padding = "VALID")

def conv_layer(input,filter_height, filter_width, channels_in , channels_out,name_scope):
    '''
    return the 2-D tensor
    '''
    #input = tf.reshape(input, [-1,8,8,9])
    with tf.name_scope(name_scope):
        with tf.name_scope("W"):
            w = weight_variable([filter_height, filter_width, channels_in, channels_out])
        with tf.name_scope("B"):
            b = bias_variable([channels_out])
        tf.summary.histogram("weights",w)
        tf.summary.histogram("bias",b)
        conv = conv2d(input = input, filter= w)
        act = tf.nn.relu(conv + b)
        return act

def fully_connect_layer(input, channels_in, channels_out):
    w = weight_variable([channels_in,channels_out])
    b = bias_variable([channels_out])
    value = tf.matmul(input,w) + b
    act = tf.nn.relu(value)
    return act

def logits_layer(input,channels_in,channels_out):
    w = weight_variable([channels_in,channels_out])
    b = bias_variable([channels_out])
    value = tf.matmul(input,w) + b
    return value

def reversi_CNN_model(features,labels,mode):
    features = tf.reshape(features, [-1,8,8,9])
    # [batch_size, image_width, image_height, channels]
    # [batch_size        , 8          , 8           , 9       ]
    
    conv = conv_layer(input = features, filter_height = 3, filter_width = 3, channels_in = 9, channels_out = CONV_FILTERS,name_scope = "conv")
    #[batch_size, 6, 6, 32]
    with tf.name_scope("pool"):
        pool = max_pooling_2x2(conv)
    #[batch_size, 3, 3, 32]
    with tf.name_scope("dense"):
        flattend = tf.reshape(pool,[-1,3 * 3 * CONV_FILTERS])
    #[batch_size, 288]
    with tf. name_scope("fully_connected"):
        fc = fully_connect_layer(flattend, 3 * 3* CONV_FILTERS, FULLY_CONNECT_LAYER_N)
    #[batch_size , 1024]
    with tf.name_scope("logits"):
        logits = logits_layer(fc,FULLY_CONNECT_LAYER_N,65)
    #[batch_size, 65]
    
    pred_prob = logits[:,:-1]
    pred_winner = logits[:,-1]
    #pred_winner =tf.nn.relu(pred_winner)                           ####!!!!!!!!!!!!!!#####
    
    if mode == "PREDICT":
        with tf.name_scope("predict_part"):
            return model_predict(prob = pred_prob,winner = pred_winner)

    if mode == "TRAIN":
        with tf.name_scope("train_part"):
            return model_train(pred_prob = pred_prob, pred_winner = pred_winner,labels = labels) #return training op


def model_predict(prob,winner):
    prediction = {
        "probability" : tf.nn.softmax(prob),
        "winner" : winner
    }
    return prediction


def model_train(pred_prob,pred_winner,labels):
    desired_prob = labels[:,:-1]
    desired_winner = labels[:,-1]
    pred_prob = tf.nn.softmax(pred_prob)
    loss = -tf.reduce_mean(desired_prob*tf.log(pred_prob))  + tf.losses.mean_squared_error(desired_winner , pred_winner)
    #loss.eval()
    tf.summary.scalar("loss",loss)
    train_step = tf.train.AdamOptimizer(LEARNING_RATE).minimize(loss)
    return train_step,loss


def train_input_fn(features, labels, batch_size):
    data = tf.data.Dataset.from_tensor_slices((features, labels))
    data = data.repeat().batch(batch_size)
    return data.make_one_shot_iterator().get_next()

def init_model_predict():
    x = tf.placeholder(tf.float32,[8*8,9],name = 'x')

    z = reversi_CNN_model(x ,None ,"PREDICT")
    sess.run(tf.global_variables_initializer())
    saver = tf.train.Saver()
    saver.restore(sess,export_dir)


    #train_data = np.genfromtxt('/Users/mro/Desktop/cnnInput',delimiter = ' ',dtype=np.float32)
    #train_data = train_data.reshape(-1,64,9);
    #predict_test(z,{x:train_data[0]})
    return x, z

def predict_test(z,xin):
    '''
    train_data = np.genfromtxt('/Users/mro/Desktop/cnnInput',delimiter = ' ',dtype=np.float32)
    train_data = train_data.reshape(-1,64,9);

    x , z = init_model_predict()
    start = time.time()
    #print(type(sess.run(z, {x : train_data[1]})))
    #print(z,{x : train_data[1]})
    #print(end - start)
    print(type({x:train_data[0]}))
    '''
    #print(z)
    #print(xin)

    x = tf.placeholder(tf.float32,[8*8,9],name = 'x')

    try:
        return sess.run(z,xin)
    except Exception as e:
        print(e)
    #sess.run(z, xin)



def train_test(winner,batch_size,steps):
    x = tf.placeholder(tf.float32,[None,8*8,9],name = "x")
    y_ = tf.placeholder(tf.float32,[None, 65], name = "labels")

    train_data = np.genfromtxt('/Users/mro/Desktop/cnnInput',delimiter = ' ',dtype=np.float32)
    train_data = train_data.reshape(-1,64,9);
    train_labels = np.genfromtxt('/Users/mro/Desktop/cnnOut',
                                 dtype=np.float32)
    #add winner to end of the ndarray
    print("Winner is ",winner)
    if winner == 1:
        train_labels = np.column_stack((train_labels,np.ones(train_labels.shape[0],dtype=np.float32)))
    elif winner == 0:
        train_labels = np.column_stack((train_labels,np.zeros(train_labels.shape[0],dtype=np.float32)))

    input_data_iter = train_input_fn(train_data,train_labels,batch_size)
    
    train_op , loss = reversi_CNN_model(x, y_, "TRAIN")

    merged_summary =tf.summary.merge_all()


    file_writer = tf.summary.FileWriter('/tmp/low_level_reversi/log',sess.graph)

    #total_steps = tf.Variable(0,dtype = tf.float32,name = "total_steps")
    
    sess.run(tf.global_variables_initializer())

    steps_log = 0

    saver = tf.train.Saver()
    try:
        saver.restore(sess,export_dir)
        steps_log_file = open('/tmp/low_level_reversi/log/steps_log','r')
        steps_log = steps_log_file.read()
        steps_log_file.close()

        steps_log = int(steps_log)
        steps_log = steps_log + steps

        steps_log_file = open('/tmp/low_level_reversi/log/steps_log','w')
        steps_log_file.write(str(steps_log))
        steps_log_file.close()

    except Exception as e:
        steps_log_file = open('/tmp/low_level_reversi/log/steps_log','w')
        steps_log_file.write('0')
        steps_log_file.close() 
        
    print("Checkpoint:%d" %(steps_log))

    for i in range(steps):
        step = i + steps_log
        input_data = sess.run(input_data_iter)
        #print(input_data)
        if i % 5 == 0:
            s = sess.run(merged_summary,{x: input_data[0], y_: input_data[1]})
            file_writer.add_summary(s,step)
        if i % 50 == 0:
            print('Where: %d, loss: %f' % (step,loss.eval({x: input_data[0], y_: input_data[1]})))
            
        train_op.run({x: input_data[0], y_: input_data[1]})

    file_writer.close()
    saver.save(sess,export_dir)

#predict_test()
#train_test(1, 60, 100)
#init_model_predict()