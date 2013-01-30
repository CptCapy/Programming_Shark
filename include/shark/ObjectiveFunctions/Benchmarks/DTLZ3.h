//===========================================================================
/*!
*  \brief Objective function DTLZ3.
*
*  \author T.Voss, T. Glasmachers, O.Krause
*  \date 2010-2011
*
*  \par Copyright (c) 1998-2007:
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
#ifndef SHARK_OBJECTIVEFUNCTIONS_BENCHMARK_DTLZ3_H
#define SHARK_OBJECTIVEFUNCTIONS_BENCHMARK_DTLZ3_H

#include <shark/Core/AbstractBoxConstraintsProvider.h>
#include <shark/ObjectiveFunctions/AbstractMultiObjectiveFunction.h>
#include <shark/Core/Traits/ObjectiveFunctionTraits.h>
#include <shark/Core/Traits/MultiObjectiveFunctionTraits.h>

#include <shark/Core/SearchSpaces/VectorSpace.h>
#include <shark/Rng/GlobalRng.h>

#include <boost/math/special_functions.hpp>


namespace shark {
	/**
	* \brief Implements the benchmark function DTLZ3.
	*
	* See: http://citeseerx.ist.psu.edu/viewdoc/download?doi=10.1.1.18.7531&rep=rep1&type=pdf
	* The benchmark function exposes the following features:
	*	- Scalable w.r.t. the searchspace and w.r.t. the objective space.
	*/
	struct DTLZ3 : public AbstractMultiObjectiveFunction< VectorSpace<double> >,
		public TraitsBoxConstraintsProvider< VectorSpace<double>::PointType, DTLZ3 > {

			typedef AbstractMultiObjectiveFunction< VectorSpace<double> > super;
			typedef TraitsBoxConstraintsProvider< VectorSpace<double>::PointType, DTLZ3 > meta;

			typedef super::ResultType ResultType;
			typedef super::SearchPointType SearchPointType;

			DTLZ3() : super( 2 ) {
				m_features |= CAN_PROPOSE_STARTING_POINT;
				m_features |= IS_CONSTRAINED_FEATURE;
				m_features |= CAN_PROVIDE_CLOSEST_FEASIBLE;
				m_name="DTLZ3";
			}

			void init() {
			}

		ResultType eval( const SearchPointType & x ) const {
			m_evaluationCounter++;

			ResultType value( noObjectives() );

			int    k ;
			double g ;

			k = numberOfVariables() - noObjectives() + 1 ;
			g = 0.0 ;

			for( unsigned int i = numberOfVariables() - k; i < numberOfVariables(); i++ )
				g += boost::math::pow<2>( x( i ) - 0.5 ) - ::cos( 20. * M_PI * ( x(i) - 0.5 ) );

			g = 100 * (k + g);

			 for( unsigned int i = 0; i < noObjectives(); i++ ) {
			     double f = (1 + g);
			     for( unsigned int j = 0; j < noObjectives() - (i + 1); j++)            
				 f *= ::cos( x[j] * 0.5 * M_PI );                
			     if (i != 0){
				 unsigned int aux = noObjectives() - (i + 1);
				 f *= ::sin(x[aux] * 0.5 * M_PI);
			     }
			     value( i ) = f;
			 }

			return value;
		}

		void proposeStartingPoint( SearchPointType & x ) const {
			meta::proposeStartingPoint( x, m_numberOfVariables );
		}

		bool isFeasible( const SearchPointType & v ) const {
			return( meta::isFeasible( v ) );
		}

		void closestFeasible( SearchPointType & v ) const {
			meta::closestFeasible( v );
		}
	};

	/**
	 * \brief Specializes ObjectiveFunctionTraits for DTLZ3.
	 */
	template<> struct ObjectiveFunctionTraits<DTLZ3> {

		static DTLZ3::SearchPointType lowerBounds( unsigned int n ) {
			return( DTLZ3::SearchPointType( n, 0. ) );
		}

		static DTLZ3::SearchPointType upperBounds( unsigned int n ) {
			return( DTLZ3::SearchPointType( n, 1. ) );
		}

	};

	/**
	 * \brief Specializes MultiObjectiveFunctionTraits for DTLZ3.
	 */
	template<> struct MultiObjectiveFunctionTraits<DTLZ3> {

		/**
		* \brief Models the reference Pareto-front type.
		*/
		typedef std::vector< DTLZ3::ResultType > ParetoFrontType;

		/**
		* \brief Models the reference Pareto-set type.
		*/
		typedef std::vector< DTLZ3::SearchPointType > ParetoSetType;

		static std::vector< DTLZ3::ResultType > referenceFront( std::size_t noPoints, std::size_t n, std::size_t m ) {
			if( m != 2 )
				throw( shark::Exception( "DTLZ3: No reference front for no. of objectives other than 2 available." ) );
			std::vector< DTLZ3::ResultType > result( noPoints, DTLZ3::ResultType( m ) );
			for( std::size_t i = 0; i < result.size(); i++ ) {
				result[ i ][ 0 ] = static_cast< double >( i ) / static_cast< double >( result.size() - 1 );
				result[ i ][ 1 ] = ::sqrt( 1 - boost::math::pow<2>( result[ i ][ 0 ] ) );
			}

			return( result );
		}
	};

	ANNOUNCE_MULTI_OBJECTIVE_FUNCTION( DTLZ3, shark::moo::RealValuedObjectiveFunctionFactory );
}
#endif
