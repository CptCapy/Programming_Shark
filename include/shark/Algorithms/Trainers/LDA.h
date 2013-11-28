//===========================================================================
/*!
 *  \file LDA.h
 *
 *  \brief LDA
 *
 *  \author O. Krause
 *  \date 2010
 *
 *  \par Copyright (c) 1998-2011:
 *      Institut f&uuml;r Neuroinformatik<BR>
 *      Ruhr-Universit&auml;t Bochum<BR>
 *      D-44780 Bochum, Germany<BR>
 *      Phone: +49-234-32-25558<BR>
 *      Fax:   +49-234-32-14209<BR>
 *      eMail: Shark-admin@neuroinformatik.ruhr-uni-bochum.de<BR>
 *      www:   http://www.neuroinformatik.ruhr-uni-bochum.de<BR>
 *      <BR>
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
#ifndef SHARK_ALGORITHMS_TRAINERS_LDA_H
#define SHARK_ALGORITHMS_TRAINERS_LDA_H


#include <shark/Core/IParameterizable.h>
#include <shark/Models/LinearClassifier.h>
#include <shark/Algorithms/Trainers/AbstractTrainer.h>

namespace shark {


//!
//! \brief Linear Discriminant Analysis (LDA)
//!
//! This classes implements the well known linear discriminant analysis. LDA assumes that
//! every point is drawn from a multivariate normal distributions. Every class has its own mean
//! but all classes have the same covariance.
//!
//! An arbitrary number of classes is supported. The resulting model is of the form
//! \f[ \arg \max_c \log(p(x|c)*P(c)) \f]
//! where \f$ p(x|c) = \exp(-(x-m_c)^T(C+\alpha I)(x-m_c)) \f$.
//! \f$ m_c\f$ are the means of class c, \f$ C \f$ is the covariance matrix formed by all data points.
//! The regularization paramter \f$ \alpha \f$ is by default 0. The trainer is implemented such, that
//! it still works when C is singular, in this case the singular directions are ignored. 	
class LDA : public AbstractTrainer<LinearClassifier<>, unsigned int>, public IParameterizable
{
public:
	/// constructor
	LDA(double regularization = 0.0){
		setRegularization(regularization);
	}

	/// \brief From INameable: return the class name.
	std::string name() const
	{ return "Linear Discriminant Analysis (LDA)"; }

	/// return the regularization constant
	double regularization()const{
		return m_regularization;
	}

	/// set the regularization constant. 0 means no regularization.
	void setRegularization(double regularization) {
		RANGE_CHECK(regularization >= 0.0);
		m_regularization = regularization;
	}

	/// inherited from IParameterizable; read the regularization parameter
	RealVector parameterVector() const {
		RealVector param(1);
		param(0) = m_regularization;
		return param;
	}
	/// inherited from IParameterizable; set the regularization parameter
	void setParameterVector(RealVector const& param) {
		SIZE_CHECK(param.size() == 1);
		m_regularization = param(0);
	}
	/// inherited from IParameterizable
	size_t numberOfParameters() const {
		return 1;
	}

	//! Compute the LDA solution for a multi-class problem.
	void train(LinearClassifier<>& model, LabeledData<RealVector, unsigned int> const& dataset);

protected:
	//!The regularization parameter \f$ \lambda \f$ adds
	//! \f$ - \lambda I \f$ to the second moment matrix, where
	//! \f$ I \f$ is the identity matrix
	double m_regularization;
};

}
#endif

