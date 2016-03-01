FROM ubuntu:trusty
RUN apt-get update
RUN apt-get install -y build-essential
RUN apt-get install -y 'g++-mingw-w64-x86-64'
COPY bmsampler.cpp /bmsampler/
COPY vendor /bmsampler/vendor
WORKDIR /bmsampler/
RUN x86_64-w64-mingw32-g++ bmsampler.cpp -I/bmsampler/vendor/libsndfile/include -I/bmsampler/vendor/json/include -std=c++11 -L/bmsampler/vendor/libsndfile/lib -static-libgcc -static-libstdc++ -Wl,-Bstatic -lstdc++ -lpthread -Wl,-Bdynamic -lsndfile-1 -o bmsampler.exe
