/*
 * AMR.cxx
 *
 *  Created on: 2 juil. 2014
 *      Author: mekkas
 */

#include <algorithm>
#include <fstream>

#include <MEDCouplingAMRAttribute.hxx>
#include <MEDCouplingCartesianAMRMesh.hxx>
#include <MEDCouplingFieldDouble.hxx>
#include <MEDCouplingIMesh.hxx>
#include <BoxSplittingOptions.hxx>

#include "CdmathException.hxx"
#include "AMR.hxx"

using namespace ParaMEDMEM;
using namespace INTERP_KERNEL;
using namespace std;

AMR::AMR(int maxLevels,
		 int frequencyOfRefinement,
		 std::vector<double> efficiencyGoal,
		 std::vector<double> efficiencyThreshold,
		 std::vector<int> maximumNbOfCellsInPatch,
		 std::vector<int> minimumPatchLength,
		 std::vector<int> maximumPatchLength,
		 std::vector< std::vector<int> > coeficientsRefinement)
{
	_maxLevels=maxLevels;
	_efficiencyGoal=efficiencyGoal;
	_efficiencyThreshold=efficiencyThreshold;
	_maximumNbOfCellsInPatch=maximumNbOfCellsInPatch;
	_minimumPatchLength=minimumPatchLength;
	_maximumPatchLength=maximumPatchLength;
	_coeficientsRefinement=coeficientsRefinement;
	_frequencyOfRefinement=frequencyOfRefinement;
	_fields=NULL;
}

int
AMR::getFrequencyOfRefinement( void ) const
{
	return _frequencyOfRefinement;
}

void
AMR::setFrequencyOfRefinement( int frequencyOfRefinement )
{
	_frequencyOfRefinement=frequencyOfRefinement;
}

std::vector< std::pair<std::string,int> >
AMR::getFieldsInfo( void ) const
{
	return _fieldsInfos;
}

void
AMR::setFieldsInfo(std::vector< std::pair<std::string,int> > fieldsInfos)
{
	_fieldsInfos=fieldsInfos;
}

std::vector<double>
AMR::getEfficiencyGoal( void ) const
{
	return _efficiencyGoal;
}

double
AMR::getEfficiencyGoalAtLevel( int iLevel ) const
{
	if(iLevel>_maxLevels-1)
	    throw CdmathException("AMR::getEfficiencyGoalAtLevel : iLevel>_maxLevels-1 !");
	return _efficiencyGoal[iLevel];
}

std::vector<double>
AMR::getEfficiencyThreshold( void ) const
{
	return _efficiencyThreshold;
}

double
AMR::getEfficiencyThresholdAtLevel( int iLevel ) const
{
	if(iLevel>_maxLevels-1)
	    throw CdmathException("AMR::getEfficiencyThresholdAtLevel : iLevel>_maxLevels-1 !");
	return _efficiencyThreshold[iLevel];
}

int
AMR::getMaximumLevels( void )
{
	return _maxLevels;
}


int
AMR::getMaximumNbOfCellsInPatchAtLevel( int iLevel ) const
{
	if(iLevel>_maxLevels-1)
	    throw CdmathException("AMR::getMaximumNbOfCellsInPatchAtLevel : iLevel>_maxLevels-1 !");
	return _maximumNbOfCellsInPatch[iLevel];
}

std::vector<int>
AMR::getMaximumNbOfCellsInPatch( void ) const
{
	return _maximumNbOfCellsInPatch;
}

int
AMR::getMinimumPatchLengthAtLevel( int iLevel ) const
{
	if(iLevel>_maxLevels-1)
	    throw CdmathException("AMR::getMinimumPatchLengthAtLevel : iLevel>_maxLevels-1 !");
	return _minimumPatchLength[iLevel];
}

std::vector<int>
AMR::getMinimumPatchLength( void ) const
{
	return _minimumPatchLength;
}

int
AMR::getMaximumPatchLengthAtLevel( int iLevel ) const
{
	if(iLevel>_maxLevels-1)
	    throw CdmathException("AMR::getMaximumPatchLengthAtLevel : iLevel>_maxLevels-1 !");
	return _maximumPatchLength[iLevel];
}

std::vector<int>
AMR::getMaximumPatchLength( void ) const
{
	return _maximumPatchLength;
}

