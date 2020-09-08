#ifndef _SelectMgr_BVHThreadPool_HeaderFile
#define _SelectMgr_BVHThreadPool_HeaderFile

#include <Standard_Transient.hxx>
#include <OSD_Thread.hxx>
#include <Standard_Mutex.hxx>
#include <Select3D_SensitiveEntity.hxx>
#include <Standard_Condition.hxx>

//! Class defining a thread pool for building BVH for Select3D_SensitiveEntity in multi-threaded mode.
class SelectMgr_BVHThreadPool : public Standard_Transient
{
  DEFINE_STANDARD_RTTIEXT(SelectMgr_BVHThreadPool, Standard_Transient)
public:
  //! Main constructor
  Standard_EXPORT SelectMgr_BVHThreadPool (Standard_Integer theNbThreads);

  //! Destructor
  Standard_EXPORT virtual ~SelectMgr_BVHThreadPool();

public:
  //! Queue a sensitive entity to build its BVH
  Standard_EXPORT void BuildBVH (const Handle(Select3D_SensitiveEntity)& theEntity);

  //! Lock mutexes for building BVH
  Standard_EXPORT void LockBVHBuildMutex();

  //! Unlock mutexes for building BVH
  Standard_EXPORT void UnlockBVHBuildMutex();

  //! Stops threads
  Standard_EXPORT void StopThreads();

  //! Waits for all threads finish their jobs
  Standard_EXPORT void WaitThreads();

protected:

  //! Thread function, accept address of BVHBuild_Data struct as parameter
  static Standard_Address buildBVHThreadFunc (Standard_Address data);

protected:

  //! Structure that will be passed to a separate thread
  struct BVHBuild_Data
  {
    SelectMgr_BVHThreadPool* Pool;
    Standard_Mutex Mutex;
  };
  NCollection_List<Handle(Select3D_SensitiveEntity)> myBVHToBuildList; //!< list of queued sensitive entities
  NCollection_Array1<OSD_Thread> myBVHThreads;                         //!< threads to build BVH
  Standard_Boolean myToStopBVHThread;                                  //!< flag to stop BVH threads
  Standard_Mutex myBVHListMutex;                                       //!< mutex for interaction with myBVHToBuildList
  NCollection_Array1<BVHBuild_Data> myBVHBuildData;                    //!< list of mutexes for building BVH
  Standard_Condition myWakeEvent;                                      //!< raises when any sensitive is added to the BVH list
  Standard_Condition myIdleEvent;                                      //!< raises when BVH list become empty
  Standard_Boolean myIsStarted;                                        //!< indicates that threads are running
};

#endif
