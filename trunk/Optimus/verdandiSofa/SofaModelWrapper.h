/******************************************************************************
*       SOFA, Simulation Open-Framework Architecture, version 1.0 RC 1        *
*                (c) 2006-2011 MGH, INRIA, USTL, UJF, CNRS                    *
*                                                                             *
* This library is free software; you can redistribute it and/or modify it     *
* under the terms of the GNU Lesser General Public License as published by    *
* the Free Software Foundation; either version 2.1 of the License, or (at     *
* your option) any later version.                                             *
*                                                                             *
* This library is distributed in the hope that it will be useful, but WITHOUT *
* ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or       *
* FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License *
* for more details.                                                           *
*                                                                             *
* You should have received a copy of the GNU Lesser General Public License    *
* along with this library; if not, write to the Free Software Foundation,     *
* Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301 USA.          *
*******************************************************************************
*                               SOFA :: Modules                               *
*                                                                             *
* Authors: The SOFA Team and external contributors (see Authors.txt)          *
*                                                                             *
* Contact information: contact@sofa-framework.org                             *
******************************************************************************/
#ifndef SOFA_SIMULATION_SOFA_MODEL_WRAPPER_H
#define SOFA_SIMULATION_SOFA_MODEL_WRAPPER_H

#include <sofa/core/objectmodel/BaseObject.h>
#include <sofa/core/behavior/BaseAnimationLoop.h>
#include <sofa/core/ExecParams.h>
#include <sofa/simulation/common/common.h>
#include <sofa/simulation/common/Node.h>
#include <sofa/helper/AdvancedTimer.h>
#include <sofa/simulation/common/Simulation.h>

#include <sofa/simulation/common/CollisionAnimationLoop.h>
#include <sofa/component/constraintset/LCPConstraintSolver.h>
#include <sofa/component/component.h>

#include "../src/OptimParams.h"

#include "VerdandiClasses.h"
#include "sofa/component/projectiveconstraintset/FixedConstraint.h"
#include "ObservationSource.h"
#include "SimulatedStateObservationSource.h"

#include "sofa/core/Mapping.h"
#include "sofa/component/topology/TriangleSetTopologyContainer.h"

#include <src/PointProjection.h>


using namespace sofa::core::objectmodel;
using namespace sofa::core::behavior;

namespace sofa
{

namespace simulation
{

/**
 *  \brief Wrapper class implementing an interface between SOFA and Verdandi
 */

enum FilterType { UNDEF, FORWARD, UKF, ROUKF };

template <class Model, class ObservationManager >
class SOFA_SIMULATION_COMMON_API SofaReducedOrderUKF : public Verdandi::ReducedOrderUnscentedKalmanFilter<Model, ObservationManager>, public sofa::core::objectmodel::BaseObject
{
protected:    
    bool positionInState, velocityInState;

public:            
    typedef typename Verdandi::ReducedOrderUnscentedKalmanFilter<Model, ObservationManager> Inherit1;
    typedef typename sofa::core::objectmodel::BaseObject Inherit2;

    Data<std::string> m_outputDirectory, m_configFile, m_sigmaPointType, m_observationErrorVariance;
    Data<bool> m_saveVQ, m_showIteration, m_showTime, m_analyzeFirstStep, m_withResampling;
    Data<bool> m_positionInState, m_velocityInState;
    Data<int> m_transformParams;

    SofaReducedOrderUKF();

    void init();

    void InitializeFilter();
    void InitializeParams();
    void InitializeStructures();
};


template <class Model, class ObservationManager >
class SOFA_SIMULATION_COMMON_API SofaUnscentedKalmanFilter : public Verdandi::UnscentedKalmanFilter<Model, ObservationManager>, public sofa::core::objectmodel::BaseObject
{
public:
};


template <class Model>
class SOFA_SIMULATION_COMMON_API SofaForwardDriver: public Verdandi::ForwardDriver<Model>, public sofa::core::objectmodel::BaseObject
{
public:
};


template <class Type>
class SOFA_SIMULATION_COMMON_API SofaModelWrapper : public Verdandi::VerdandiBase, public sofa::core::objectmodel::BaseObject
{
public:
    SOFA_CLASS(SOFA_TEMPLATE(SofaModelWrapper,Type),sofa::core::objectmodel::BaseObject);
    /// SOFA TYPES
    typedef sofa::core::objectmodel::BaseObject Inherit;

    typedef sofa::component::container::OptimParams<sofa::helper::vector<Type> > OPVector;


    /// VERDANDI TYPES

