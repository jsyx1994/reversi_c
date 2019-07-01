g++ -c ultiTrain.cpp -std=c++11 -I/Users/mro/Documents/venv/ai/include/python3.6m -Wno-unused-result -Wsign-compare -Wunreachable-code -fno-common -dynamic -DNDEBUG -g -fwrapv -O3 -Wall -Wstrict-prototypes

g++ ultiTrain.o -o ~/Desktop/LinuxCNN_reversi/self-play -L/usr/local/opt/python/Frameworks/Python.framework/Versions/3.6/lib/python3.6/config-3.6m-darwin -lpython3.6m -ldl -framework CoreFoundation

./self-play "{\"requests\":[{\"x\":-1,\"y\":-1}],\"responses\":[]}"