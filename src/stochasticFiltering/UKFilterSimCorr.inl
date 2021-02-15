/******************************************************************************
*                 SOFA, Simulation Open-Framework Architecture                *
*                    (c) 2006 INRIA, USTL, UJF, CNRS, MGH                     *
*                                                                             *
* This program is free software; you can redistribute it and/or modify it     *
* under the terms of the GNU General Public License as published by the Free  *
* Software Foundation; either version 2 of the License, or (at your option)   *
* any later version.                                                          *
*                                                                             *
* This program is distributed in the hope that it will be useful, but WITHOUT *
* ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or       *
* FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for    *
* more details.                                                               *
*                                                                             *
* You should have received a copy of the GNU General Public License along     *
* with this program. If not, see <http://www.gnu.org/licenses/>.              *
*******************************************************************************
* Authors: The SOFA Team and external contributors (see Authors.txt)          *
*                                                                             *
* Contact information: contact@sofa-framework.org                             *
******************************************************************************/
#pragma once

#include "UKFilterSimCorr.h"
#include <iostream>
#include <fstream>
#include <Eigen/src/Core/IO.h>


namespace sofa
{

namespace component
{

namespace stochastic
{


template <class FilterType>
UKFilterSimCorr<FilterType>::UKFilterSimCorr()
    : Inherit()
    , d_filename( initData(&d_filename, "filename", "output file name"))
    , outfile(NULL)
    , d_filenameState( initData(&d_filenameState, "filenameState", "output file name"))
    , d_filenameVar( initData(&d_filenameVar, "filenameVar", "output file name"))
    , d_filenameInn( initData(&d_filenameInn, "filenameInn", "output file name"))
    , d_state( initData(&d_state, "state", "actual expected value of reduced state (parameters) estimated by the filter" ) )
    , d_variance( initData(&d_variance, "variance", "actual variance  of reduced state (parameters) estimated by the filter" ) )
    , d_covariance( initData(&d_covariance, "covariance", "actual co-variance  of reduced state (parameters) estimated by the filter" ) )
    , d_innovation( initData(&d_innovation, "innovation", "innovation value computed by the filter" ) )    
{

}


template <class FilterType>
void UKFilterSimCorr<FilterType>::computePrediction()
{    
    //PRNS("Computing prediction, T= " << this->actualTime  << " ======");
    /// Computes background error variance Cholesky factorization.
    Eigen::LLT<EMatrixX> lltU(stateCovar);
    EMatrixX matPsqrt = lltU.matrixL();

    //PRNS("matPsqrt: " << matPsqrt);

    stateExp = masterStateWrapper->getState();
    //PRNS("X(n): \n" << stateExp.transpose());
    //PRNS("P(n): \n" << stateCovar);

    /// Computes X_{n}^{(i)-} sigma points        
    for (size_t i = 0; i < sigmaPointsNum; i++) {
        matXi.col(i) = stateExp + matPsqrt * matI.row(i).transpose();
    }
    //PRNS("MatXi: \n" << matXi);

    /// There's no dynamics => the state is not changed

    /// compute the expected value
    stateExp.fill(FilterType(0.0));
    for (size_t i = 0; i < sigmaPointsNum; i++)
        stateExp += matXi.col(i) * vecAlpha(i);
    // stateExp = stateExp * alpha;

    stateCovar.fill(FilterType(0.0));
    ////    EVectorX tmpX(stateSize);
    ////    tmpX.fill(FilterType(0.0));
    ////    for (size_t i = 0; i < sigmaPointsNum; i++) {
    ////       tmpX = matXi.col(i) - stateExp;
    ////       for (size_t x = 0; x < stateSize; x++)
    ////           for (size_t y = 0; y < stateSize; y++)
    ////               stateCovar(x,y) += vecAlphaVar(i) * tmpX(x)*tmpX(y);
    ////    }
    EMatrixX matXiTrans= matXi.transpose();
    EMatrixX centeredPxx = matXiTrans.rowwise() - matXiTrans.colwise().mean();
    EMatrixX weightedCenteredPxx = centeredPxx.array().colwise() * vecAlphaVar.array();
    EMatrixX covPxx = (centeredPxx.adjoint() * weightedCenteredPxx);
    //EMatrixX covPxx = (centeredPxx.adjoint() * centeredPxx) / double(centeredPxx.rows() )
    stateCovar = covPxx;
    // stateCovar = alphaVar*stateCovar;

    //PRNS("X(n+1)-: " << stateExp.transpose());
    //PRNS("P(n+1)-: \n" << stateCovar);
}

template <class FilterType>
void UKFilterSimCorr<FilterType>::computeCorrection()
{            
    if (observationManager->hasObservation(this->actualTime)) {
        //PRNS("======= Computing correction, T= " << this->actualTime << " ======");

        /// Compute variance factorization
        Eigen::LLT<EMatrixX> lltU(stateCovar);
        EMatrixX matPsqrt = lltU.matrixL();

        /// Computes X_{n}^{(i)-} sigma points
        for (size_t i = 0; i < sigmaPointsNum; i++) {
            matXi.col(i) = stateExp + matPsqrt * matI.row(i).transpose();         
        }
        //PRNS("MatXi: \n" << matXi);

        EVectorX xCol(stateSize), zCol(observationSize);
        int id;
        matZmodel.resize(observationSize,sigmaPointsNum);
        predObsExp.resize(observationSize);
        predObsExp.fill(FilterType(0.0));        
        stateExp.fill(FilterType(0.0));
        /// compute predicted observations
        for (size_t i = 0; i < sigmaPointsNum; i++) {
            xCol = matXi.col(i);            
            //PRNS("pX_n["<<i<<"]: "<< xCol.transpose());
            stateWrappers[0]->computeSimulationStep(xCol, mechParams, id);
            observationManager->getPredictedObservation(id,  zCol);
            //PRNS("pZ_n["<< i <<"]:" << zCol.transpose());
            matZmodel.col(i) = zCol;
            predObsExp = predObsExp + zCol * vecAlpha(i);
            stateExp = stateExp + xCol * vecAlpha(i);
        }
        //PRNS("Z: \n" << matZmodel);
        //predObsExp = alpha*predObsExp;
        //stateExp = alpha*stateExp;

        EMatrixX matPxz(stateSize, observationSize);
        EMatrixX matPz(observationSize, observationSize);
        matPxz.fill(FilterType(0.0));
        matPz.fill(FilterType(0.0));

        EMatrixX matXiTrans= matXi.transpose();
        EMatrixX matZItrans = matZmodel.transpose();
        EMatrixX centeredCx = matXiTrans.rowwise() - matXiTrans.colwise().mean();
        EMatrixX centeredCz = matZItrans.rowwise() - matZItrans.colwise().mean();
        EMatrixX weightedCenteredCz = centeredCz.array().colwise() * vecAlphaVar.array();
        EMatrixX covPxz = (centeredCx.adjoint() * weightedCenteredCz);
        matPxz=covPxz;
        EMatrixX covPzz = (centeredCz.adjoint() * weightedCenteredCz);
        matPz = obsCovar + covPzz;

        ////    EVectorX vx(stateSize), z(observationSize), vz(observationSize);
        ////    vx.fill(FilterType(0.0));
        ////    vz.fill(FilterType(0.0));
        ////    z.fill(FilterType(0.0));
        ////    for (size_t i = 0; i < sigmaPointsNum; i++) {
        ////       vx = matXi.col(i) - stateExp;
        ////       z = matZmodel.col(i);
        ////       vz = z - predObsExp;
        ////       for (size_t x = 0; x < stateSize; x++)
        ////           for (size_t y = 0; y < observationSize; y++)
        ////               matPxz(x,y) += vecAlphaVar(i) * vx(x)*vz(y);
        ////
        ////       for (size_t x = 0; x < observationSize; x++)
        ////           for (size_t y = 0; y < observationSize; y++)
        ////               matPz(x,y) += vecAlphaVar(i) * vz(x)*vz(y);
        ////    }
        //matPxz = alphaVar * matPxz;
        //matPz = alphaVar * matPz + obsCovar;
        ////    matPz = matPz + obsCovar;
        //PRNS("ObsCovar: " << obsCovar);

        EMatrixX matK(stateSize, observationSize);
        matK = matPxz * matPz.inverse();

        EVectorX innovation(observationSize);
        observationManager->getInnovation(this->actualTime, predObsExp, innovation);

        stateExp = stateExp + matK * innovation;
        stateCovar = stateCovar - matK*matPxz.transpose();

        stateWrappers[0]->computeSimulationStep(stateExp, mechParams, id);

        //PRNS("X(n+1)+: \n" << stateExp.transpose());
        //PRNS("P(n+1)+n: \n" << stateCovar);

        helper::WriteAccessor<Data <helper::vector<FilterType> > > stat = d_state;
        helper::WriteAccessor<Data <helper::vector<FilterType> > > var = d_variance;
        helper::WriteAccessor<Data <helper::vector<FilterType> > > covar = d_covariance;
        helper::WriteAccessor<Data <helper::vector<FilterType> > > innov = d_innovation;

        //stat.resize(stateSize);
        //var.resize(stateSize);
        //size_t numCovariances = (stateSize*(stateSize-1))/2;
        //covar.resize(numCovariances);
        //innov.resize(observationSize);

        //size_t gli = 0;
        //for (size_t i = 0; i < stateSize; i++) {
        //    stat[i] = stateExp[i];
        //    var[i] = stateCovar(i,i);
        //    for (size_t j = i+1; j < stateSize; j++) {
        //        covar[gli++] = stateCovar(i,j);
        //    }
        //}
        //for (size_t index = 0; index < observationSize; index++) {
        //    innov[index] = innovation[index];
        //}

        for (size_t i = 0; i < (size_t)stateCovar.rows(); i++) {
            diagStateCov(i) = stateCovar(i,i);
        }

        writeEstimationData(d_filenameState.getValue(), stateExp);
        writeEstimationData(d_filenameVar.getValue(), diagStateCov);
        writeEstimationData(d_filenameInn.getValue(), innovation);
    }
}

template <class FilterType>
void UKFilterSimCorr<FilterType>::init() {
    Inherit::init();
    assert(this->gnode);    

    this->gnode->template get<StochasticStateWrapperBaseT<FilterType> >(&stateWrappers, this->getTags(), sofa::core::objectmodel::BaseContext::SearchDown);
    PRNS("found " << stateWrappers.size() << " state wrappers");
    masterStateWrapper=NULL;
    size_t numSlaveWrappers = 0;
    size_t numMasterWrappers = 0;
    numThreads = 0;
    for (size_t i = 0; i < stateWrappers.size(); i++) {
        stateWrappers[i]->setFilterKind(SIMCORR);
        if (stateWrappers[i]->isSlave()) {            
            numSlaveWrappers++;
            PRNS("found stochastic state slave wrapper: " << stateWrappers[i]->getName());
        } else {
            masterStateWrapper = stateWrappers[i];            
            numMasterWrappers++;
            PRNS("found stochastic state master wrapper: " << masterStateWrapper->getName());
        }
    }

    if (numMasterWrappers != 1) {
        PRNE("Wrong number of master wrappers: " << numMasterWrappers);
        return;
    }

    if (numThreads > 0) {
        PRNS("number of slave wrappers: " << numThreads-1);
        /// slaves + master
    }
    numThreads=numSlaveWrappers+numMasterWrappers;

    this->gnode->get(observationManager, core::objectmodel::BaseContext::SearchDown);
    if (observationManager) {
        PRNS("found observation manager: " << observationManager->getName());
    } else
        PRNE("no observation manager found!");

    const std::string& filename = d_filename.getFullPath();
    outfile = new std::ofstream(filename.c_str());

    this->saveParam = false;
    if (!d_filenameState.getValue().empty()) {
        std::ofstream paramFileState(d_filenameState.getValue().c_str());
        if (paramFileState .is_open()) {
            this->saveParam = true;
            paramFileState.close();
        }
    }
    if (!d_filenameVar.getValue().empty()) {
        std::ofstream paramFile(d_filenameVar.getValue().c_str());
        if (paramFile.is_open()) {
            this->saveParam = true;
            paramFile.close();
        }
    }
    if (!d_filenameInn.getValue().empty()) {
        std::ofstream paramFileInn(d_filenameInn.getValue().c_str());
        if (paramFileInn .is_open()) {
            this->saveParam = true;
            paramFileInn.close();
        }
    }
}

template <class FilterType>
void UKFilterSimCorr<FilterType>::bwdInit() {
    assert(masterStateWrapper);

    stateSize = masterStateWrapper->getStateSize();
    PRNS("StateSize " << stateSize);

    /// Initialize Observation's data
    if (!initialiseObservationsAtFirstStep.getValue()) {
        observationSize = this->observationManager->getObservationSize();
        PRNS("Observation size: " << observationSize);
        if (observationSize == 0) {
            PRNE("No observations available, cannot allocate the structures!");
        }
        obsCovar = observationManager->getErrorVariance();
    }

    /// Initialize Model state expected value and covariance matrix
    stateExp = masterStateWrapper->getState();
    stateCovar = masterStateWrapper->getStateErrorVariance();

    diagStateCov.resize(stateCovar.rows());
    for (size_t i = 0; i < (size_t)stateCovar.rows(); i++) {
        diagStateCov(i)=stateCovar(i,i);
    }

    /// compute sigma points
    switch (this->m_sigmaTopology) {
    case STAR:
        computeStarSigmaPoints(matI);
        break;
    case SIMPLEX:
    default:
        computeSimplexSigmaPoints(matI);
    }
    sigmaPointsNum = matI.rows();
    matXi.resize(stateSize, sigmaPointsNum);
    matXi.setZero();    

    /// copy state (exp, covariance) to SOFA data for exporting
    helper::WriteAccessor<Data <helper::vector<FilterType> > > dstate = this->d_state;
    helper::WriteAccessor<Data <helper::vector<FilterType> > > dvar = this->d_variance;
    helper::WriteAccessor<Data <helper::vector<FilterType> > > dcovar = this->d_covariance;

    //dstate.resize(stateSize);
    //dvar.resize(stateSize);
    //size_t numCovar = (stateSize*(stateSize-1))/2;
    //dcovar.resize(numCovar);

    //size_t gli = 0;
    //for (size_t i = 0; i < stateSize; i++) {
    //    dstate[i] = stateExp(i);
    //    dvar[i] = stateCovar(i);
    //    for (size_t j = i+1; j < stateSize; j++) {
    //        dcovar[gli++] = stateCovar(i,j);
    //    }
    //}
}

template <class FilterType>
void UKFilterSimCorr<FilterType>::initializeStep(const core::ExecParams* _params, const size_t _step) {
    Inherit::initializeStep(_params, _step);

    /// Initialize Observation's data
    if (initialiseObservationsAtFirstStep.getValue()) {
        observationSize = this->observationManager->getObservationSize();
        PRNS("Observation size: " << observationSize);
        if (observationSize == 0) {
            PRNE("No observations available, cannot allocate the structures!");
        }
        obsCovar = observationManager->getErrorVariance();
        initialiseObservationsAtFirstStep.setValue(false);
    }

    for (size_t i = 0; i < stateWrappers.size(); i++)
        stateWrappers[i]->initializeStep(stepNumber);

    observationManager->initializeStep(stepNumber);
}

template <class FilterType>
void UKFilterSimCorr<FilterType>::computeSimplexSigmaPoints(EMatrixX& sigmaMat) {
    size_t p = stateSize;
    size_t r = stateSize + 1;

    EMatrixX workingMatrix = EMatrixX::Zero(p, r);

    Type scal, beta, sqrt_p;
    beta = Type(p) / Type(p+1);
    sqrt_p = sqrt(Type(p));
    scal = Type(1.0)/sqrt(Type(2) * beta);
    workingMatrix(0,0) = -scal;
    workingMatrix(0,1) = scal;

    for (size_t i = 1; i < p; i++) {
        scal = Type(1.0) / sqrt(beta * Type(i+1) * Type(i+2));

        for (size_t j = 0; j < i+1; j++)
            workingMatrix(i,j) = scal;
        workingMatrix(i,i+1) = -Type(i+1) * scal;
    }


    sigmaMat.resize(r,p);
    for (size_t i = 0; i < r; i++)
        sigmaMat.row(i) = workingMatrix.col(i) * sqrt_p;

    vecAlpha.resize(r);
    vecAlpha.fill(Type(1.0)/Type(r));
    alphaConstant = true;
    alpha = vecAlpha(0);

    alphaVar = (this->useUnbiasedVariance.getValue()) ? Type(1.0)/Type(r-1) : Type(1.0)/Type(r);
    vecAlphaVar.resize(r);
    vecAlphaVar.fill(alphaVar);
    //PRNS("sigmaMat: \n" << sigmaMat);
    //PRNS("vecAlphaVar: \n" << vecAlphaVar);
}


template <class FilterType>
void UKFilterSimCorr<FilterType>::computeStarSigmaPoints(EMatrixX& sigmaMat) {
    size_t p = stateSize;
    size_t r = 2 * stateSize + 1;

    EMatrixX workingMatrix = EMatrixX::Zero(p, r);

    Type lambda, sqrt_vec;
    lambda = this->lambdaScale.getValue();
    PRNS("lambda scale equals: " << lambda);
    sqrt_vec = sqrt(p + lambda);

    for (size_t j = 0; j < p; j++) {
        workingMatrix(j,j) = sqrt_vec;
    }
    for (size_t j = p; j < 2 * p; j++) {
        workingMatrix(2*p-j-1,j) = -sqrt_vec;
    }

    sigmaMat.resize(r,p);
    for (size_t i = 0; i < r; i++)
        sigmaMat.row(i) = workingMatrix.col(i);

    vecAlpha.resize(r);
    vecAlpha.fill(Type(1.0)/Type(2 * (p + lambda)));
    vecAlpha(2 * p) = Type(lambda) / Type(p + lambda);
    alphaConstant = false;
    alpha = vecAlpha(0);

    alphaVar = (this->useUnbiasedVariance.getValue()) ? Type(1.0)/Type(2 * (p + lambda) - 1) : Type(1.0)/Type(2 * (p + lambda));
    vecAlphaVar.resize(r);
    vecAlphaVar.fill(alphaVar);
    vecAlphaVar(2 * p) = Type(lambda) / Type(p + lambda);
    //PRNS("sigmaMat: \n" << sigmaMat);
    //PRNS("vecAlphaVar: \n" << vecAlphaVar);
}



template <class FilterType>
void UKFilterSimCorr<FilterType>::writeEstimationData(std::string filename, EVectorX& data){
    if (this->saveParam) {
        Eigen::IOFormat CommaInitFmt(Eigen::StreamPrecision, Eigen::DontAlignCols, " ", " ");
        std::ofstream paramFile(filename.c_str(), std::ios::app);
        if (paramFile.is_open()) {
            paramFile << std::setprecision(15) << data.transpose().format(CommaInitFmt) << "\n";
            paramFile.close();
        }
    }
}


} // stochastic

} // component

} // sofa

