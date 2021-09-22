// Copyright (c) 2018-2021 OPEN CASCADE SAS
//
// This file is part of Open CASCADE Technology software library.
//
// This library is free software; you can redistribute it and/or modify it under
// the terms of the GNU Lesser General Public License version 2.1 as published
// by the Free Software Foundation, with special exception defined in the file
// OCCT_LGPL_EXCEPTION.txt. Consult the file LICENSE_LGPL_21.txt included in OCCT
// distribution for complete text of the license and disclaimer of any warranty.
//
// Alternatively, this file may be used under the terms of Open CASCADE
// commercial license or contractual agreement.

#include <Dynamics_MotionState.hxx>

#include <Dynamics_Internal.hxx>

IMPLEMENT_STANDARD_RTTIEXT(Dynamics_MotionState, Standard_Transient)

#ifdef HAVE_BULLET
ATTRIBUTE_ALIGNED16(struct) Dynamics_MotionState::Dynamics_MotionStateProxy : public btDefaultMotionState
{
  Dynamics_MotionStateProxy (Dynamics_MotionState* theHolder,
                             const btTransform& theStartTrans = btTransform::getIdentity(),
                             const btTransform& theCenterOfMassOffset = btTransform::getIdentity(),
                             const gp_Trsf& thePrsOffset = gp_Trsf())
  : btDefaultMotionState (theStartTrans, theCenterOfMassOffset),
    myHolder (theHolder),
    myPrsOffset (thePrsOffset) {}

  virtual void setWorldTransform (const btTransform& theCenterOfMassWorldTrans) Standard_OVERRIDE
  {
    btDefaultMotionState::setWorldTransform (theCenterOfMassWorldTrans);

    btTransform aTrsfBt;
    getWorldTransform (aTrsfBt);

    gp_Trsf aTrsf;
    Dynamics_MotionState::TrsfGpFromBt (aTrsf, aTrsfBt);
    aTrsf *= myPrsOffset;
    myHolder->setWorldTransform (aTrsf);
  }
private:
  Dynamics_MotionState* myHolder;
  gp_Trsf myPrsOffset;
};
#endif

// =======================================================================
// function : Dynamics_MotionState
// purpose  :
// =======================================================================
Dynamics_MotionState::Dynamics_MotionState()
: myMotionState (NULL),
  myToDisplaceCenterOfMass (false)
{
  //
}

// =======================================================================
// function : ~Dynamics_MotionState
// purpose  :
// =======================================================================
Dynamics_MotionState::~Dynamics_MotionState()
{
  releaseMotionState();
}

// =======================================================================
// function : InitProxy
// purpose  :
// =======================================================================
void Dynamics_MotionState::InitProxy (const gp_Trsf& theStartTrans,
                                      const gp_Trsf& theCenterOfMassOffset,
                                      const gp_Trsf& thePrsOffset)
{
  releaseMotionState();
#ifdef HAVE_BULLET
  btTransform aTrsfBt, aMassOffsetBt;
  Dynamics_MotionState::TrsfBtFromGp (aTrsfBt, theStartTrans);
  Dynamics_MotionState::TrsfBtFromGp (aMassOffsetBt, theCenterOfMassOffset);
  myMotionState = new Dynamics_MotionStateProxy (this, aTrsfBt, aMassOffsetBt, thePrsOffset);
#else
  (void )theStartTrans;
  (void )theCenterOfMassOffset;
  (void )thePrsOffset;
#endif
}

// =======================================================================
// function : InitDefault
// purpose  :
// =======================================================================
void Dynamics_MotionState::InitDefault (const btTransform& theStartTrans,
                                        const btTransform& theCenterOfMassOffset)
{
  releaseMotionState();
#ifdef HAVE_BULLET
  myMotionState = new btDefaultMotionState (theStartTrans, theCenterOfMassOffset);
#else
  (void )theStartTrans;
  (void )theCenterOfMassOffset;
#endif
}

// =======================================================================
// function : setWorldTransform
// purpose  :
// =======================================================================
void Dynamics_MotionState::setWorldTransform (const gp_Trsf& theTrsf)
{
  (void )theTrsf;
}

// =======================================================================
// function : releaseMotionState
// purpose  :
// =======================================================================
void Dynamics_MotionState::releaseMotionState()
{
#ifdef HAVE_BULLET
  delete myMotionState;
#endif
  myMotionState = NULL;
}

// =======================================================================
// function : TrsfGpFromBt
// purpose  :
// =======================================================================
void Dynamics_MotionState::TrsfGpFromBt (gp_Trsf& theLoc, const btTransform& theTrsf)
{
#ifdef HAVE_BULLET
  const btVector3&   anOrigin = theTrsf.getOrigin();
  const btMatrix3x3& aMat3bt  = theTrsf.getBasis();
  theLoc.SetValues (aMat3bt.getRow (0).x(), aMat3bt.getRow (0).y(), aMat3bt.getRow (0).z(), anOrigin.x(),
                    aMat3bt.getRow (1).x(), aMat3bt.getRow (1).y(), aMat3bt.getRow (1).z(), anOrigin.y(),
                    aMat3bt.getRow (2).x(), aMat3bt.getRow (2).y(), aMat3bt.getRow (2).z(), anOrigin.z());
#else
  (void )theTrsf;
  (void )theLoc;
#endif
}

// =======================================================================
// function : TrsfBtFromGp
// purpose  :
// =======================================================================
void Dynamics_MotionState::TrsfBtFromGp (btTransform& theTrsf, const gp_Trsf& theLoc)
{
#ifdef HAVE_BULLET
  theTrsf.setIdentity();
  if (theLoc.Form() == gp_Identity)
  {
    return;
  }

  const gp_Mat aMat3 = theLoc.VectorialPart();
  btVector3   anOrigin (btScalar (theLoc.TranslationPart().X()), btScalar (theLoc.TranslationPart().Y()), btScalar (theLoc.TranslationPart().Z()));
  btMatrix3x3 aMat3bt  (btScalar (aMat3.Value (1, 1)), btScalar (aMat3.Value (1, 2)), btScalar (aMat3.Value (1, 3)),
                        btScalar (aMat3.Value (2, 1)), btScalar (aMat3.Value (2, 2)), btScalar (aMat3.Value (2, 3)),
                        btScalar (aMat3.Value (3, 1)), btScalar (aMat3.Value (3, 2)), btScalar (aMat3.Value (3, 3)));
  theTrsf.setOrigin(anOrigin);
  theTrsf.setBasis (aMat3bt);
#else
  (void )theTrsf;
  (void )theLoc;
#endif
}
