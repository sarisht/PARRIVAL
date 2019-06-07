#include "random.h"
#include <cstdlib>

Random::Random(int max, int seed) {
    srand(seed); 
    while (numbers.size() < max+4) {
        numbers.push_back(rand());
    }
    it = 0;
}
int Random::next() {
    if (it >= numbers.size() - 4) it = 0;
    return numbers.at(it++);
}