    //! The numerical type (e.g., double).
    typedef Type value_type;
    //! Pointer to the numerical type.
    typedef Type* pointer;
    //! Const pointer to the numerical type.
    typedef const Type* const_pointer;
    //! Reference to the numerical type.
    typedef Type& reference;
    //! Const reference to the numerical type.
    typedef const Type& const_reference;
#ifdef VERDANDI_SPARSE
    //! Type of the background error covariance matrix.
    typedef Seldon::Matrix<Type, General, RowSparse> state_error_variance;
    //! \brief Type of the reduced matrix \f$U\f$ in the \f$LUL^T\f$
    //  decomposition of the background error covariance matrix.
    typedef Seldon::Matrix<Type, General, RowSparse> state_error_variance_reduced;
    //! Type of a row of the background error variance.
    typedef Seldon::Vector<Type> state_error_variance_row;
    //! Type of the model/observation crossed matrix.
    typedef Seldon::Matrix<Type, General, RowSparse> matrix_state_observation;
    //! Type of the tangent linear operator.
    typedef Seldon::Matrix<Type, General, RowSparse> tangent_linear_operator;
#else
    //! Type of the background error covariance matrix.
    typedef Seldon::Matrix<Type> state_error_variance;
    // ! \brief Type of the reduced matrix \f$U\f$ in the \f$LUL^T\f$
    //  decomposition of the background error covariance matrix.
    typedef Seldon::Matrix<Type> state_error_variance_reduced;
    //! Type of a row of the background error variance.
    typedef Seldon::Vector<Type> state_error_variance_row;
    //! Type of the model/observation crossed matrix.
    typedef Seldon::Matrix<Type> matrix_state_observation;
    //! Type of the tangent linear operator.
    typedef Seldon::Matrix<Type> tangent_linear_operator;
#endif
    //! Type of the model state vector.
    typedef Seldon::Vector<Type> state;
    //! Collection of vector state.
    typedef Seldon::Vector<state, Seldon::Collection> state_collection;

    typedef core::behavior::MechanicalState<defaulttype::Vec3dTypes> MechStateVec3d;
    typedef component::projectiveconstraintset::FixedConstraint<defaulttype::Vec3dTypes> FixedConstraintVec3d;

    typedef core::behavior::MechanicalState<defaulttype::Rigid3dTypes> MechStateRigid3d;
    typedef component::projectiveconstraintset::FixedConstraint<defaulttype::Rigid3dTypes> FixedConstraintRigid3d;

    typedef struct {
        simulation::Node* gnode;

        FilterType filterType;
        int transformParams;   /// 0: do nothing, 1: take absolute value, 2: quadratic (not implemented)
        bool positionInState;
        bool velocityInState;
        double errorVarianceSofaState;
        bool verbose;
    } ModelData;

    typedef std::pair<OPVector*,helper::vector<size_t> > OPVecInd;

    typedef struct {
        simulation::Node* node;
        helper::vector<OPVecInd> oparams;
        MechStateVec3d* vecMS;
        MechStateRigid3d* rigidMS;
        FixedConstraintVec3d* vecFC;
        FixedConstraintRigid3d* rigidFC;
        helper::vector<std::pair<size_t, size_t> > positionPairs;
        helper::vector<std::pair<size_t, size_t> > velocityPairs;
    } SofaObject;

public:
    const core::ExecParams* execParams;
    int numStep;

    int current_row_;
    size_t dim_;
    size_t state_size_;
    size_t reduced_state_size_;
    size_t reduced_state_index_;
    size_t applyOpNum;

    helper::vector<size_t> listStateBegin;
    helper::vector<size_t> listStateMiddle;
    helper::vector<size_t> listStateEnd;
    helper::vector<size_t> listParamBegin;
    helper::vector<size_t> listParamEnd;

    state state_, duplicated_state_;

    Data<bool> displayTime;
    Data<bool> m_solveVelocityConstraintFirst;

    ///// OLD:
    //size_t free_nodes_size;
    //OPVector* vecParams;
    //MechStateVec3d * mechanicalObject;
    //FixedConstraintVec3d* fixedConstraints;
    //helper::vector<size_t> freeIndices;
    ////


    ///LATEST:
    helper::vector<SofaObject> sofaObjects;

    //OPVector* vecParams;
    //MechStateVec3d * mechanicalObject;
    //FixedConstraintVec3d* fixedConstraints;

    //helper::vector<OPVector*> listOP3d;

