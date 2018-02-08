#include "utils.h"
#include <random>
#include <time.h>
using namespace std;

static uniform_real_distribution<double> uniform;
static default_random_engine engine(clock());

double randomDouble(double max)
{
	return uniform(engine) * max;
}

int randomInt(int max)
{
	return static_cast<int>(randomDouble(max));
}