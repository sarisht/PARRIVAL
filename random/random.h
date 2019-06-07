#include <vector>

using namespace std;

class Random {
    private:
        vector<int> numbers;
        int it;
    public:
        Random(int max, int seed);
        int next();
};