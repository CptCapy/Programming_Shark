//===========================================================================
/*!
 *  \brief Generic stochastic gradient descent training for kernel-based models.
 *
 *
 *  \author  T. Glasmachers
 *  \date    2013
 *
 *
 *  <BR><HR>
 *  This file is part of Shark. This library is free software;
 *  you can redistribute it and/or modify it under the terms of the
 *  GNU General Public License as published by the Free Software
 *  Foundation; either version 3, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this library; if not, see <http://www.gnu.org/licenses/>.
 *
 */
//===========================================================================


#ifndef SHARK_ALGORITHMS_KERNELSGDTRAINER_H
#define SHARK_ALGORITHMS_KERNELSGDTRAINER_H


#include <shark/Algorithms/Trainers/AbstractTrainer.h>
#include <shark/Models/Kernels/KernelExpansion.h>
#include <shark/ObjectiveFunctions/Loss/AbstractLoss.h>


namespace shark {


///
/// \brief Generic stochastic gradient descent training for kernel-based models.
///
/// Given a differentiable loss function L(f, y) for classification
/// this trainer solves the regularized risk minimization problem
/// \f[
///     \min \frac{1}{2} \sum_j \|w_j\|^2 + C \sum_i L(y_i, f(x_i)),
/// \f]
/// where i runs over training data, j over classes, and C is the
/// regularization parameter.
///
template <class InputType>
class KernelSGDTrainer : public AbstractTrainer< KernelClassifier<InputType> >
{
public:
	typedef AbstractTrainer< KernelExpansion<InputType> > base_type;
	typedef AbstractKernelFunction<InputType> KernelType;
	typedef KernelClassifier<InputType> ClassifierType;
	typedef KernelExpansion<InputType> ModelType;
	typedef AbstractLoss<unsigned int, RealVector> LossType;
	typedef typename ConstProxyReference<typename Batch<InputType>::type const>::type ConstBatchInputReference;

	/// \brief Constructor
	///
	/// \param  kernel          kernel function to use for training and prediction
	/// \param  loss            (sub-)differentiable loss function
	/// \param  C               regularization parameter - always the 'true' value of C, even when unconstrained is set
	/// \param  offset          whether to train with offset/bias parameter or not
	/// \param  unconstrained   when a C-value is given via setParameter, should it be piped through the exp-function before using it in the solver?
	KernelSGDTrainer(KernelType* kernel, const LossType* loss, double C, bool offset, bool unconstrained = false)
	: m_kernel(kernel)
	, m_loss(loss)
	, m_C(C)
	, m_offset(offset)
	, m_unconstrained(unconstrained)
	{ }

	/// \brief From INameable: return the class name.
	std::string name() const
	{ return "KernelSGDTrainer"; }

	void train(ClassifierType& classifier, const LabeledData<InputType, unsigned int>& dataset)
	{
		std::size_t ell = dataset.numberOfElements();
		std::size_t batches = dataset.numberOfBatches();
		unsigned int classes = numberOfClasses(dataset);
		ModelType& model = classifier.decisionFunction();

		model.setStructure(m_kernel, dataset.inputs(), m_offset, classes);

		RealMatrix& alpha = model.alpha();
		RealVector& bias = model.offset();
		if (m_offset)
		{
			throw SHARKEXCEPTION("[KernelSGDTrainer::train] training with offset is not implemented yet");
		}

		// pre-compute the kernel matrix (may change in the future)
		// and create linear array of labels
		std::vector<unsigned int> y(ell);
		RealMatrix K(ell, ell);
		for (std::size_t i=0, ii=0; i<batches; i++)
		{
			ConstBatchInputReference xi = dataset.inputs().batch(i);
			for (std::size_t j=0, jj=j; j<i; j++)
			{
				ConstBatchInputReference xj = dataset.inputs().batch(j);
				RealMatrix mat;
				m_kernel->eval(xi, xj, mat);
				subrange(K, ii, ii+boost::size(xi), jj, jj+boost::size(xj)) = mat;
				subrange(K, jj, jj+boost::size(xj), ii, ii+boost::size(xi)) = trans(mat);
				jj += boost::size(xj);
			}
			RealMatrix mat;
			m_kernel->eval(xi, xi, mat);
			subrange(K, ii, ii+boost::size(xi), ii, ii+boost::size(xi)) = mat;

			Batch<unsigned int>::type const& yi = dataset.labels().batch(i);
			for (std::size_t n=0; n<yi.size(); n++) y[ii+n] = get(yi, n);
			ii += boost::size(xi);
		}

		// SGD loop
		double factor = 1.0;
		std::size_t iterations = std::max(10 * ell, std::size_t(std::ceil(m_C * ell)));
		for (std::size_t iter = 0; iter < iterations; iter++)
		{
			// active variable
			std::size_t b = Rng::discrete(0, ell - 1);

			// learning rate
			const double eta = 1.0 / (iter + 2.0);

			// compute prediction
			RealVector f_b(classes, 0.0);
			fast_prod(trans(alpha), row(K, b), f_b, false, factor);
//			if (m_offset) f_b += bias;

			// stochastic gradient descent (SGD) step
			RealVector derivative(classes, 0.0);
			m_loss->evalDerivative(y[b], f_b, derivative);
//			factor *= (1.0 - eta);
			factor = (1.0 - 1.0 / (iter + 3.0));   // should be numerically more stable
			row(alpha, b) -= (eta * m_C / factor) * derivative;
		}

		alpha *= factor;
//		offset *= factor;
	}

	KernelType* kernel()
	{ return m_kernel; }
	const KernelType* kernel() const
	{ return m_kernel; }
	void setKernel(KernelType* kernel)
	{ m_kernel = kernel; }

	bool isUnconstrained() const
	{ return m_unconstrained; }

	bool trainOffset() const
	{ return m_offset; }

	/// get the hyper-parameter vector
	RealVector parameterVector() const
	{
		size_t kp = m_kernel->numberOfParameters();
		RealVector ret(kp + 1);
		if (m_unconstrained)
			init(ret) << parameters(m_kernel), log(m_C);
		else
			init(ret) << parameters(m_kernel), m_C;
		return ret;
	}

	/// set the vector of hyper-parameters
	void setParameterVector(RealVector const& newParameters)
	{
		size_t kp = m_kernel->numberOfParameters();
		SHARK_ASSERT(newParameters.size() == kp + 1);
		init(newParameters) >> parameters(m_kernel), m_C;
		if (m_unconstrained) m_C = exp(m_C);
	}

	/// return the number of hyper-parameters
	size_t numberOfParameters() const{ 
		return m_kernel->numberOfParameters() + 1;
	}

protected:
	KernelType* m_kernel;                     ///< pointer to kernel function
	const LossType* m_loss;                   ///< pointer to loss function
	double m_C;                               ///< regularization parameter
	bool m_offset;                            ///< should the resulting model have an offset term?
	bool m_unconstrained;                     ///< should C be stored as log(C) as a parameter?
};


}
#endif