    //helper::vector<MechStateVec3d*> listMS3d;
    //helper::vector<FixedConstraintVec3d*> listFC3d;

    //helper::vector<MechStateRigid3d*> listRigidMS;
    //helper::vector<FixedConstraintRigid3d*> listRigidFC;

    //size_t free_nodes_size;
    //helper::vector<helper::vector<size_t> > listFreeIndices;

    //bool positionInState, velocityInState, verbose;

    /// error variance
    //double state_error_variance_state_, state_error_variance_params_;

    //! Background error covariance matrix (B).
    state_error_variance state_error_variance_;
    //! Inverse of the background error covariance matrix (B^-1).
    state_error_variance state_error_variance_inverse_;
    //! Value of the row of B currently stored.
    state_error_variance_row state_error_variance_row_;

    //! \brief Projector matrix L in the decomposition of the
    //  background error covariance matrix (\f$B\f$) as a product LUL^T
    state_error_variance state_error_variance_projector_;
    //! \brief Reduced matrix U in the decomposition of the
    //  background error covariance matrix (\f$B\f$) as a product LUL^T
    state_error_variance_reduced state_error_variance_reduced_;
    //! Is state error variance projector allocated?
    bool variance_projector_allocated_;
    //! Is reduced state error variance allocated?
    bool variance_reduced_allocated_;



    double time_;

    ModelData modelData;

    sofa::core::behavior::ConstraintSolver *constraintSolver;


public:
    SofaModelWrapper();
    virtual ~SofaModelWrapper();

    void setInitStepData(const core::ExecParams* _execParams) {
        execParams = _execParams;
    }

    void initSimuData(ModelData& _md);

    SofaObject* getObject(MechStateVec3d *_state);
    void SetSofaVectorFromVerdandiState(defaulttype::Vec3dTypes::VecCoord & vec, const state& _state, SofaObject *obj);

    /// functions propagating state between SOFA and Verdandi

    void StateSofa2Verdandi();
    void StateVerdandi2Sofa();

    /// functions required by Verdandi API:

    int GetNstate() const { return state_size_; }
    double GetTime() { return time_; }
    state& GetState();
    void GetStateCopy(state& _copy);

    void Initialize(std::string &) {
        Initialize();
    }

    void Initialize();
    void InitializeStep() { numStep++; time_ = numStep*modelData.gnode->getDt(); applyOpNum = 0; }
    void Finalize() {}
    void FinalizeStep();
    bool HasFinished() { return(false); }
    void StateUpdated();
    void SetTime(double _time);

    double ApplyOperator(state& _x, bool _preserve_state = true, bool _update_force = true);
    void Forward(bool _update_force = true, bool _update_time = true);

    void StepDefault(bool _update_force, bool _update_time);
    void StepFreeMotion(bool _update_force, bool _update_time);

    void Message(string _message);
    void Verb(string _s) {
        if (modelData.verbose)
            std::cout << "[" << this->getName() << "]: " << _s << std::endl;
    }

    void computeCollision();

    state_error_variance& GetStateErrorVariance();
    state_error_variance_row& GetStateErrorVarianceRow(int row);
    state_error_variance& GetStateErrorVarianceProjector();
    state_error_variance_reduced& GetStateErrorVarianceReduced();


    /// Construction method called by ObjectFactory.
    //template<class T>
    //static typename T::SPtr create(T*, BaseContext* context, BaseObjectDescription* arg)
    //{
    //    simulation::Node* gnode = dynamic_cast<simulation::Node*>(context);
    //    typename T::SPtr obj = sofa::core::objectmodel::New<T>(gnode);
    //    if (context) context->addObject(obj);
    //    if (arg) obj->parse(arg);
    //    return obj;
    //}

    void printMatrix(Seldon::Matrix<Type>& M, std::ostream &of) {
        for (int i = 0; i < M.GetM(); i++)
          {
            for (int j = 0; j < M.GetN(); j++)
              of << M(i, j) << '\t';
            of << '\n';
          }
    }

    void printMatrixInRow(Seldon::Matrix<Type>& M, std::ostream &of) {
        for (int i = 0; i < M.GetM(); i++)
          {
            for (int j = 0; j < M.GetN(); j++)
              of << M(i, j) << '\t';
          }
        of << '\n';
    }

