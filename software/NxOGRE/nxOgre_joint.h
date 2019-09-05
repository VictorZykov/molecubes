/*

	NxOgre a wrapper for the PhysX (formerly Novodex) physics library and the Ogre 3D rendering engine.
	Copyright (C) 2005, 2006 Robin Southern and NxOgre.org http://www.nxogre.org

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	This library is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
	Lesser General Public License for more details.

	You should have received a copy of the GNU Lesser General Public
	License along with this library; if not, write to the Free Software
	Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

*/


#ifndef __nxOgre_joint_H__
#define __nxOgre_joint_H__

#include "nxOgre_includes.h"
#include "nxOgre_joint_params.h"

namespace nxOgre {


	class _nxOgreExport joint {

		friend body;
		friend scene;
		
		public:

			static void	destroyJoint(joint *j);
			static pulleyJoint* createPulleyJoint(body *a, body *b, Ogre::Vector3 apulley, Ogre::Vector3 bpulley, Ogre::Vector3 axis, Ogre::Real distance, Ogre::Real ratio);
			static sphericalJoint* createSphericalJoint(body *_first, body *_second, Ogre::Vector3 _anchor);
			static sphericalJoint*  createSphericalJoint(body *_first, Ogre::Vector3 _anchor);
			static revoluteJoint*  createRevoluteJoint(body *_first, body *_second, Ogre::Vector3 _anchor, Ogre::Vector3 _axis, bool _collisions, params<limit> _jointLimits = params<limit>(), params<spring> _spring = params<spring>());
			static revoluteJoint*  createRevoluteJoint(body *_first, Ogre::Vector3 _anchor, Ogre::Vector3 _axis, params<limit> _jointLimits = params<limit>());
			static SDFJoint* createD6Joint(body *_first, Ogre::Vector3 _anchor, Ogre::Vector3 _axis);

			enum jointType {
				SPHERICAL_JOINT, REVOLUTE_JOINT,
				CYLINDRICAL_JOINT, PRISMATIC_JOINT,
				POINTONLINE_JOINT, POINTONPLANE_JOINT,
				FIXED_JOINT, DISTANCE_JOINT,
				PULLEY_JOINT, SDF_JOINT,
				MOTORISED_JOINT
			};

			joint(jointType _type,body *_first, body *_second);
			joint(jointType _type,body *_first);
			~joint();

			jointType getType();

			bool	breakJoint();
			void	onBreak();

			body *mFirst;
			body *mSecond;

			
			//NxJoint *mJoint;


		protected:
			jointType mType;
			static void __destroyJoint(joint *j);
			static void __releaseJoint(joint *j, scene *s);

	};

	// ========================================================


	class _nxOgreExport jointDescription {
		
		friend joint;
		friend revoluteJoint;

		public:

			jointDescription();
			jointDescription(Ogre::Real _low, Ogre::Real _high);
			jointDescription(Ogre::Real _spring);

			void setHighLimits(Ogre::Real _value, Ogre::Real _hardness = NULL, Ogre::Real _restitution = NULL);
			void setLowLimits(Ogre::Real _value, Ogre::Real _hardness = NULL, Ogre::Real _restitution = NULL);
			void setSpring(Ogre::Real _spring, Ogre::Real _damper = NULL, Ogre::Real _target = NULL);

		protected:
			

			// Push it to the limit.
			bool limit;
			Ogre::Real limit_high_value;
			Ogre::Real limit_high_hardness;
			Ogre::Real limit_high_restitution;
			Ogre::Real limit_low_value;
			Ogre::Real limit_low_hardness;
			Ogre::Real limit_low_restitution;


			// Springy...
			bool spring;
			Ogre::Real spring_damper;
			Ogre::Real spring_spring;
			Ogre::Real spring_target;
	
	};


	// ========================================================
	// 					Joint subclasses.
	// ========================================================

	class _nxOgreExport sphericalJoint : public joint {
		
		public:
			sphericalJoint(body *_first, body *_second, Ogre::Vector3 _anchor);
			sphericalJoint(body *_first, Ogre::Vector3 _anchor);
			~sphericalJoint();

		
		
			NxJoint *mJoint;	
			NxSphericalJointDesc mJointDesc;

	};

	class _nxOgreExport revoluteJoint : public joint {

