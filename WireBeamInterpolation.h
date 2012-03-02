/******************************************************************************
 *       SOFA, Simulation Open-Framework Architecture, version 1.0 beta 4      *
 *                (c) 2006-2009 MGH, INRIA, USTL, UJF, CNRS                    *
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
//
// C++ Implementation : WireBeamInterpolation / AdaptiveBeamForceFieldAndMass
//
// Description:
//
//
// Author: Christian Duriez, INRIA
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef SOFA_COMPONENT_FEM_WIREBEAMINTERPOLATION_H
#define SOFA_COMPONENT_FEM_WIREBEAMINTERPOLATION_H


#include "initBeamAdapter.h"
#include "WireRestShape.h"
#include "BeamInterpolation.h"
#include <sofa/core/behavior/ForceField.h>
#include <sofa/core/behavior/Mass.h>
#include <sofa/core/objectmodel/Data.h>
#include <sofa/defaulttype/SolidTypes.h>
#include <sofa/core/topology/BaseMeshTopology.h>

#include <sofa/helper/vector.h>
#include <sofa/defaulttype/Vec.h>
#include <sofa/defaulttype/Mat.h>

#include <sofa/core/objectmodel/BaseObject.h>


namespace sofa
{

namespace component
{

namespace fem
{
using sofa::helper::vector;
using namespace sofa::core::topology;
using namespace sofa::component::engine;



/** Compute Finite Element elastic force and mass based on Adaptive 6D beam elements.
  - Adaptive beam interpolation
  - Adaptive Force and Mass computation
  - Adaptive Mapping

  TODO : put in a separate class what is specific to wire shape !
 */


/// AdaptiveBeam Interpolation provides the basis of the Beam computation
/// As the computation is adaptive, the interpolation can be modified at each time step.
template<class DataTypes>
class SOFA_BEAMADAPTER_API WireBeamInterpolation : public virtual sofa::component::fem::BeamInterpolation<DataTypes>
{
public:

	SOFA_CLASS( SOFA_TEMPLATE(WireBeamInterpolation              , DataTypes) ,
			SOFA_TEMPLATE(sofa::component::fem::BeamInterpolation, DataTypes) );

	typedef sofa::component::fem::BeamInterpolation<DataTypes> Inherited;

	typedef typename Inherited::VecCoord VecCoord;
	typedef typename Inherited::VecDeriv VecDeriv;
	typedef typename Inherited::VecReal VecReal;
	typedef typename Inherited::Coord Coord;
	typedef typename Inherited::Deriv Deriv;

	typedef typename Inherited::Real Real;
	typedef typename Inherited::Index Index;
	typedef typename Inherited::ElementID ElementID;
	typedef typename Inherited::VecElementID VecElementID;
	typedef typename Inherited::VecEdges VecEdges;
	typedef typename Inherited::VecIndex VecIndex;

	typedef typename Inherited::Transform Transform;
	typedef typename Inherited::SpatialVector SpatialVector;

	typedef typename Inherited::Vec3 Vec3;
	typedef typename Inherited::Vec6 Vec6;

	WireBeamInterpolation(sofa::component::engine::WireRestShape<DataTypes> *_restShape = NULL);

	~WireBeamInterpolation();

	void init();
	void bwdInit();
	void reinit(){init(); bwdInit(); }

	virtual void clear();

	using BeamInterpolation::addBeam;
    void addBeam(const BaseMeshTopology::EdgeID &eID  , const Real &length, const Real &x0, const Real &x1,
                 const Transform &DOF0_H_Node0, const Transform &DOF1_H_Node1);


	virtual void getSamplingParameters(helper::vector<Real>& xP_noticeable, helper::vector< int>& nbP_density)
	{
		this->m_restShape->getSamplingParameters(xP_noticeable, nbP_density);
	}

	virtual Real getRestTotalLength()
	{
		return this->m_restShape->getLength();
	}

