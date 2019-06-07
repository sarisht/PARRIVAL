clang++ -Xpreprocessor -fopenmp -lomp -I"$(brew --prefix libomp)/include" -L"$(brew --prefix libomp)/lib" -flto -march=native -O3 experiments/runExperiment.cpp -ltbb
./a.out $1/edges.txt $1/labels.txt $1/$2.txt $3
