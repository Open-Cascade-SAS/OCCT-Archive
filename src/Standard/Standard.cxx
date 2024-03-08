// Copyright (c) 1999-2023 OPEN CASCADE SAS
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

#include <Standard.hxx>

#include <Standard_OutOfMemory.hxx>

#include <stdlib.h>

#if(defined(_WIN32) || defined(__WIN32__))
#include <windows.h>
#include <malloc.h>
#include <locale.h>
#endif

#if defined(_MSC_VER) || defined(__ANDROID__) || defined(__QNX__)
#include <malloc.h>
#elif (defined(__GNUC__) && (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 1)) && (defined(__i386) || defined(__x86_64)))
#include <mm_malloc.h>
#else
extern "C" int posix_memalign(void** thePtr, size_t theAlign, size_t theSize);
#endif

#ifdef OCCT_MMGT_OPT_JEMALLOC
#define JEMALLOC_NO_DEMANGLE
#include <jemalloc.h>
#endif // OCCT_MMGT_OPT_JEMALLOC

// paralleling with Intel TBB
#ifdef HAVE_TBB
#include <tbb/scalable_allocator.h>
#else
#ifdef OCCT_MMGT_OPT_TBB
#undef OCCT_MMGT_OPT_TBB
#endif
#define scalable_malloc malloc
#define scalable_calloc calloc
#define scalable_realloc realloc
#define scalable_free free
#endif

// Available macros definition
// - OCCT_MMGT_OPT_TBB, using tbb::scalable_allocator
// - OCCT_MMGT_OPT_NATIVE, using native calloc, free
// - OCCT_MMGT_OPT_JEMALLOC, using external jecalloc, jefree

//=======================================================================
//function : Allocate
//purpose  :
//=======================================================================
Standard_Address Standard::Allocate(const Standard_Size theSize)
{
#if defined OCCT_MMGT_OPT_JEMALLOC
  Standard_Address aPtr = je_calloc(theSize, sizeof(char));
  if (!aPtr)
    throw Standard_OutOfMemory("Standard_MMgrRaw::Allocate(): malloc failed");
  return aPtr;
#elif defined OCCT_MMGT_OPT_TBB
  Standard_Address aPtr = scalable_calloc(theSize, sizeof(char));
  if (!aPtr)
    throw Standard_OutOfMemory("Standard_MMgrRaw::Allocate(): malloc failed");
  return aPtr;
#else
  Standard_Address aPtr = calloc(theSize, sizeof(char));
  if (!aPtr)
    throw Standard_OutOfMemory("Standard_MMgrRaw::Allocate(): malloc failed");
  return aPtr;
#endif
}

//=======================================================================
//function : Free
//purpose  :
//=======================================================================
void Standard::Free(Standard_Address theStorage)
{
#if defined OCCT_MMGT_OPT_JEMALLOC
  je_free(theStorage);
#elif defined OCCT_MMGT_OPT_TBB
  scalable_free(theStorage);
#else
  free(theStorage);
#endif
}

//=======================================================================
//function : Reallocate
//purpose  :
//=======================================================================
Standard_Address Standard::Reallocate(Standard_Address theStorage,
                                      const Standard_Size theSize)
{
  // Note that it is not possible to ensure that additional memory
  // allocated by realloc will be cleared (so as to satisfy myClear mode);
  // in order to do that we would need using memset..
#if defined OCCT_MMGT_OPT_JEMALLOC
  Standard_Address aNewStorage = (Standard_Address)je_realloc(theStorage, theSize);
  if (!aNewStorage)
    throw Standard_OutOfMemory("Standard_MMgrRaw::Reallocate(): realloc failed");
  return aNewStorage;
#elif defined OCCT_MMGT_OPT_TBB
  Standard_Address aNewStorage = (Standard_Address)scalable_realloc(theStorage, theSize);
  if (!aNewStorage)
    throw Standard_OutOfMemory("Standard_MMgrRaw::Reallocate(): realloc failed");
  return aNewStorage;
#else
  Standard_Address aNewStorage = (Standard_Address)realloc(theStorage, theSize);
  if (!aNewStorage)
    throw Standard_OutOfMemory("Standard_MMgrRaw::Reallocate(): realloc failed");
  return aNewStorage;
#endif
}

//=======================================================================
//function : Purge
//purpose  :
//=======================================================================
Standard_Integer Standard::Purge()
{
  return true;
}

//=======================================================================
//function : AllocateAligned
//purpose  :
//=======================================================================
Standard_Address Standard::AllocateAligned(const Standard_Size theSize,
                                           const Standard_Size theAlign)
{
#ifdef OCCT_MMGT_OPT_JEMALLOC
  return je_aligned_alloc(theAlign, theSize);
#elif defined OCCT_MMGT_OPT_TBB
  return scalable_aligned_malloc(theSize, theAlign);
#else
#if defined(_MSC_VER)
  return _aligned_malloc(theSize, theAlign);
#elif defined(__ANDROID__) || defined(__QNX__)
  return memalign(theAlign, theSize);
#elif (defined(__GNUC__) && (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 1)) && (defined(__i386) || defined(__x86_64)))
  return _mm_malloc(theSize, theAlign);
#else
  void* aPtr;
  if (posix_memalign(&aPtr, theAlign, theSize))
  {
    return NULL;
  }
  return aPtr;
#endif
#endif
}

//=======================================================================
//function : FreeAligned
//purpose  :
//=======================================================================
void Standard::FreeAligned(Standard_Address thePtrAligned)
{
#ifdef OCCT_MMGT_OPT_JEMALLOC
return je_free(thePtrAligned);
#elif defined OCCT_MMGT_OPT_TBB
  return scalable_aligned_free(thePtrAligned);
#else
#if defined(_MSC_VER)
  _aligned_free(thePtrAligned);
#elif defined(__ANDROID__) || defined(__QNX__)
  free(thePtrAligned);
#elif (defined(__GNUC__) && (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 1)) && (defined(__i386) || defined(__x86_64)))
  _mm_free(thePtrAligned);
#else
  free(thePtrAligned);
#endif
#endif
}
