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


#ifndef __nxOgre_body_H__
#define __nxOgre_body_H__

#include "nxOgre_includes.h"
#include "nxOgre_statemachine.h"
#include "nxOgre_body_blueprint.h"
#include "nxOgre_params.h"
#include "nxOgre_shape.h"

namespace nxOgre {

	template<>
	class _nxOgreExport params<rigidbody> {

		public:
			
			params<rigidbody>() {toDefault();}
			/// Params
			/// node:<name>
			/// offset:<vector3>
			/// scale:<vector3>
			/// orientation:<quat>
			/// visible:<bool>
			/// shadows:<bool>
			/// entity: <name>|<pose>;<name>|<pose>|...		(TODO)
			///
			params<rigidbody>(Ogre::String p);

			void toDefault() {
				mNode			=	"";
				mOffset			=	Ogre::Vector3::ZERO;
				mScale			=	Ogre::Vector3(1,1,1);
				mOrientation	=	Ogre::Quaternion::IDENTITY;
				mVisible		=	true;
				mShadows		=	true;
				mGroup			=	0;
				mExtraEntities.clear();
			}

			/// Node to use.	(Pointer to Node, or 0 if create)
			/// Default:""
			Ogre::String mNode;			
			/// Entity(0) offset
			/// Note if offset or orientation are not default, then a second node inside #
			/// mNode will be created to the offset and orientation.
			/// Default: Vector3::ZERO
			Ogre::Vector3	 mOffset;		
			
			/// Node scale
			/// Default: Vector3(1,1,1)
			Ogre::Vector3	 mScale;
			
			/// Entity(0) orientation
			/// Default: Quaternion::Identity;
			Ogre::Quaternion mOrientation;	
			
			/// Is Visible or not
			/// Default: true
			bool			 mVisible;		
			
			/// Cast shadows
			/// Default: true
			bool			 mShadows;		
			
			/// Group to use
			/// Default: 0 (Use default according to type)
			group*			 mGroup;

			/// Extra entities/nodes to add and there pose according to mNode fframe.
			/// Default: Empty vector
			std::vector< std::pair< Ogre::String, pose > > mExtraEntities;

	};

	class _nxOgreExport  body {
	
	friend bodies;
	friend scene;
	friend joint;
	friend sceneIterator;

	public:
		
		/// BodyType enum
		/// Bodies can either be static (non-movable) or dynamic (movable). There is a mixture of both
		/// called kinematic which is a dynamic body that does not respond to forces.
		/// @see scene::createBody()
		/// @see scene::createStaticBody()
		/// @see body::setKinematic()
		enum bodyTypes {
			STATIC,
			DYNAMIC
		};

		enum deletePolicy {
			NXOGRE_OWNED,
			USER_OWNED
		};

		/// Duplicate the body
		/// Make an exact copy of a body, rename it to _otherName and put it at _pose
		/// @param _otherName Name of the copy
		/// @param _pose Where to put it
		/// @return The copy of the body
		body*				duplicate(Ogre::String _otherName, pose _pose = pose());
				
		/// Checks if the Body's Actor has a flag.
		/// @see NxActorFlag
		/// @return Yes or No
		bool				checkActorNxFlag(NxActorFlag _flag);
		
		/// Clear the Body's Actor of a flag.
		/// @see NxActorFlag
		void				clearActorNxFlag(NxActorFlag _flag);
		
		/// Set's the Body's Actor of a flag.
		/// @see NxActorFlag
		void				raiseActorNxFlag(NxActorFlag _flag);
		
		/// Returns the value of the Angular Damping of this body
		/// @return Angular Damping of this body
		Ogre::Real				getAngularDamping();

		/// Sets the value Angular Damping of this body
		/// @param _damping Damping value (0.0 to Inf)
		void				setAngularDamping(Ogre::Real _damping);

		/// Checks if the Body's Actor's Body has a flag, only works if the body is dynamic
		/// @see NxActorFlag
		/// @return Yes or No
		bool				checkBodyNxFlag(NxBodyFlag _flag);

		/// Clear the Body's Actor's Body flag, only works if the body is dynamic
		/// @see NxActorFlag
		void				clearBodyNxFlag(NxBodyFlag _flag);

		/// Raises the Body's Actor's Body flag, only works if the body is dynamic
		/// @see NxActorFlag
		void				raiseBodyNxFlag(NxBodyFlag _flag);


		/// Clears the body's state clear
		///  Depending on the state will reverse - it will be delete within this or the next frame.
		/// @param _stateName State to clear
		void				clearState(stateType _stateName);


		/// Sets the body's density
		/// Sets the body's density then updates the mass based on the volume of the shapes.
		/// @param _density Density in kg.
		void				setDensity(Ogre::Real _density);


		/// Add a shape.
		/// Add's a shape to the body. If you are using this to add shapes one at a time, use
		/// a shapeGroup in the body creation function instead.
		/// @param	_shape	A shape to add.
		void				addShape(shape *_shape);


		/// Add's a force to the body based upon a vector
		/// @param _force		Vector of the force to add in Newton, based on the global frame.
		/// @param _forceMode	NxForceMode of the force.
		void				addForce(Ogre::Vector3 _force, NxForceMode _forceMode = NX_FORCE);

		/// Add's a force to the body based upon a vector
		/// @param _x			X vector of the force to add in Newton, based on the global frame.
		/// @param _z			Y vector of the force to add in Newton, based on the global frame.
		/// @param _z			Z vector of the force to add in Newton, based on the global frame.
		/// @param _forceMode	NxForceMode of the force.		
		void				addForce(Ogre::Real x,Ogre::Real y, Ogre::Real z, NxForceMode _forceMode = NX_FORCE);

