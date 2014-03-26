/*!
 * 
 *
 * \brief       Implements the generational Multi-objective Covariance Matrix Adapation ES.
 * 
 * 
 *
 * \author      T.Voss
 * \date        2010
 *
 *
 * \par Copyright 1995-2014 Shark Development Team
 * 
 * <BR><HR>
 * This file is part of Shark.
 * <http://image.diku.dk/shark/>
 * 
 * Shark is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published 
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * Shark is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public License
 * along with Shark.  If not, see <http://www.gnu.org/licenses/>.
 *
 */
#ifndef SHARK_ALGORITHMS_DIRECT_SEARCH_MOCMA
#define SHARK_ALGORITHMS_DIRECT_SEARCH_MOCMA

// MOO specific stuff
#include <shark/Algorithms/DirectSearch/Indicators/HypervolumeIndicator.h>
#include <shark/Algorithms/DirectSearch/Indicators/AdditiveEpsilonIndicator.h>
#include <shark/Algorithms/DirectSearch/Indicators/LeastContributorApproximator.h>
#include <shark/Algorithms/DirectSearch/Operators/Selection/IndicatorBasedSelection.h>
#include <shark/Algorithms/DirectSearch/Operators/Evaluation/PenalizingEvaluator.h>
#include <shark/Algorithms/DirectSearch/CMA/CMAIndividual.h>


#include <shark/Algorithms/AbstractMultiObjectiveOptimizer.h>
#include <shark/Core/ResultSets.h>
#include <shark/Core/SearchSpaces/VectorSpace.h>

#include <boost/foreach.hpp>

namespace shark {

namespace detail {

/**
 * \brief Implements the generational MO-CMA-ES.
 *
 * Please see the following papers for further reference:
 *	- Igel, Suttorp and Hansen. Steady-state Selection and Efficient Covariance Matrix Update in the Multi-Objective CMA-ES.
 *	- Vo�, Hansen and Igel. Improved Step Size Adaptation for the MO-CMA-ES.
 */
template<typename Indicator = HypervolumeIndicator>
class MOCMA {
public:
	
	/**
	 * \brief The result type of the optimizer, a vector of tuples \f$( \vec x, \vec{f}( \vec{x} )\f$.
	 */
	typedef std::vector< ResultSet< RealVector, RealVector > > SolutionSetType;
	
	/**
	 * \brief Typedef of the algorithm's own type.
	 */
	typedef MOCMA<Indicator> this_type;
	
	std::vector<CMAIndividual > m_pop; ///< Population of size \f$\mu+\mu\f$.
	shark::moo::PenalizingEvaluator m_evaluator; ///< Evaluation operator.
	IndicatorBasedSelection< Indicator > m_selection; ///< Selection operator relying on the (contributing) hypervolume indicator.

	bool m_useNewUpdate; ///< Flag for deciding whether the improved step-size adaptation shall be used.
	double m_individualSuccessThreshold;
	double m_initialSigma;
	
	/**
	 * \brief Default parent population size.
	 */
	static std::size_t DEFAULT_MU()	{
		return 100;
	}

	/**
	 * \brief Default penalty factor for penalizing infeasible solutions.
	 */
	static double DEFAULT_PENALTY_FACTOR() {
		return 1E-6;
	}

	/**
	 * \brief Default success threshold for step-size adaptation.
	 */
	static double DEFAULT_SUCCESS_THRESHOLD() {
		return 0.44;
	}

	/**
	 * \brief Default notion of success.
	 */
	static const char *DEFAULT_NOTION_OF_SUCCESS()	{
		return "IndividualBased";
	}
	/**
	 * \brief Default choice for the initial sigma
	 */
	static double DEFAULT_INITIAL_SIGMA() {
		return 1.0;
	}

	/**
	 * \brief Default c'tor.
	 */
	MOCMA() {
		init();
	}

	/**
	 * \brief Returns the name of the algorithm.
	 */
	std::string name() const {
		return "MOCMA";
	}
	
	std::size_t mu()const{
		return m_selection.m_mu;
	}

	/**
	 * \brief Stores/loads the algorithm's state.
	 * \tparam Archive The type of the archive.
	 * \param [in,out] archive The archive to use for loading/storing.
	 * \param [in] version Currently unused.
	 */
	template<typename Archive>
	void serialize(Archive &archive, const unsigned int version) {
		archive &BOOST_SERIALIZATION_NVP(m_pop);

		archive &BOOST_SERIALIZATION_NVP(m_evaluator);
		archive &BOOST_SERIALIZATION_NVP(m_selection);
		archive &BOOST_SERIALIZATION_NVP(m_useNewUpdate);
	}

	/**
	 * \brief Initializes the algorithm.
	 * \param [in] mu The population size.
	 * \param [in] penaltyFactor The penalty factor for penalizing infeasible solutions.
	 * \param [in] successThreshold The success threshold \f$p_{\text{thresh}}\f$ for cutting off evolution path updates.
	 * \param [in] notionOfSuccess The notion of success.
	 * \param [in] initialSigma value for the initial choice of sigma.
	*/
	void init(unsigned int mu = this_type::DEFAULT_MU(),
		double penaltyFactor = this_type::DEFAULT_PENALTY_FACTOR(),
	        double successThreshold = this_type::DEFAULT_SUCCESS_THRESHOLD(),
	        const std::string &notionOfSuccess = this_type::DEFAULT_NOTION_OF_SUCCESS(),
		double initialSigma = this_type::DEFAULT_INITIAL_SIGMA()
	) {
		m_selection.setMu(mu);
		m_initialSigma = initialSigma;
		m_evaluator.m_penaltyFactor = penaltyFactor;
		m_individualSuccessThreshold = successThreshold;

		if (notionOfSuccess == "IndividualBased") {
			m_useNewUpdate = false;
		} else if (notionOfSuccess == "PopulationBased") {
			m_useNewUpdate = true;
		}
	}