    void printVector(Seldon::Vector<Type>& V, std::ofstream &of) {
        for (int i = 0; i < V.GetSize(); i++)
            of << V(i) << '\n';
    }
};


/*class SOFA_SIMULATION_COMMON_API SofaObservationManagerBase : public Verdandi::VerdandiBase, public sofa::core::objectmodel::BaseObject
{
public:
    typedef Seldon::Matrix<double> error_variance;
    typedef Seldon::Vector<double> observation;
    typedef Seldon::Vector<double> state;
    typedef SofaModelWrapper<double> model;
    typedef Seldon::Matrix<double> tangent_linear_operator;
    typedef Seldon::Vector<double> tangent_linear_operator_row;


    virtual void DiscardObservation(bool _discard_observation) {} // = 0;

    virtual error_variance& GetErrorVariance() const {} //  = 0;
    virtual error_variance& GetErrorVarianceInverse() const {} //  = 0;

    virtual observation& GetInnovation(const state& _x) {} // = 0;

    virtual int GetNobservation() const {} //  = 0;

    virtual bool HasObservation() const {} //  = 0;
    virtual bool HasObservation(double time) {} //  = 0;

    virtual void Initialize(model& _model, std::string configuration_file) {} // = 0;

    virtual void SetTime(model& _model, double time) {} // = 0;
    virtual void SetTime(double time) {} //  = 0;
};*/


template <class T>
class SOFA_SIMULATION_COMMON_API SofaLinearObservationManager : public Verdandi::LinearObservationManager<T>, public sofa::core::objectmodel::BaseObject
{
public:            
    typedef typename Verdandi::LinearObservationManager<T> Inherit1;

    Data<double> m_errorVariance;

    SofaLinearObservationManager()
        : m_errorVariance( initData(&m_errorVariance, double(1.0), "errorVariance", "observation error variance") )
    {
    }


    virtual typename Inherit1::observation& GetInnovation(const typename SofaModelWrapper<double>::state& x) {
        return Inherit1::GetInnovation(x);
    }

    virtual void SetTime(SofaModelWrapper<double>& model, double time) {
        return Inherit1::SetTime(model, time);
    }

    virtual int GetNobservation() {
        return Inherit1::GetNobservation();
    }

    virtual const typename Inherit1::error_variance& GetErrorVariance() {
        return Inherit1::GetErrorVariance();
    }

    virtual const typename Inherit1::error_variance& GetErrorVarianceInverse() {
        return Inherit1::GetErrorVarianceInverse();
    }

    virtual void Initialize(SofaModelWrapper<double>& model, std::string confFile) {
        Verb("initialize sofaLinObsManager");
        //Inherit1::Initialize(model, confFile);

        return;
    }

    void Verb(string _s) {
        //if (modelData.verbose)
            std::cout << "[" << this->getName() << "]: " << _s << std::endl;
    }

};


template <class DataTypes1, class DataTypes2>
class SOFA_SIMULATION_COMMON_API MappedPointsObservationManager : public SofaLinearObservationManager<double>
{
public:
    typedef typename DataTypes1::Real Real1;
    typedef SofaLinearObservationManager<double> Inherit;
    typedef core::behavior::MechanicalState<DataTypes1> MasterState;
    typedef core::behavior::MechanicalState<DataTypes2> MappedState;
    typedef sofa::core::Mapping<DataTypes1, DataTypes2> Mapping;
    typedef sofa::component::container::SimulatedStateObservationSource<DataTypes1> ObservationSource;

protected:

    SofaModelWrapper<Real1>* sofaModel;
    typename SofaModelWrapper<Real1>::SofaObject* sofaObject;
    size_t masterStateSize;
    size_t mappedStateSize;


public:

    MappedPointsObservationManager()
        : Inherit() {
    }

    Mapping* mapping;
    MappedState* mappedState;
    MasterState* masterState;
    ObservationSource *observationSource;

    void init();
    void bwdInit();

    virtual typename Inherit::observation& GetInnovation(const typename SofaModelWrapper<double>::state& x);

    virtual void SetTime(SofaModelWrapper<double>& model, double time) {
        std::cout << "Setting time: " << time << std::endl;
        return Inherit::SetTime(model, time);
    }

    virtual int GetNobservation() {
        return this->Nobservation_;
    }

    virtual const typename Inherit::error_variance& GetErrorVariance() {        
        return Inherit::GetErrorVariance();
    }

    virtual const typename Inherit::error_variance& GetErrorVarianceInverse() {        
        return Inherit::GetErrorVarianceInverse();
    }

    virtual void Initialize(SofaModelWrapper<double>& /*model*/, std::string /*confFile*/);

};



template <class DataTypes1, class DataTypes2>
class SOFA_SIMULATION_COMMON_API ARObservationManager : public SofaLinearObservationManager<double>
{
public:
    SOFA_CLASS(SOFA_TEMPLATE2(ARObservationManager, DataTypes1, DataTypes2), SOFA_TEMPLATE(SofaLinearObservationManager, double));

