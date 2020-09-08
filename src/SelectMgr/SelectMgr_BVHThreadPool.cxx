#include <SelectMgr_BVHThreadPool.hxx>
#include <OSD_Parallel.hxx>

IMPLEMENT_STANDARD_RTTIEXT(SelectMgr_BVHThreadPool, Standard_Transient)

//==================================================
// Function: SelectMgr_BVHThreadPool
// Purpose :
//==================================================
SelectMgr_BVHThreadPool::SelectMgr_BVHThreadPool (Standard_Integer theNbThreads)
: myToStopBVHThread(Standard_False),
  myWakeEvent(Standard_False),
  myIdleEvent(Standard_True),
  myIsStarted(Standard_False)
{
  Standard_Integer aBVHThreadsNum = theNbThreads > 0 ? theNbThreads : OSD_Parallel::NbLogicalProcessors() > 1 ? OSD_Parallel::NbLogicalProcessors() - 1 : 1;
  myBVHThreads = NCollection_Array1<OSD_Thread>(1, aBVHThreadsNum);
  myBVHBuildData = NCollection_Array1<BVHBuild_Data>(1, aBVHThreadsNum);

  for (Standard_Integer i = myBVHThreads.Lower(); i <= myBVHThreads.Upper(); ++i)
  {
    myBVHThreads.ChangeValue(i).SetFunction (buildBVHThreadFunc);
    myBVHThreads.ChangeValue(i).SetPriority (THREAD_PRIORITY_BELOW_NORMAL);
    myBVHBuildData.ChangeValue(i).Pool = this;
  }
}

//==================================================
// Function: ~SelectMgr_BVHThreadPool
// Purpose :
//==================================================
SelectMgr_BVHThreadPool::~SelectMgr_BVHThreadPool()
{
  StopThreads();
}

//==================================================
// Function: StopThreads
// Purpose :
//==================================================
void SelectMgr_BVHThreadPool::StopThreads()
{
  if (!myIsStarted)
  {
    return;
  }
  myToStopBVHThread = Standard_True;
  myWakeEvent.Set();
  for (Standard_Integer i = myBVHThreads.Lower(); i <= myBVHThreads.Upper(); ++i)
  {
    myBVHThreads.ChangeValue(i).Wait();
  }
  myToStopBVHThread = Standard_False;
  myIsStarted = Standard_False;
}

//==================================================
// Function: WaitThreads
// Purpose :
//==================================================
void SelectMgr_BVHThreadPool::WaitThreads()
{
  myIdleEvent.Wait();

  LockBVHBuildMutex();
  UnlockBVHBuildMutex();
}

//=======================================================================
//function : BuildBVH
//purpose  : 
//=======================================================================
void SelectMgr_BVHThreadPool::BuildBVH (const Handle(Select3D_SensitiveEntity)& theEntity)
{
  myBVHListMutex.Lock();
  myBVHToBuildList.Append (theEntity);
  myWakeEvent.Set();
  myIdleEvent.Reset();
  myBVHListMutex.Unlock();

  if (!myIsStarted)
  {
    myIsStarted = Standard_True;
    for (Standard_Integer i = myBVHThreads.Lower(); i <= myBVHThreads.Upper(); ++i)
    {
      myBVHThreads.ChangeValue(i).Run((Standard_Address)(&myBVHBuildData.ChangeValue(i)));
    }
  }
}

//=======================================================================
//function : buildBVHThreadFunc
//purpose  : 
//=======================================================================
Standard_Address SelectMgr_BVHThreadPool::buildBVHThreadFunc (Standard_Address data)
{
  BVHBuild_Data* aData = reinterpret_cast<BVHBuild_Data*> (data);
  SelectMgr_BVHThreadPool* aPool = aData->Pool;
  Standard_Mutex& aBVHBuildMutex = aData->Mutex;

  for (;;)
  {
    aPool->myWakeEvent.Wait();

    if (aPool->myToStopBVHThread)
    {
      break;
    }

    aPool->myBVHListMutex.Lock();
    if (aPool->myBVHToBuildList.IsEmpty())
    {
      aPool->myWakeEvent.Reset();
      aPool->myIdleEvent.Set();
      aPool->myBVHListMutex.Unlock();
      continue;
    }
    Handle(Select3D_SensitiveEntity) anEntity = aPool->myBVHToBuildList.First();
    aPool->myBVHToBuildList.RemoveFirst();
    
    aBVHBuildMutex.Lock();
    aPool->myBVHListMutex.Unlock();

    if (!anEntity.IsNull())
    {
      anEntity->BVH();
    }
    aBVHBuildMutex.Unlock();
  }

  return (Standard_Address)(0);
}

//=======================================================================
//function : LockBVHBuildMutex
//purpose  : 
//=======================================================================
void SelectMgr_BVHThreadPool::LockBVHBuildMutex()
{
  for (Standard_Integer i = myBVHBuildData.Lower(); i <= myBVHBuildData.Upper(); ++i)
  {
    myBVHBuildData.ChangeValue(i).Mutex.Lock();
  }
}

//=======================================================================
//function : UnlockBVHBuildMutex
//purpose  : 
//=======================================================================
void SelectMgr_BVHThreadPool::UnlockBVHBuildMutex()
{
  for (Standard_Integer i = myBVHBuildData.Lower(); i <= myBVHBuildData.Upper(); ++i)
  {
    myBVHBuildData.ChangeValue(i).Mutex.Unlock();
  }
}