		public:

			revoluteJoint(body *_first, body *_second, Ogre::Vector3 _anchor, Ogre::Vector3 _axis, bool _collisions, params<limit> _jointLimits = params<limit>(), params<spring> _spring = params<spring>());
			revoluteJoint(body *_first, Ogre::Vector3 _anchor, Ogre::Vector3 _axis, params<limit> _jointLimits = params<limit>());

			void setReporter(customJointReporter *_reporter);
			void addLimitPlane(Ogre::Vector3 _normal, Ogre::Vector3 _pointInPlane);
			void setBreakable(Ogre::Real _maxForce, Ogre::Real _maxTorque);
			void destroyLimitPlanes();

			NxJoint *mJoint;
			NxRevoluteJointDesc mJointDesc;
		
		private:
			customJointReporter *mReporter;
			
	};

	class _nxOgreExport SDFJoint : public joint {

		public:

			SDFJoint(body *_first, body *_second, Ogre::Vector3 _anchor, Ogre::Vector3 _axis);
	
			void setReporter(customJointReporter *_reporter);
			//void addLimitPlane(Ogre::Vector3 _normal, Ogre::Vector3 _pointInPlane);
			//void setBreakable(Ogre::Real _maxForce, Ogre::Real _maxTorque);
			//void destroyLimitPlanes();

			//NxJoint *mJoint;
			NxD6Joint *mJoint;

			//NxRevoluteJointDesc mJointDesc;
			NxD6JointDesc mJointDesc;
		
		private:
			customJointReporter *mReporter;	
	};
	
	class _nxOgreExport fixedJoint : public joint {

		public:

			fixedJoint(body *_first, body *_second, Ogre::Vector3 _anchor = Ogre::Vector3::ZERO, Ogre::Vector3 _anchor2 = Ogre::Vector3::ZERO);
			fixedJoint(body *_first, Ogre::Vector3 _anchor);

			void setReporter(customJointReporter *_reporter);
			void setBreakable(Ogre::Real _maxForce, Ogre::Real _maxTorque);

			NxJoint *mJoint;
			NxFixedJointDesc mJointDesc;
		
		private:
			customJointReporter *mReporter;
			
	};

		class _nxOgreExport distanceJoint : public joint {

		public:

			distanceJoint(body *_first, body *_second, Ogre::Vector3 _anchor = Ogre::Vector3::ZERO, Ogre::Vector3 _anchor2 = Ogre::Vector3::ZERO);
			distanceJoint(body *_first, Ogre::Vector3 _anchor);
			~distanceJoint();

			NxJoint *mJoint;
			NxDistanceJointDesc mJointDesc;
	
			
	};

	class _nxOgreExport cylindricalJoint : public joint {
		
		public:
			cylindricalJoint(body *_first, body *_second, Ogre::Vector3 _anchor, Ogre::Vector3 _axis);
			cylindricalJoint(body *_first, Ogre::Vector3 _anchor, Ogre::Vector3 _axis);

			NxJoint *mJoint;
			NxCylindricalJointDesc mJointDesc;

	};


	class _nxOgreExport motorisedJoint : public joint {
		
		public:
			motorisedJoint(body *_first, body *_second, Ogre::Vector3 _anchor, Ogre::Vector3 _axis, bool _collisions = false);
			motorisedJoint(body *_first, Ogre::Vector3 _anchor, Ogre::Vector3 _axis);

			void 	setReporter(customJointReporter *_reporter);
			void	setVelocityTarget(Ogre::Real _target);
			void	setMaxForce(Ogre::Real _force);

			Ogre::Real	getVelocityTarget();
			Ogre::Real	getMaxForce();
			void	setFreeSpin(bool _value);
			void	setBreakable(Ogre::Real _force, Ogre::Real _torque);

			NxRevoluteJoint *mJoint;
			
		
		private:
			customJointReporter *mReporter;	
	};

	class _nxOgreExport pulleyJoint : public joint {
		
		public:

			pulleyJoint(body *a, body *b, Ogre::Vector3 apulley, Ogre::Vector3 bpulley, Ogre::Vector3 axis, Ogre::Real distance, Ogre::Real ratio);

			NxPulleyJoint *mJoint;

		private:

			
	};

} //::nxOgre.

#endif