std::vector<int>
AMR::getCoeficientsRefinementAtLevel( int iLevel ) const
{
	if(iLevel>_maxLevels-1)
	    throw CdmathException("AMR::getCoeficientsRefinementAtLevel : iLevel>_maxLevels-1 !");
	return _coeficientsRefinement[iLevel];
}

std::vector< std::vector<int> >
AMR::getCoeficientsRefinement( void ) const
{
	return _coeficientsRefinement;
}

ParaMEDMEM::MEDCouplingAutoRefCountObjectPtr<ParaMEDMEM::MEDCouplingAMRAttribute>
AMR::getFields( void ) const
{
	return _fields;
}

void
AMR::initialize(const MEDCouplingIMesh* coarseMesh,
		 	 	std::vector< std::pair<std::string,int> > fieldsInfos,
		 	 	const GenericSolver& genericSolver)
{
	_fieldsInfos=fieldsInfos;

	MEDCouplingFieldDouble* yy0=genericSolver.initialConditions(coarseMesh);

	// !!!!!!!!!!! CDLIM PHYSIQUE !!!!!!!!!!!!
	//	vector< vector<int> > bottomG(numberOfCellsGhost),leftG(numberOfCellsGhost),topG(numberOfCellsGhost),rightG(numberOfCellsGhost);
	//    getBLTRToPeriodic(m1,numberOfCellsGhost,bottomG,leftG,topG,rightG);
	//   vector< vector<int> > bottom(numberOfCellsGhost),left(numberOfCellsGhost),top(numberOfCellsGhost),right(numberOfCellsGhost);
	//   getBLTRToPeriodic(amr1->getImageMesh(),numberOfCellsGhost,bottom,left,top,right);
	//    fillCellGhostPeriodic(bottom,left,top,right,bottomG,leftG,topG,rightG,amr1->getImageMesh(),yy0,numberOfCellsGhost);
	///////////////////////////

	MEDCouplingFieldDouble* fd=genericSolver.refinementCriterion(yy0);

	yy0->decrRef();

    MEDCouplingAutoRefCountObjectPtr<MEDCouplingCartesianAMRMesh> amr=MEDCouplingCartesianAMRMesh::New(const_cast<MEDCouplingIMesh *>(coarseMesh));

    int maxLevels=getMaximumLevels();
	vector<BoxSplittingOptions> bsos(maxLevels);
	for(int i=0;i<maxLevels;i++)
	{
	    bsos[i].setEfficiencyGoal(getEfficiencyGoalAtLevel(i));
	    bsos[i].setEfficiencyThreshold(getEfficiencyThresholdAtLevel(i)) ;
	    bsos[i].setMaximumNbOfCellsInPatch(getMaximumNbOfCellsInPatchAtLevel(i)) ;
	    bsos[i].setMinimumPatchLength(getMinimumPatchLengthAtLevel(i)) ;
	    bsos[i].setMaximumPatchLength(getMaximumPatchLengthAtLevel(i));
	}

	vector<const BoxSplittingOptions*> boxOptions(bsos.size());
	for(int i=0;i<maxLevels;i++)
		boxOptions[i]=&(bsos[i]);

	amr->createPatchesFromCriterionML(boxOptions,fd->getArray(),_coeficientsRefinement,1e-12);
	fd->decrRef();

	int numberOfGhostCells=genericSolver.getNumberOfGhostCells();
	_fields=MEDCouplingAMRAttribute::New(amr,fieldsInfos,numberOfGhostCells);
	_fields->alloc();

	int maxLevs=amr->getMaxNumberOfLevelsRelativeToThis();
	vector< vector<MEDCouplingCartesianAMRPatchGen*> > levs(maxLevs);
	vector< vector< MEDCouplingAutoRefCountObjectPtr <MEDCouplingCartesianAMRPatchGen> > > levsSafe(maxLevs);
	for (int i=0;i<maxLevs;i++)
	{
		vector<MEDCouplingCartesianAMRPatchGen *> tmpLev(amr->retrieveGridsAt(i));
		std::size_t sz(tmpLev.size());
		vector< MEDCouplingAutoRefCountObjectPtr <MEDCouplingCartesianAMRPatchGen> > tmpLevSafe(sz);
		for(std::size_t ii=0;ii<sz;ii++)
			tmpLevSafe[ii]=tmpLev[ii];
		levs[i]=tmpLev;
		levsSafe[i]=tmpLevSafe;
		for (size_t p=0;p<levs[i].size();p++)
		{
			MEDCouplingCartesianAMRMeshGen* patch=const_cast<MEDCouplingCartesianAMRMeshGen *>(levs[i][p]->getMesh());
			DataArrayDouble* yyWithGhost=const_cast<DataArrayDouble *>(_fields->getFieldOn(patch,fieldsInfos[0].first));
			for(int k=0;k<yyWithGhost->getNumberOfTuples();k++)
				yyWithGhost->setIJ(k,0,0.0);
			MEDCouplingFieldDouble* yy=genericSolver.initialConditions(patch->getImageMesh());

			MEDCouplingFieldDouble* YY3=AMR::buildFieldWithGhostFromFieldWithoutGhost(numberOfGhostCells,patch->getImageMesh(),yy);

			std::copy(YY3->getArray()->getPointer(),YY3->getArray()->getPointer()+YY3->getNumberOfTuples(),yyWithGhost->getPointer());
			YY3->decrRef();
			yy->decrRef();
			// CL PHYSIQUE //
			/*
			if (i==0)
			{
				MEDCouplingFieldDouble* fieldyyWithGhost=MEDCouplingFieldDouble::New(ON_CELLS);
				fieldyyWithGhost->setName("YY") ;
				MEDCouplingIMesh* m1=amr->getImageMesh()->buildWithGhost(numberOfCellsGhost);
				fieldyyWithGhost->setMesh(m1);
				fieldyyWithGhost->setArray(yyWithGhost);
				fieldyyWithGhost->checkCoherency();
				fieldyyWithGhost->setTime(0.,0,0);
				fillCellGhostPeriodic(bottom,left,top,right,bottomG,leftG,topG,rightG,amr1->getImageMesh(),fieldyyWithGhost,numberOfCellsGhost);
				fieldyyWithGhost->decrRef();
				m1->decrRef();
			}
			*/
			///////////////////////////
		}
	}
	_fields->synchronizeAllGhostZones();
}