	virtual void getCollisionSampling(Real &dx, const Real& x_localcurv_abs)
	{
		this->m_restShape->getCollisionSampling(dx,x_localcurv_abs);
	}

	virtual void getNumberOfCollisionSegment(Real &dx, unsigned int &numLines)
	{
		this->m_restShape->getNumberOfCollisionSegment(dx,numLines);
	}


	virtual void getYoungModulusAtX(int beamId,Real& x_curv, Real& youngModulus, Real& cPoisson)
	{
		this->getAbsCurvXFromBeam(beamId, x_curv);
		this->m_restShape->getYoungModulusAtX(x_curv, youngModulus, cPoisson);
	}


	virtual void getRestTransform(unsigned int edgeInList, Transform &local0_H_local1_rest);
	virtual void getSplineRestTransform(unsigned int edgeInList, Transform &local_H_local0_rest, Transform &local_H_local1_rest);
	virtual void getBeamAtCurvAbs(const Real& x_input, unsigned int &edgeInList_output, Real& baryCoord_output);
	void getCurvAbsAtBeam(unsigned int &edgeInList_input, Real& baryCoord_input, Real& x_output);
	bool getApproximateCurvAbs(const Vec3& x_input, const VecCoord& x,  Real& x_output);	// Project a point on the segments, return false if cant project
	bool getCurvAbsOfProjection(const Vec3& x_input, const VecCoord&, Real& x_output, const Real& tolerance);


	bool breaksInTwo(const Real &x_min_out,  Real &x_break, int &numBeamsNotUnderControlled );

        void setPathToRestShape(const std::string &o){m_restShape.setPath(o);};

        void getRestTransformOnX(Transform &global_H_local, const Real &x)
        {
            if(this->m_restShape){
                this->m_restShape->getRestTransformOnX(global_H_local, x);
                return;
            }
            else
            {
                global_H_local.set(Vec3(x,0,0), Quat());

            }

        }

protected :
        // pointer on an external rest-shape
//	sofa::core::objectmodel::DataObjectRef m_restShapePath;

        //link on an external rest-shape...
        SingleLink<WireBeamInterpolation<DataTypes>, sofa::component::engine::WireRestShape<DataTypes>, BaseLink::FLAG_STOREPATH|BaseLink::FLAG_STRONGLINK> m_restShape;

        //sofa::component::engine::WireRestShape<DataTypes> *m_restShape;


public:

	template<class T>
        static bool canCreate(T* obj, sofa::core::objectmodel::BaseContext* context, sofa::core::objectmodel::BaseObjectDescription* arg)
	{
		return Inherited::canCreate(obj,context,arg);
	}


        template<class T>
        static typename T::SPtr  create(T* tObj, core::objectmodel::BaseContext* context, core::objectmodel::BaseObjectDescription* arg)
	{
		WireRestShape<DataTypes>* _restShape = NULL;
		bool                  _pathCheckedOk = false;
		std::string _restShapePath;

		if(arg){

			if(arg->getAttribute("WireRestShape",NULL) != NULL){
				_restShapePath = arg->getAttribute("WireRestShape");
                                 context->findLinkDest(_restShape, _restShapePath, NULL);
				_pathCheckedOk = true;
			}

			if(_restShape == NULL)
			{
				_restShapePath=" ";
                                context->serr << "WARNING("<<className ( tObj ) <<") : WireRestShape attribute not used, WireBeamInterpolation will be constructed with a default WireRestShape"<<context->sendl;
				_restShape = new WireRestShape<DataTypes>();
				_pathCheckedOk = false;
			}
		}

                typename T::SPtr obj = sofa::core::objectmodel::New<T>(_restShape);
                obj->setPathToRestShape(_restShapePath);
                if (context) context->addObject(obj);
                if (arg) obj->parse(arg);
                return obj;

	}

};

} // namespace fem

} // namespace component

} // namespace sofa

#endif  /*SOFA_COMPONENT_FEM_WIREBEAMINTERPOLATION_H*/
