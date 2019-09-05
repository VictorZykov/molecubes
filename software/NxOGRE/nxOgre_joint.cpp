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

#include "nxOgre_joint.h"
#include "nxOgre_world.h"
#include "nxOgre_scene.h"
#include "nxOgre_joint_params.h"
#include "nxOgre_converters.h"

namespace nxOgre {

//////////////////////////////////////////////////////////////////////

joint::joint(jointType _type, body *_first, body *_second) {
mType = _type;
mFirst = _first;
mSecond = _second;
}

//////////////////////////////////////////////////////////////////////

joint::joint(jointType _type, body *_first) {
mType = _type;
mFirst = _first;
mSecond = NULL;
}

//////////////////////////////////////////////////////////////////////

joint::~joint() {
}

//////////////////////////////////////////////////////////////////////

joint::jointType joint::getType() {
	return mType;
}


void joint::destroyJoint(joint *j) {
	j->mFirst->owner->destroyJoint(j);
}

//////////////////////////////////////////////////////////////////////

void joint::__destroyJoint(joint *j) {

	switch(j->getType()) {
		case joint::REVOLUTE_JOINT:
			delete static_cast<revoluteJoint*>(j);
		break;

		case joint::FIXED_JOINT:
			delete static_cast<fixedJoint*>(j);
		break;

		case joint::PULLEY_JOINT:
			delete static_cast<pulleyJoint*>(j);
		break;
	}

}

void joint::__releaseJoint(nxOgre::joint *j, nxOgre::scene *s) {
	
	switch(j->getType()) {
		case joint::REVOLUTE_JOINT:
			s->mScene->releaseJoint(*static_cast<revoluteJoint*>(j)->mJoint);
		break;

		case joint::FIXED_JOINT:
			s->mScene->releaseJoint(*static_cast<fixedJoint*>(j)->mJoint);
		break;

		case joint::PULLEY_JOINT:
			NXD("Releasing joint?");
			s->mScene->releaseJoint(*static_cast<pulleyJoint*>(j)->mJoint);
		break;

	}


}

//////////////////////////////////////////////////////////////////////

jointDescription::jointDescription() {
	limit_high_value = NULL;
	limit_low_value = NULL;
	limit = false;
	spring = false;
}

//////////////////////////////////////////////////////////////////////

jointDescription::jointDescription(Ogre::Real _low, Ogre::Real _high) {
	setLowLimits(_low);
	setHighLimits(_high);
}

//////////////////////////////////////////////////////////////////////

jointDescription::jointDescription(Ogre::Real _spring) {
	setSpring(_spring);
}

//////////////////////////////////////////////////////////////////////

void jointDescription::setHighLimits(Ogre::Real _value, Ogre::Real _hardness , Ogre::Real _restitution) {
	this->limit_high_value = _value;
	this->limit_high_restitution = _restitution;
	this->limit_high_hardness = _hardness;

	if (limit_low_value != NULL)
		limit = true;
}

//////////////////////////////////////////////////////////////////////

void jointDescription::setLowLimits(Ogre::Real _value, Ogre::Real _hardness , Ogre::Real _restitution) {
	this->limit_low_value = _value;
	this->limit_low_restitution = _restitution;
	this->limit_low_hardness = _hardness;

	if (limit_high_value != NULL)
		limit = true;		
}

//////////////////////////////////////////////////////////////////////

void jointDescription::setSpring(Ogre::Real _spring, Ogre::Real _damper, Ogre::Real _target) {
	this->spring_spring = _spring;
	this->spring_damper = _damper;
	this->spring_target = _target;

	spring = true;
}

//////////////////////////////////////////////////////////////////////

sphericalJoint::sphericalJoint(body *_first, body *_second, Ogre::Vector3 _anchor) : joint (joint::SPHERICAL_JOINT,_first,_second){

	mJointDesc.setToDefault();
	mJointDesc.actor[0] = mFirst->mActor;
	mJointDesc.actor[1] = mSecond->mActor;
	mJointDesc.setGlobalAnchor(NxTools::convert(_anchor));

	mJoint = mFirst->owner->mScene->createJoint(mJointDesc);
	mJoint->userData = this;
	
}

//////////////////////////////////////////////////////////////////////

sphericalJoint::sphericalJoint(body *_first, Ogre::Vector3 _anchor) : joint (joint::SPHERICAL_JOINT,_first) {
	mJointDesc.setToDefault();
	mJointDesc.actor[0] = mFirst->mActor;
	mJointDesc.actor[1] = NULL;

	mJointDesc.setGlobalAnchor(NxTools::convert(_anchor));

	mJoint = mFirst->owner->mScene->createJoint(mJointDesc);
	mJoint->userData = this;
}

//////////////////////////////////////////////////////////////////////

fixedJoint::fixedJoint(body *_first, body *_second, Ogre::Vector3 _anchor,Ogre::Vector3 _anchor2) : joint (joint::FIXED_JOINT,_first,_second){

	mJointDesc.setToDefault();
	mJointDesc.actor[0] = mFirst->mActor;
	mJointDesc.actor[1] = mSecond->mActor;
	mJointDesc.localAnchor[0] = NxTools::convert(_anchor);
	mJointDesc.localAnchor[1] = NxTools::convert(_anchor2);
	// Temp.
//		mJointDesc.spring.damper = 1.0f;
//		mJointDesc.spring.spring = 200.0f;
//		mJointDesc.flags |= NX_DJF_MAX_DISTANCE_ENABLED | NX_DJF_SPRING_ENABLED;

	mJoint = mFirst->owner->mScene->createJoint(mJointDesc);
	mJoint->userData = this;
	
}

//////////////////////////////////////////////////////////////////////

void fixedJoint::setBreakable(Ogre::Real _maxForce, Ogre::Real _maxTorque) {
	mJoint->setBreakable(_maxForce, _maxTorque);
}

//////////////////////////////////////////////////////////////////////

distanceJoint::distanceJoint(body *_first, body *_second, Ogre::Vector3 _anchor,Ogre::Vector3 _anchor2) : joint (joint::DISTANCE_JOINT,_first,_second){

	mJointDesc.setToDefault();
	mJointDesc.actor[0] = mFirst->mActor;
	mJointDesc.actor[1] = mSecond->mActor;
	mJointDesc.localAnchor[0] = NxTools::convert(_anchor);
	mJointDesc.localAnchor[1] = NxTools::convert(_anchor2);
	// Temp.
	mJointDesc.spring.damper = 1.0f;
	mJointDesc.spring.spring = 200.0f;
	mJointDesc.flags |= NX_DJF_MAX_DISTANCE_ENABLED | NX_DJF_SPRING_ENABLED;

	mJoint = mFirst->owner->mScene->createJoint(mJointDesc);
	mJoint->userData = this;
}

//////////////////////////////////////////////////////////////////////

distanceJoint::~distanceJoint() {
	NxActor* a;
	NxActor* b;
	mJoint->getActors(&a,&b);
	body* b1 = static_cast<body*>(a->userData);
	b1->owner->mScene->releaseJoint(*mJoint);		
}

///////////////////////////////////////////////////////////////////////

SDFJoint::SDFJoint(body *_first, body *_second, Ogre::Vector3 _anchor, Ogre::Vector3 _axis) : joint(joint::SDF_JOINT, _first, _second) {
	mJointDesc.setToDefault();
	mJointDesc.actor[0] = mFirst->mActor;
	mJointDesc.actor[1] = mSecond->mActor;

	mJointDesc.setGlobalAnchor(NxTools::convert(_anchor));
	mJointDesc.setGlobalAxis(NxTools::convert(_axis));

	mJointDesc.twistMotion = NX_D6JOINT_MOTION_LOCKED;
	mJointDesc.swing1Motion = NX_D6JOINT_MOTION_LOCKED;
	mJointDesc.swing2Motion = NX_D6JOINT_MOTION_LOCKED;

	mJointDesc.xMotion = NX_D6JOINT_MOTION_LOCKED;
	mJointDesc.yMotion = NX_D6JOINT_MOTION_LOCKED;
	mJointDesc.zMotion = NX_D6JOINT_MOTION_LOCKED;
	
	mJointDesc.projectionMode = NX_JPM_NONE;

	mJoint = (NxD6Joint*)(mFirst->owner->mScene->createJoint(mJointDesc));
	mJoint->userData = this;
}

///////////////////////////////////////////////////////////////////////

revoluteJoint::revoluteJoint(body *_first, body *_second, Ogre::Vector3 _anchor, Ogre::Vector3 _axis, bool _collisions, params<limit> _jointLimits, params<spring> _spring) : joint(joint::REVOLUTE_JOINT, _first, _second) {
	mJointDesc.setToDefault();
	mJointDesc.actor[0] = mFirst->mActor;
	mJointDesc.actor[1] = mSecond->mActor;

	mJointDesc.setGlobalAnchor(NxTools::convert(_anchor));
	mJointDesc.setGlobalAxis(NxTools::convert(_axis));

	if (_jointLimits.mJointLimited) {

		mJointDesc.flags |= NX_RJF_LIMIT_ENABLED;
		
		mJointDesc.limit.high.value = _jointLimits.mHigh_value.valueRadians();
		mJointDesc.limit.low.value = _jointLimits.mLow_value.valueRadians();

		mJointDesc.limit.high.restitution = _jointLimits.mHigh_restitution;
		mJointDesc.limit.high.hardness = _jointLimits.mHigh_hardness;
		
		mJointDesc.limit.low.restitution = _jointLimits.mLow_restitution;
		mJointDesc.limit.low.hardness = _jointLimits.mLow_hardness;

	}

	if (_spring.mTarget != Ogre::Radian(0)) {
		mJointDesc.flags |= NX_RJF_SPRING_ENABLED;
		mJointDesc.spring.targetValue = _spring.mTarget.valueRadians();
		mJointDesc.spring.damper = _spring.mDamper;
		mJointDesc.spring.spring = _spring.mSpring;
	}

	if (_collisions) {
		mJointDesc.jointFlags |= NX_JF_COLLISION_ENABLED;
	}

	mJoint = mFirst->owner->mScene->createJoint(mJointDesc);
	mJoint->userData = this;
}

//////////////////////////////////////////////////////////////////////

revoluteJoint::revoluteJoint(body *_first, Ogre::Vector3 _anchor, Ogre::Vector3 _axis, params<limit> _jointLimits) : joint(joint::REVOLUTE_JOINT, _first){
	mJointDesc.setToDefault();
	mJointDesc.actor[0] = mFirst->mActor;
	mJointDesc.actor[1] = NULL;

	mJointDesc.setGlobalAnchor(NxTools::convert(_anchor));
	mJointDesc.setGlobalAxis(NxTools::convert(_axis));

	if (_jointLimits.mJointLimited) {

		mJointDesc.flags |= NX_RJF_LIMIT_ENABLED;
		
		mJointDesc.limit.high.value = _jointLimits.mHigh_value.valueRadians();
		mJointDesc.limit.low.value = _jointLimits.mLow_value.valueRadians();

		mJointDesc.limit.high.restitution = _jointLimits.mHigh_restitution;
		mJointDesc.limit.high.hardness = _jointLimits.mHigh_hardness;
		
		mJointDesc.limit.low.restitution = _jointLimits.mLow_restitution;
		mJointDesc.limit.low.hardness = _jointLimits.mLow_hardness;

	}

	/*
	if (_js->spring) {


		NxSpringDesc mSpringDesc;
		mSpringDesc.spring = _js->spring_spring;

		if (_js->spring_damper != NULL)
			mSpringDesc.damper = _js->spring_damper;

		if (_js->spring_target != NULL)
			mSpringDesc.targetValue = _js->spring_target;

		mJointDesc.flags |= NX_RJF_SPRING_ENABLED;
		mJointDesc.spring = mSpringDesc;
		
	}*/

	mJoint = mFirst->owner->mScene->createJoint(mJointDesc);
	mJoint->userData = this;
}

//////////////////////////////////////////////////////////////////////

void revoluteJoint::addLimitPlane(Ogre::Vector3 _normal, Ogre::Vector3 _pointInPlane) {
	mJoint->addLimitPlane(NxTools::convert(_normal),NxTools::convert(_pointInPlane));
}

//////////////////////////////////////////////////////////////////////

void revoluteJoint::setBreakable(Ogre::Real _maxForce, Ogre::Real _maxTorque) {
	mJoint->setBreakable(_maxForce, _maxTorque);
}

//////////////////////////////////////////////////////////////////////

void revoluteJoint::destroyLimitPlanes() {
	mJoint->purgeLimitPlanes();
}

//////////////////////////////////////////////////////////////////////

cylindricalJoint::cylindricalJoint(body *_first, body *_second, Ogre::Vector3 _anchor, Ogre::Vector3 _axis) : joint(joint::CYLINDRICAL_JOINT,_first,_second) {
	mJointDesc.setToDefault();
	mJointDesc.actor[0] = mFirst->mActor;
	mJointDesc.actor[1] = mSecond->mActor;

	mJointDesc.setGlobalAnchor(NxTools::convert(_anchor));
	mJointDesc.setGlobalAxis(NxTools::convert(_axis));

	mJoint = mFirst->owner->mScene->createJoint(mJointDesc);
	mJoint->userData = this;
}

//////////////////////////////////////////////////////////////////////

cylindricalJoint::cylindricalJoint(body *_first, Ogre::Vector3 _anchor, Ogre::Vector3 _axis) : joint(joint::CYLINDRICAL_JOINT,_first)  {
	mJointDesc.setToDefault();
	mJointDesc.actor[0] = mFirst->mActor;
	mJointDesc.actor[1] = NULL;

	mJointDesc.setGlobalAnchor(NxTools::convert(_anchor));
	mJointDesc.setGlobalAxis(NxTools::convert(_axis));

	mJoint = mFirst->owner->mScene->createJoint(mJointDesc);
	mJoint->userData = this;
}

//////////////////////////////////////////////////////////////////////

motorisedJoint::motorisedJoint(body *_first, body *_second, Ogre::Vector3 _anchor, Ogre::Vector3 _axis, bool _collisions) : joint(joint::MOTORISED_JOINT,_first,_second) {
	NxRevoluteJointDesc mJointDesc;
	mJointDesc.actor[0] = mFirst->mActor;
	mJointDesc.actor[1] = mSecond->mActor;

	mJointDesc.setGlobalAnchor(NxTools::convert(_anchor));
	mJointDesc.setGlobalAxis(NxTools::convert(_axis));

	if (_collisions) {
		mJointDesc.jointFlags |= NX_JF_COLLISION_ENABLED;
	}

	mJointDesc.motor.maxForce = 1000;
	mJointDesc.motor.velTarget = 0.15;
	mJointDesc.flags |= NX_RJF_MOTOR_ENABLED;
	
	mJoint = static_cast<NxRevoluteJoint*>(mFirst->owner->mScene->createJoint(mJointDesc));
	mJoint->userData = this;

}

//////////////////////////////////////////////////////////////////////

motorisedJoint::motorisedJoint(body *_first, Ogre::Vector3 _anchor, Ogre::Vector3 _axis) : joint(joint::MOTORISED_JOINT,_first) {

	NxRevoluteJointDesc mJointDesc;
	mJointDesc.actor[0] = mFirst->mActor;
	mJointDesc.actor[1] = NULL;

	mJointDesc.setGlobalAnchor(NxTools::convert(_anchor));
	mJointDesc.setGlobalAxis(NxTools::convert(_axis));


	mJointDesc.motor.maxForce = 1000;
	mJointDesc.motor.velTarget = 0.15;
	mJointDesc.flags |= NX_RJF_MOTOR_ENABLED;
	
	mJoint = static_cast<NxRevoluteJoint*>(mFirst->owner->mScene->createJoint(mJointDesc));
	mJoint->userData = this;
}

//////////////////////////////////////////////////////////////////////

void motorisedJoint::setVelocityTarget(Ogre::Real _target) {
	NxMotorDesc mMotorDesc;
	mJoint->getMotor(mMotorDesc);
	mMotorDesc.velTarget = _target;
	mJoint->setMotor(mMotorDesc);
}

//////////////////////////////////////////////////////////////////////

void motorisedJoint::setMaxForce(Ogre::Real _force) {
	NxMotorDesc mMotorDesc;
	mJoint->getMotor(mMotorDesc);
	mMotorDesc.maxForce = _force;
	mJoint->setMotor(mMotorDesc);
}

//////////////////////////////////////////////////////////////////////

void motorisedJoint::setFreeSpin(bool _value) {
	NxMotorDesc mMotorDesc;
	mJoint->getMotor(mMotorDesc);
	mMotorDesc.freeSpin = _value;
	mJoint->setMotor(mMotorDesc);
}

//////////////////////////////////////////////////////////////////////

Ogre::Real motorisedJoint::getVelocityTarget() {
	return 0;
}

//////////////////////////////////////////////////////////////////////

Ogre::Real motorisedJoint::getMaxForce() {
	return 0;
}

//////////////////////////////////////////////////////////////////////

pulleyJoint* joint::createPulleyJoint(body *a, body *b, Ogre::Vector3 apulley, Ogre::Vector3 bpulley, Ogre::Vector3 axis, Ogre::Real distance, Ogre::Real ratio) {
	pulleyJoint *j = new pulleyJoint(a,b,apulley,bpulley,axis,distance,ratio);
	a->owner->createJoint(j);
	return j;
}

sphericalJoint* joint::createSphericalJoint(body *_first, body *_second, Ogre::Vector3 _anchor) {
	sphericalJoint *j = new sphericalJoint(_first,_second,_anchor);
	_first->owner->createJoint(j);
	return j;
}


//////////////////////////////////////////////////////////////////////

sphericalJoint* joint::createSphericalJoint(body *_first, Ogre::Vector3 _anchor)  {
	sphericalJoint *j = new sphericalJoint(_first,_anchor);
	_first->owner->createJoint(j);
	return j;
}


//////////////////////////////////////////////////////////////////////

revoluteJoint* joint::createRevoluteJoint(body *_first, body *_second, Ogre::Vector3 _anchor, Ogre::Vector3 _axis, bool _collisions, params<limit> _jointLimits, params<spring> _spring) {
	revoluteJoint *j = new revoluteJoint(_first, _second,_anchor, _axis, _collisions, _jointLimits, _spring);
	_first->owner->createJoint(j);
	return j;
}


//////////////////////////////////////////////////////////////////////

revoluteJoint* joint::createRevoluteJoint(body *_first, Ogre::Vector3 _anchor, Ogre::Vector3 _axis, params<limit> _jointLimits) {
	revoluteJoint *j = new revoluteJoint(_first, _anchor, _axis, _jointLimits);
	_first->owner->createJoint(j);
	return j;
}


//////////////////////////////////////////////////////////////////////

pulleyJoint::pulleyJoint(body *a, body *b, Ogre::Vector3 apulley, Ogre::Vector3 bpulley, Ogre::Vector3 axis, Ogre::Real distance, Ogre::Real ratio) : joint(joint::PULLEY_JOINT, a, b){
	
	NxPulleyJointDesc pulleyDesc;
	pulleyDesc.actor[0] = a->mActor;
	pulleyDesc.actor[1] = b->mActor;
	pulleyDesc.localAnchor[0] = NxVec3(0,0,0); // Need something for this? Param maybe.
	pulleyDesc.localAnchor[1] = NxVec3(0,0,0); // "                                   ".
	pulleyDesc.setGlobalAxis(NxTools::convert(axis));
	
	pulleyDesc.pulley[0].set(apulley.x, apulley.y, apulley.z);    // suspension points of two bodies in world space
	pulleyDesc.pulley[1].set(bpulley.x, bpulley.y, bpulley.z);
	pulleyDesc.distance = distance;    // the rest length of the rope connecting the two objects.  The distance is computed as ||(pulley0 - anchor0)|| +  ||(pulley1 - anchor1)|| * ratio.
	pulleyDesc.ratio = ratio;    // transmission ratio
	pulleyDesc.flags = NX_PJF_IS_RIGID;	// This is a combination of the bits defined by ::NxPulleyJointFlag. 
	pulleyDesc.stiffness = 0.01;    // how stiff the constraint is, between 0 and 1 (stiffest)
	
	pulleyDesc.jointFlags |= NX_JF_COLLISION_ENABLED;

	mJoint = static_cast<NxPulleyJoint*>(a->owner->mScene->createJoint(pulleyDesc));
}

//////////////////////////////////////////////////////////////////////

}//::nxOgre