void
AMR::PostTreatment(int it, double time, string nameOfField, string fileName, bool fromscratch) const
{
        cout << "PostProcessing - Iteration = " << it << " - time = " << time << endl;
        const MEDCouplingCartesianAMRMesh* amr=getFields()->getMyGodFather();
		string ret1=writeVTKAMRFieldOnRecurse(it,nameOfField,fileName);
		writePVD(fileName,ret1,time,fromscratch);
		int maxLevs=amr->getMaxNumberOfLevelsRelativeToThis();
		for (int i=0;i<maxLevs;i++)
		{
//            string rets=writeVTKAMRStructure(i,amr1,it,"Grid");
//            writePVD("Grids",rets,time,fromscratch);
//                string retf=writeVTKAMRField(i,amr1,att,"YY",it,"Fields");
//				writePVD("Fields",retf,time,fromscratch);
        }
}

void
AMR::compute(const GenericSolver& genericSolver)
{
	bool fromscratch=true;
	int iter=0;
	double currentTime=0.;
	int maximumNumberbOfIter=genericSolver.getMaximumNumberbOfIter();
	double finalTime=genericSolver.getFinalTime();
	int maxLevels=getMaximumLevels();
	vector<BoxSplittingOptions> bsos(maxLevels);
	for(int i=0;i<maxLevels;i++)
	{
	    bsos[i].setEfficiencyGoal(getEfficiencyGoalAtLevel(i));
	    bsos[i].setEfficiencyThreshold(getEfficiencyThresholdAtLevel(i)) ;
	    bsos[i].setMaximumNbOfCellsInPatch(getMaximumNbOfCellsInPatchAtLevel(i)) ;
	    bsos[i].setMinimumPatchLength(getMinimumPatchLengthAtLevel(i)) ;
	    bsos[i].setMaximumPatchLength(getMaximumPatchLengthAtLevel(i));
	}

	vector<const BoxSplittingOptions*> boxOptions(bsos.size());
	for(int i=0;i<maxLevels;i++)
		boxOptions[i]=&(bsos[i]);

	int frequencyOfPostTreatment=genericSolver.getFrequencyOfPostTreatment();

	string nameOfField=_fieldsInfos[0].first;
	string fileName="Field"+nameOfField;
	while (iter < maximumNumberbOfIter && currentTime <= finalTime)
	{
		if (iter%frequencyOfPostTreatment == 0)
		{
			PostTreatment(iter,currentTime,nameOfField,fileName,fromscratch);
			fromscratch=false;
		}
	    cout << "iteration = " << iter << ", time = " << currentTime << ", number of generated patches = " << _fields->getMyGodFather()->getNumberOfPatches() << endl;
	    double dtCoarse=unsteadyAMRDriver(currentTime,genericSolver);
	    currentTime+=dtCoarse;
	    iter+=1;
		if (iter%_frequencyOfRefinement == 0 && iter!=maximumNumberbOfIter)
			refinement(boxOptions,genericSolver);
	}
}