		/// Create's a counterpart in the counterpart FX scene.
		/// Create's a counterpart (either 
		bool				createCounterpart();

		void				addForceAtLocalPos(Ogre::Vector3 _force, Ogre::Vector3  _pos, NxForceMode _forceMode = NX_FORCE);
		void				addForceAtPos(Ogre::Vector3 _force, Ogre::Vector3 _pos, NxForceMode _forceMode = NX_FORCE);
		void				addEntity(Ogre::String _meshName, pose _pose = pose());
		void				addEntity(Ogre::Entity* _ent);

		void				setLinearVelocity(Ogre::Vector3 direction, Ogre::Real velocity);
		void				freezeAll();
		void				freezeMovement(Ogre::Vector3 _axis);
		void				freezeMovement();
		void				freezeRotation(Ogre::Vector3 _rot);
		Ogre::Vector3		getFrozenMovementState();
		Ogre::Vector3		getFrozenRotationState();
		bool				getFrozenState();
		void				unFreezeAll();
		void				unFreezeMovement(Ogre::Vector3 _axis);
		void				unFreezeRotation(Ogre::Vector3 _rot);

		std::vector<shape*>		getAllShapes();
		std::vector<shape*>		getSpecificShapes(shape::shapeType type);


		Ogre::Quaternion	getGlobalOrientation();
		Ogre::Vector3		getGlobalPosition();
		void				moveGlobalPosition(Ogre::Vector3 _vel);
		void				moveGlobalPosition(Ogre::Real _x, Ogre::Real _y, Ogre::Real _z);		
		void				moveGlobalOrientationQuat(Ogre::Quaternion _o);
		void				moveGlobalOrientationQuat(Ogre::Real _w,Ogre::Real _x, Ogre::Real _y,Ogre::Real _z);
		void				setGlobalPosition(Ogre::Vector3 _pos);
		void				setGlobalPosition(Ogre::Real _x, Ogre::Real _y, Ogre::Real _z);
		void				setGlobalOrientation(Ogre::Quaternion _o);
		void				setGlobalOrientation(Ogre::Real _w,Ogre::Real _x, Ogre::Real _y,Ogre::Real _z);
		void				setGroup(group *_group);

		void				setIgnoreGravity(bool _ignore);
		bool				isIgnoringGravity();

		void				setKinematic(bool _v);
		bool				isKinematic();

		void				setLinearDamping(Ogre::Real _damping);
		Ogre::Real			getLinearDamping();
		void				addLocalForce(Ogre::Vector3 _force, NxForceMode _forceMode = NX_FORCE);
		void				addLocalForce(Ogre::Real x,Ogre::Real y, Ogre::Real z, NxForceMode _forceMode = NX_FORCE);
		void				addLocalForceAtLocalPos(Ogre::Vector3 _force, Ogre::Vector3 _pos, NxForceMode _forceMode = NX_FORCE);
		void				addLocalForceAtPos(Ogre::Vector3 _force, Ogre::Vector3 _pos, NxForceMode _forceMode = NX_FORCE);

		void				moveTo(pose p);
		void				moveTo(body *b);
		void				moveTo(scene *_scene, pose p);

		Ogre::Real			getMass();

		Ogre::String		getName();
		
							// Returns as 0 if the shapes have more than one material.
		material*			getMaterial();
		Ogre::String		getMaterialAsString();

		void				setSerialisable(bool _v);
		bool				isSerialisable();
		void				simulate(float _time);
		void				render();
		bool				isSleeping();
		void				putToSleep();
		void				addState(state _state);
		void				addState(stateType _stateName, Ogre::Real _frequency = 0);
		bool				isStatic();
		bool				isUnderState(stateType _stateName);
		
		void				addTorque(Ogre::Vector3 _force, NxForceMode _forceMode = NX_FORCE);
		void				addLocalTorque(Ogre::Vector3 _force, NxForceMode _forceMode = NX_FORCE);

		void				wakeUp();
		Ogre::Real			getAge();
		void				setAutoDeletable(bool y);
		bool				isAutoDeletable();

		// Variables
		
		NxActor*				mActor;
		Ogre::Entity*			mEntity;
		Ogre::SceneNode*		mNode;
		std::vector < state >	mState;
		std::vector < shape* >	mShape;
		scene				   *owner;
		void					*ActorUserData;
		

		static const char* NxActorNameIdentifier;

		// "GameObject" constructor. Used for custom classes which inherit body.
		// Use mScene->createBody() or mScene->createStaticBody() to create a body.
		body(Ogre::String _name, nxOgre::shape *_shape, Ogre::Real _density, pose _pos, nxOgre::scene *_scene, bodyTypes _type = body::DYNAMIC);
		

	protected:

		// *structors
		body(Ogre::String _name, nxOgre::scene *_scene, Ogre::String _meshName, nxOgre::shape *_shape, Ogre::Real _density, pose _pos, bodyTypes _type = body::DYNAMIC, params<rigidbody> = params<rigidbody>());	
		body(Ogre::String _name, nxOgre::scene *_scene, Ogre::String _meshName, NxActorDesc &_actor_desc, nxOgre::shape *_shape = NULL);
		~body();

		void setCounterpart(body *b) {
			mCounterpart = b;
		}

		void simulateCounterpart();

		Ogre::String			mName;
		bool					mSerialisable;
		bodyTypes				mType;
		Ogre::Real				mAge;
		bool					mAutoDelete;
		unsigned int			mEntityCount;
		body*					mCounterpart;
		bool					_addShape(NxArray<NxShapeDesc*> &shapes, nxOgre::shape *_shape);
		void					_destroyShape(shape *_shape);
		void					_addVisualWarning(bool isFatal);
		deletePolicy			mDeletePolicy;

	};

}


#endif