    typedef typename DataTypes1::Real Real1;
    typedef typename DataTypes1::VecCoord VecCoord1;
    typedef SofaLinearObservationManager<double> Inherit;
    typedef core::behavior::MechanicalState<DataTypes1> MasterState;
    typedef core::behavior::MechanicalState<DataTypes2> MappedState;
    typedef sofa::core::Mapping<DataTypes1, DataTypes2> Mapping;
    typedef sofa::component::container::SimulatedStateObservationSource<DataTypes1> ObservationSource;

    typedef sofa::component::topology::TriangleSetTopologyContainer TriangleContainer;

protected:
    SingleLink<ARObservationManager<DataTypes1,DataTypes2>, TriangleContainer, BaseLink::FLAG_STOREPATH|BaseLink::FLAG_STRONGLINK> triangleContainerLink;
    SingleLink<ARObservationManager<DataTypes1,DataTypes2>, MechanicalState<DataTypes1>, BaseLink::FLAG_STOREPATH|BaseLink::FLAG_STRONGLINK> triangleMStateLink;


    SofaModelWrapper<Real1>* sofaModel;
    typename SofaModelWrapper<Real1>::SofaObject* sofaObject;
    size_t masterStateSize;
    size_t mappedStateSize;

    TriangleContainer* triangleContainer;
    PointProjection<Real1>* pointProjection;
    MechanicalState<DataTypes1>* localMState;
    MechanicalState<DataTypes1>* triangleMState;



    //SingleLink<ARObservationManager<DataTypes1, DataTypes2>, TriangleContainer, BaseLink::FLAG_STOREPATH|BaseLink::FLAG_STRONGLINK> triangleTopologyContainer;


public:

    TriangleContainer* triaCont;
    ARObservationManager()
        : Inherit()
        , triangleContainerLink(initLink("triangleTopologyContainer", "link to the triangle topology needed for point projection"), triangleContainer)
        , triangleMStateLink(initLink("triangleMState", "link to the mstate of the surface needed for point projection"), triangleMState)
    {
    }

    Mapping* mapping;
    MappedState* mappedState;
    MasterState* masterState;
    ObservationSource *observationSource;

    void init() {
        this->getContext()->get(localMState);

        if (localMState == NULL)
            std::cerr << "[" << this->getName() << "]: ERROR: cannot find the local mechanical state" << std::endl;
        else
            std::cout << "[" << this->getName() << "]: the local mechanical state name: " << localMState->getName() << std::endl;

        triangleMState = triangleMStateLink.get();
        if (triangleMState == NULL)
            std::cerr << "[" << this->getName() << "]: ERROR: cannot find the triangle mechanical state" << std::endl;
        else
            std::cout << "[" << this->getName() << "]: the triangle  mechanical state name: " << triangleMState->getName() << std::endl;

        triangleContainer = triangleContainerLink.get();
        if (triangleContainer == NULL)
            std::cerr << "[" << this->getName() << "]: ERROR: cannot find the triangle container" << std::endl;
        else
            std::cout << "[" << this->getName() << "]: the triangle  container name: " << triangleContainer->getName() << std::endl;
    }


    void bwdInit() {
        pointProjection = new PointProjection<Real1>(*triangleContainer);

        /// project initially all the points from triangle mstate to the surface and store in the local m-state!

        //const VecCoord1& pX = triangleMState->getX();
        //const VecCoord1& pX = triangleMState->getX();


    }


    virtual void SetTime(SofaModelWrapper<double>& model, double time) {
        std::cout << "Setting time: " << time << std::endl;
        return Inherit::SetTime(model, time);
    }

    virtual int GetNobservation() {
        return this->Nobservation_;
    }

    virtual const typename Inherit::error_variance& GetErrorVariance() {
        return Inherit::GetErrorVariance();
    }

    virtual const typename Inherit::error_variance& GetErrorVarianceInverse() {
        return Inherit::GetErrorVarianceInverse();
    }

    virtual void Initialize(SofaModelWrapper<double>& /*model*/, std::string /*confFile*/) {}

    virtual typename Inherit::observation& GetInnovation(const typename SofaModelWrapper<double>::state& x) {}

};




} // namespace simulation

} // namespace sofa

#endif  /* SOFA_SIMULATION_SOFA_MODEL_WRAPPER_H */