void
AMR::refinement(const vector<const BoxSplittingOptions*>& bsos,const GenericSolver& genericSolver)
{
    cout << "Refinement..." << endl;

    MEDCouplingCartesianAMRMesh* amr1=_fields->getMyGodFather();
    int numberOfCellsGhost=genericSolver.getNumberOfGhostCells();
	DataArrayDouble* yyWithGhost=const_cast<DataArrayDouble *>(_fields->getFieldOn(amr1,_fieldsInfos[0].first));

	MEDCouplingFieldDouble* yy=MEDCouplingFieldDouble::New(ON_CELLS);
	yy->setName("YY") ;
	MEDCouplingIMesh* m1=amr1->getImageMesh()->buildWithGhost(numberOfCellsGhost);
	yy->setMesh(const_cast<MEDCouplingIMesh *>(m1));
	m1->decrRef();
	yy->setArray(yyWithGhost);
	yy->setTime(0.,0,0);

	MEDCouplingFieldDouble* yy3=AMR::buildFieldWithoutGhostFromFieldWithGhost(numberOfCellsGhost,amr1->getImageMesh(),yy);

	MEDCouplingFieldDouble* fd=genericSolver.refinementCriterion(yy3);

	yy3->decrRef();
	yy->decrRef();

    MEDCouplingAutoRefCountObjectPtr<MEDCouplingCartesianAMRMesh> amr2=MEDCouplingCartesianAMRMesh::New(const_cast<MEDCouplingIMesh *>(amr1->getImageMesh()));
    amr2->createPatchesFromCriterionML(bsos,fd->getArray(),_coeficientsRefinement,1e-12);
    amr1=amr2;
	MEDCouplingAutoRefCountObjectPtr<MEDCouplingAMRAttribute> att2=_fields->projectTo(amr1);
	for(int i=0; i < amr1->getMaxNumberOfLevelsRelativeToThis()-1; i++)
        att2->synchronizeAllGhostZonesAtASpecifiedLevel(i+1);
	_fields=att2;
	fd->decrRef();
}

AMR::~AMR()
{
}

double
AMR::unsteadyAMRDriver(double currentTime, const GenericSolver& genericSolver)
{
	const MEDCouplingCartesianAMRMesh* amr1=_fields->getMyGodFather();
	double dtCoarse=1.E30;
	MEDCouplingCartesianAMRPatchGen* grid0=amr1->retrieveGridsAt(0)[0];
	int dirs=1;
	if(genericSolver.isAlternatingDirection())
		dirs=amr1->getSpaceDimension();

	for(int idir=0;idir<dirs;idir++)
	{
		dtCoarse=genericSolver.advancingTimeStep(idir,currentTime,_fields,grid0);
		_fields->synchronizeAllGhostZonesAtASpecifiedLevel(0);

		/* CL PHYSIQUE */
		/*
		int numberOfCellsGhost=genericSolver.getNumberOfGhostCells();
		MEDCouplingCartesianAMRMeshGen* patch=const_cast<MEDCouplingCartesianAMRMeshGen *>(grid0->getMesh());
		DataArrayDouble* yyWithGhost=const_cast<DataArrayDouble *>(_fields->getFieldOn(patch,"YY"));
		MEDCouplingFieldDouble* fieldyyWithGhost=MEDCouplingFieldDouble::New(ON_CELLS);
		fieldyyWithGhost->setName("YY") ;
		MEDCouplingIMesh* m1=amr1->getImageMesh()->buildWithGhost(numberOfCellsGhost);
		fieldyyWithGhost->setMesh(m1);
		fieldyyWithGhost->setArray(yyWithGhost);
		fieldyyWithGhost->setTime(0.,0,0);
		fieldyyWithGhost->checkCoherency();
		fillCellGhostPeriodic(bottom,left,top,right,bottomG,leftG,topG,rightG,amr1->getImageMesh(),fieldyyWithGhost,numberOfCellsGhost);
		fieldyyWithGhost->decrRef();
		m1->decrRef();
		*/
	}
	grid0->decrRef();
	if (amr1->getMaxNumberOfLevelsRelativeToThis()>1)
		unsteadyAMRDriverHigherLevels(1,currentTime,genericSolver);
	return dtCoarse;
}

