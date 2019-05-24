#ifndef PomoCountFileReader_H
#define PomoCountFileReader_H

#include "DelimitedDataReader.h"
#include "AbstractHomologousDiscreteCharacterData.h"
#include "HomologousDiscreteCharacterData.h"
#include "PoMoState.h"

#include <string>
#include <vector>

namespace RevBayesCore {


	/**
	 * Reader for allele count filesfor use with Pomo matrices.
	 * Same format as read by IQ-Tree.
	 * Description: http://www.iqtree.org/doc/Polymorphism-Aware-Models/
	 * The first line contains COUNTSFILE  then the number of populations and the number of sites.
	 * The following line contains the header: chromosome, site position, population names.
	 * The following lines contain the states.
	 *
	 *
	 *
	 * @copyright Copyright 2009-
	 * @author The RevBayes Development Core Team (Bastien Boussau)
	 * @since 2015-03-03, version 1.0
	 *
	 */
	class PomoCountFileReader : public DelimitedDataReader {

	public:

		PomoCountFileReader(const std::string &fn, const size_t virtualPopulationSize = 9, char d=' ', size_t ns=0);

		const size_t 																getNumberOfPopulations( void );
		const size_t 																getNumberOfSites( void );
		HomologousDiscreteCharacterData<PomoState>* getMatrix( void );
		const size_t 																getVirtualPopulationSize( void );

	protected:

		//std::vector<std::string>                        names;
		//MatrixReal          							matrix;
		size_t numberOfPopulations_;
		size_t numberOfSites_;
		size_t virtualPopulationSize_;
		std::vector<string> names_;
		HomologousDiscreteCharacterData<PomoState>* matrix_;
	};

}

#endif
