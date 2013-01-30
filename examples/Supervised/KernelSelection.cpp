#include <shark/Data/DataDistribution.h>
#include <shark/Models/Kernels/GaussianRbfKernel.h>
#include <shark/ObjectiveFunctions/RadiusMarginQuotient.h>
#include <shark/Algorithms/GradientDescent/Rprop.h>


using namespace shark;


int main(int argc, char** argv)
{
	// generate dataset
	Chessboard problem;
	ClassificationDataset data = problem.generateDataset(100);

	// brute force search in [1.0, 10000.0] on log scale
	GaussianRbfKernel<> kernel;
	RadiusMarginQuotient<RealVector> rm(data, &kernel);
	RealVector param(1);
	double best_value = 1e100;
	double best_gamma = 0.0;
	int i;
	printf("Grid search in the range [1, 10000] on log scale:\n");
	for (i=0; i<=400; i++)
	{
		double gamma = pow(10.0, i / 100.0);
		param(0) = gamma;
		double f = rm.eval(param);
		if (f < best_value)
		{
			best_value = f;
			best_gamma = gamma;
		}
	}
	printf("best gamma: %10.6g   radius margin quotient: %16.12g\n", best_gamma, best_value);

	// gradient-based alternative
	IRpropPlus rprop;
	rprop.init(rm, RealVector(1, 100.0), 1.0);
	printf("\nGradient-based optimization (IRprop+, 50 steps):\n");
	for (i=0; i<50; i++) rprop.step(rm);
	best_gamma = rprop.solution().point(0);
	best_value = rm.eval(RealVector(1, best_gamma));
	printf("best gamma: %10.6g   radius margin quotient: %16.12g\n", best_gamma, best_value);
}