void
AMR::unsteadyAMRDriverHigherLevels(int curLevel, double currentTime, const GenericSolver& genericSolver)
{
	const MEDCouplingCartesianAMRMesh* amr=_fields->getMyGodFather();
    int maxLevel=amr->getMaxNumberOfLevelsRelativeToThis();
    int nbTurn1=0;

	vector<MEDCouplingCartesianAMRPatchGen *> tmpLev(amr->retrieveGridsAt(curLevel));
	std::size_t sz(tmpLev.size());
	vector< MEDCouplingAutoRefCountObjectPtr <MEDCouplingCartesianAMRPatchGen> > tmpLevSafe(sz);
	for(std::size_t ii=0;ii<sz;ii++)
		tmpLevSafe[ii]=tmpLev[ii];

    if (tmpLev.size()>0)
    {
        vector<int> facts=tmpLev[0]->getMesh()->getFather()->getFactors();
        if (facts.size()!=0)
        	nbTurn1=*min_element(facts.begin(),facts.end()) ;
    }

	int dirs=1;
	bool rep=genericSolver.isAlternatingDirection();
	if (rep)
		dirs=amr->getSpaceDimension();

    for (int iTurn1=0;iTurn1<nbTurn1;iTurn1++)
    {
        for (int idir=0;idir<dirs;idir++)
        {
			for (size_t p=0;p<tmpLev.size();p++)
				double dt1=genericSolver.advancingTimeStep(idir,currentTime,_fields,tmpLev[p]);
			_fields->synchronizeAllGhostZonesAtASpecifiedLevelUsingOnlyFather(curLevel);
			_fields->synchronizeAllGhostZonesAtASpecifiedLevel(curLevel);
        }
		if (curLevel!=maxLevel-1)
		{
			unsteadyAMRDriverHigherLevels(curLevel+1,currentTime,genericSolver);
			if (iTurn1!=nbTurn1-1)
			{
				_fields->synchronizeAllGhostZonesAtASpecifiedLevelUsingOnlyFather(curLevel+1);
				_fields->synchronizeAllGhostZonesAtASpecifiedLevel(curLevel+1);
			}
		}
    }
    _fields->synchronizeFineToCoarseBetween(curLevel,curLevel-1);
}

MEDCouplingFieldDouble*
AMR::buildFieldWithGhostFromFieldWithoutGhost(int numberOfCellsGhost, const MEDCouplingIMesh* mesh,const MEDCouplingFieldDouble* fieldWithoutGhost)
{
    vector<int> cgs=mesh->getCellGridStructure();
	MEDCouplingIMesh* m1=mesh->buildWithGhost(numberOfCellsGhost);
    vector<int> cgsWG=m1->getCellGridStructure();
	DataArrayDouble *ret=DataArrayDouble::New();
	ret->alloc(m1->getNumberOfCells(),fieldWithoutGhost->getNumberOfComponents());
	double* vals=ret->getPointer();
	vector<int> fakeFactors(mesh->getSpaceDimension(),1);

	for (int j=0;j<ret->getNumberOfComponents()*ret->getNumberOfTuples();j++)
		vals[j]=0.;

	vector< std::pair<int,int> > cgs2(cgs.size());
	for (size_t i=0;i<cgs.size();i++)
	{
		cgs2[i].first=numberOfCellsGhost;
		cgs2[i].second=numberOfCellsGhost+cgs[i];
	}

	const DataArrayDouble* constArrWithoutGhost=fieldWithoutGhost->getArray();
	MEDCouplingIMesh::CondenseFineToCoarse(cgsWG,constArrWithoutGhost,cgs2,fakeFactors,ret);

    MEDCouplingFieldDouble* fieldWitGhost=MEDCouplingFieldDouble::New(ON_CELLS);
    fieldWitGhost->setName(fieldWithoutGhost->getName()) ;
    fieldWitGhost->setMesh(m1);
    fieldWitGhost->setArray(ret);
    fieldWitGhost->setTime(0.,0,0);
    m1->decrRef();
    ret->decrRef();
	return fieldWitGhost;
}