	/**
	 * \brief Initializes the algorithm from a configuration-tree node.
	 *
	 * The following sub keys are recognized:
	 *	- Mu, type: unsigned int, default value: 100.
	 *	- PenaltyFactor, type: double, default value: \f$10^{-6}\f$.
	 *	- SuccessThreshold, type: double, default value: 0.44.
	 *	- NotionOfSuccess, type: string, default value: IndividualBased.
	 *	- initialSigma, an initial estimate for the diagonal of the covariance matrix.
	 *
	 * \param [in] node The configuration tree node.
	 */
	void configure(PropertyTree const& node) {
		init(
			node.get<unsigned int>("Mu", this_type::DEFAULT_MU()),
			node.get<double>("PenaltyFactor", this_type::DEFAULT_PENALTY_FACTOR()),
			node.get<double>("SuccessThreshold", this_type::DEFAULT_SUCCESS_THRESHOLD()),
			node.get<std::string>("NotionOfSuccess", this_type::DEFAULT_NOTION_OF_SUCCESS()),
			node.template get<double>("InitialSigma",this_type::DEFAULT_INITIAL_SIGMA())
		);
	}

	/**
	 * \brief Initializes the algorithm for the supplied objective function.
	 * \tparam ObjectiveFunction The type of the objective function,
	 * needs to adhere to the concept of an AbstractObjectiveFunction.
	 * \param [in] f The objective function.
	 * \param [in] sp An initial search point.
	 */
	template<typename ObjectiveFunction>
	void init(const ObjectiveFunction &f, const shark::RealVector &sp = shark::RealVector()) {
		m_pop.reserve(2 * mu());
		std::size_t noObjectives = f.numberOfObjectives();
		std::size_t noVariables = f.numberOfVariables();
		
		for(std::size_t i = 0; i != 2 * mu(); ++i){
			CMAIndividual ind(noVariables,noObjectives,m_individualSuccessThreshold,m_initialSigma);
			f.proposeStartingPoint(ind.searchPoint());
			boost::tuple< RealVector, RealVector > result = m_evaluator(f, ind.searchPoint());
			ind.fitness(shark::tag::PenalizedFitness()) = boost::get< shark::moo::PenalizingEvaluator::PENALIZED_RESULT >(result);
			ind.fitness(shark::tag::UnpenalizedFitness()) = boost::get< shark::moo::PenalizingEvaluator::UNPENALIZED_RESULT >(result);
			m_pop.push_back(ind);
		}
	}

	/**
	 * \brief Executes one iteration of the algorithm.
	 * \tparam The type of the objective to iterate upon.
	 * \param [in] f The function to iterate upon.
	 * \returns The Pareto-set/-front approximation after the iteration.
	 */
	template<typename Function>
	SolutionSetType step(const Function &f) {

		//generate new offspring
		for (std::size_t i = 0; i < mu(); i++) {
			m_pop[mu()+i] = m_pop[i];
			m_pop[mu()+i].mutate();//mutate the search point
			m_pop[mu()+i].age() = 0;
			//function value evaluation
			boost::tuple< typename Function::ResultType, typename Function::ResultType > result = m_evaluator(f, m_pop[mu()+i].searchPoint());
			m_pop[mu()+i].fitness(shark::tag::PenalizedFitness()) = boost::get< shark::moo::PenalizingEvaluator::PENALIZED_RESULT >(result);
			m_pop[mu()+i].fitness(shark::tag::UnpenalizedFitness()) = boost::get< shark::moo::PenalizingEvaluator::UNPENALIZED_RESULT >(result);
		}

		m_selection(m_pop);
		//determine from the selection which parent-offspring pair has been successfull
		if (m_useNewUpdate) {
			//new update: an offspring is successfull if it is selected
			for (std::size_t i = 0; i < mu(); i++) {
				if ( m_pop[mu()+i].selected()) {
					m_pop[mu()+i].noSuccessfulOffspring() += 1.0;
					m_pop[i].noSuccessfulOffspring() += 1.0;
				}
			}
		}
		else
		{
			//old update: if the offspring is better than its parent, it is successfull
			for (std::size_t i = 0; i < mu(); i++) {
				if ( m_pop[mu()+i].selected() && m_pop[mu()+i].rank() <= m_pop[i].rank()) {
					m_pop[mu()+i].noSuccessfulOffspring() += 1.0;
					m_pop[i].noSuccessfulOffspring() += 1.0;
				}
			}
		}
		
		//partition the selected individuals to the front
		std::partition(m_pop.begin(), m_pop.end(),CMAIndividual::IsSelected);
		

		//update individuals and generate solution set
		SolutionSetType solutionSet;
		for (unsigned int i = 0; i < mu(); i++) {
			m_pop[i].age()++;
			m_pop[i].update();
			solutionSet.push_back(shark::makeResultSet(m_pop[i].searchPoint(), m_pop[i].fitness(shark::tag::UnpenalizedFitness())));
		}

		return solutionSet;
	};

};
}

typedef TypeErasedMultiObjectiveOptimizer< VectorSpace<double>, detail::MOCMA< HypervolumeIndicator > > MOCMA;
typedef TypeErasedMultiObjectiveOptimizer< VectorSpace<double>, detail::MOCMA< AdditiveEpsilonIndicator > > EpsilonMOCMA;
typedef TypeErasedMultiObjectiveOptimizer< VectorSpace<double>, detail::MOCMA< LeastContributorApproximator< FastRng, HypervolumeCalculator > > > ApproximatedVolumeMOCMA;

}

#endif
