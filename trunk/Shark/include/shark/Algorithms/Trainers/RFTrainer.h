//===========================================================================
/*!
 *  \brief Random Forest Trainer
 *
 *  \author  K. N. Hansen
 *  \date    2011-2012
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


#ifndef SHARK_ALGORITHMS_TRAINERS_RFTRAINER_H
#define SHARK_ALGORITHMS_TRAINERS_RFTRAINER_H

#include <shark/Algorithms/Trainers/AbstractTrainer.h>
#include <shark/Models/Trees/RFClassifier.h>

#include <boost/unordered_map.hpp>

namespace shark {
/*!
 * \brief Random Forest
 *
 * Random Forest is an ensemble learner, that builds multiple binary decision trees.
 * The trees are built using a variant of the CART methodology
 *
 * The algorithm used to generate each tree based on the SPRINT algorithm, as
 * shown by J. Shafer et al.
 *
 * Typically 100+ trees are built, and classification/regression is done by combining
 * the results generated by each tree. Typically the a majority vote is used in the
 * classification case, and the mean is used in the regression case
 *
 * Each tree is built based on a random subset of the total dataset. Furthermore
 * at each split, only a random subset of the attributes are investigated for
 * the best split
 *
 * The node impurity is measured by the Gini criteria in the classification
 * case, and the total sum of squared errors in the regression case
 *
 * After growing a maximum sized tree, the tree is added to the ensemble
 * without pruning.
 *
 * For detailed information about Random Forest, see Random Forest
 * by L. Breiman et al. 2001.
 *
 * For detailed information about the SPRINT algorithm, see
 * SPRINT: A Scalable Parallel Classifier for Data Mining
 * by J. Shafer et al.
 */
class RFTrainer 
: public AbstractTrainer<RFClassifier, unsigned int>
, public AbstractTrainer<RFClassifier>
{

public:
	/// Constructor
	RFTrainer();

	/// \brief From INameable: return the class name.
	std::string name() const
	{ return "RFTrainer"; }

	/// Train a random forest for classification.
	void train(RFClassifier& model, const ClassificationDataset& dataset);

	/// Train a random forest for regression.
	void train(RFClassifier& model, const RegressionDataset& dataset);

	/// Set the number of random attributes to investigate at each node.
	void setMTry(std::size_t mtry);

	/// Set the number of trees to grow.
	void setNTrees(std::size_t nTrees);

	/// Controls when a node is considered pure. If set to 1, a node is pure
	/// when it only consists of a single node.
	void setNodeSize(std::size_t nTrees);

	/// Set the fraction of the original training dataset to use as the
	/// out of bag sample. The default value is 0.66.
	void setOOBratio(double ratio);

protected:
	/// attribute table
	typedef std::vector < RealVector > AttributeTable;
	/// collecting of attribute tables
	typedef std::vector < AttributeTable > AttributeTables;

	/// Create attribute tables from a data set, and in the process create a count matrix (cAbove).
	/// A dataset with m features results in m attribute tables.
	/// [attribute | class/value | row id ]
	void createAttributeTables(Data<RealVector> const& dataset, AttributeTables& tables);

	/// Create a count matrix as used in the classification case.
	void createCountMatrix(const ClassificationDataset& dataset, boost::unordered_map<std::size_t, std::size_t>& cAbove);

	// Split attribute tables into left and right parts.
	void splitAttributeTables(const AttributeTables& tables, std::size_t index, std::size_t valIndex, AttributeTables& LAttributeTables, AttributeTables& RAttributeTables);

	/// Build a decision tree for classification
	RFClassifier::SplitMatrixType buildTree(const AttributeTables& tables, const ClassificationDataset& dataset, boost::unordered_map<std::size_t, std::size_t>& cAbove, std::size_t nodeId);

	/// Builds a decision tree for regression
	RFClassifier::SplitMatrixType buildTree(const AttributeTables& tables, const RegressionDataset& dataset, const std::vector<RealVector>& labels, std::size_t nodeId);

	/// comparison function for sorting an attributeTable
	static bool tableSort(const RealVector& inner1, const RealVector& inner2);

	/// Generate a histogram from the count matrix.
	RealVector hist(boost::unordered_map<std::size_t, std::size_t> countMatrix);

	/// Average label over a vector.
	RealVector average(const std::vector<RealVector>& labels);

	/// Calculate the Gini impurity of the countMatrix
	double gini(boost::unordered_map<std::size_t, std::size_t>& countMatrix, std::size_t n);

	/// Total Sum Of Squares
	double totalSumOfSquares(std::vector<RealVector>& labels, std::size_t from, std::size_t to, const RealVector& sumLabel);

	/// Generate random table indices.
	void generateRandomTableIndicies(std::set<std::size_t>& tableIndicies);

	/// Reset the training to its default parameters.
	void setDefaults();

	/// Number of attributes in the dataset
	std::size_t m_inputDimension;

	/// size of labels
	std::size_t m_labelDimension;

	/// maximum size of the histogram;
	/// classification case: maximum number of classes
	unsigned int m_maxLabel;

	/// number of attributes to randomly test at each inner node
	std::size_t m_try;

	/// number of trees in the forest
	std::size_t m_B;

	/// number of samples in the terminal nodes
	std::size_t m_nodeSize;

	/// fraction of the data set used for growing trees
	/// 0 < m_OOBratio < 1
	double m_OOBratio;

	/// true if the trainer is used for regression, false otherwise.
	bool m_regressionLearner;
};


}
#endif