MEDCouplingFieldDouble*
AMR::buildFieldWithoutGhostFromFieldWithGhost(int numberOfCellsGhost,const MEDCouplingIMesh* meshWithoutGhost,const MEDCouplingFieldDouble* fieldWithGhost)
{
    vector<int> cgs=meshWithoutGhost->getCellGridStructure();
	MEDCouplingIMesh* m1=meshWithoutGhost->buildWithGhost(numberOfCellsGhost);
    vector<int> cgsWG=m1->getCellGridStructure();
    m1->decrRef();
	DataArrayDouble *ret=DataArrayDouble::New();
	ret->alloc(meshWithoutGhost->getNumberOfCells(),fieldWithGhost->getNumberOfComponents());
	for (int j=0;j<ret->getNumberOfComponents();j++)
		for (int i=0;i<ret->getNumberOfTuples();i++)
			ret->setIJ(i,j,0.);

	vector<int> fakeFactors(meshWithoutGhost->getSpaceDimension(),1);

	vector< std::pair<int,int> > cgs2(cgs.size());
	for (size_t i=0;i<cgs.size();i++)
	{
		cgs2[i].first=numberOfCellsGhost;
		cgs2[i].second=numberOfCellsGhost+cgs[i];
	}

	MEDCouplingIMesh::SpreadCoarseToFine(fieldWithGhost->getArray(),cgsWG,ret,cgs2,fakeFactors);

    MEDCouplingFieldDouble* fieldWithoutGhost=MEDCouplingFieldDouble::New(ON_CELLS);
    fieldWithoutGhost->setName(fieldWithGhost->getName()) ;
    fieldWithoutGhost->setMesh(const_cast<MEDCouplingIMesh *>(meshWithoutGhost));
    fieldWithoutGhost->setArray(ret);
    fieldWithoutGhost->setTime(0.,0,0);
    ret->decrRef();
    return fieldWithoutGhost;
}

string
AMR::writeVTKAMRFieldOnRecurse(int it, string nameOfField, string fileName) const
{
    ostringstream its;
    its << it;
	string fi=fileName+"_"+its.str();
    MEDCouplingFieldDouble* f=_fields->buildCellFieldOnRecurseWithoutOverlapWithoutGhost(const_cast<MEDCouplingCartesianAMRMesh *>(_fields->getMyGodFather()),nameOfField);
	f->mergeNodes(1.E-15);
	string ret=f->writeVTK(fi);
	f->decrRef();
	return ret;
}

void
AMR::writePVD(string fileName, string filevt, double time, bool fromScratch) const
{
    string filPvd=fileName+".pvd";
    ostringstream times;
    times << time;

    if (fromScratch)
    {
        ofstream file(filPvd.c_str()) ;
        if (file.is_open())
        {
            file << "<VTKFile type=\"Collection\" version=\"0.1\" byte_order=\"LittleEndian\"><Collection>\n";
            file << "<DataSet timestep=\""+times.str()+"\" group=\"\" part=\"0\" file=\""+filevt+"\"/>\n";
            file << "</Collection></VTKFile>\n";
            file.close();
        }
    }
    else
    {
		ifstream myReadFile;
		myReadFile.open(filPvd.c_str());
		int nlines=count(istreambuf_iterator<char>(myReadFile),
			  istreambuf_iterator<char>(), '\n');
		myReadFile.close();
		string line;
		string output="";
		myReadFile.open(filPvd.c_str());
		if (myReadFile.is_open())
		{
			int i=0;
			while ( getline (myReadFile,line) )
			{
				if (i!=0 && i<nlines-1)
				{
					output+=line;
					output+="\n";
				}
				i++;
			}
			myReadFile.close();
		}

		ofstream file(filPvd.c_str()) ;
        if (file)
        {
            file << "<VTKFile type=\"Collection\" version=\"0.1\" byte_order=\"LittleEndian\"><Collection>\n";
        	file << output ;
            file << "<DataSet timestep=\""+times.str()+"\" group=\"\" part=\"0\" file=\""+filevt+"\"/>\n";
            file << "</Collection></VTKFile>\n";
            file.close();
        }
    }
}