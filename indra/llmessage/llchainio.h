/** 
 * @file llchainio.h
 * @author Phoenix
 * @date 2005-08-04
 * @brief This class declares the interface for constructing io chains.
 *
 * $LicenseInfo:firstyear=2005&license=viewerlgpl$
 * Second Life Viewer Source Code
 * Copyright (C) 2010, Linden Research, Inc.
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation;
 * version 2.1 of the License only.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 * 
 * Linden Research, Inc., 945 Battery Street, San Francisco, CA  94111  USA
 * $/LicenseInfo$
 */

#ifndef LL_LLCHAINIO_H
#define LL_LLCHAINIO_H

#include "llpumpio.h"

/** 
 * @class LLDeferredChain
 * @brief This class allows easy addition of a chain which will sleep
 * and then process another chain.
 */
class LLDeferredChain
{
public:
	/**
	 * @brief Add a chain to a pump in a finite # of seconds
	 *
	 * @prarm pump The pump to work on.
	 * @prarm in_seconds The number of seconds from now when chain should start.
	 * @prarm chain The chain to add in in_seconds seconds.
	 * @prarm chain_timeout timeout for chain on the pump.
	 * @return Returns true if the operation was queued.
	 */
	static bool addToPump(
		LLPumpIO* pump,
		F32 in_seconds,
		const LLPumpIO::chain_t& chain,
		F32 chain_timeout);
};

/** 
 * @class LLChainIOFactory
 * @brief This class is an abstract base class for building io chains.
 *
 * This declares an abstract base class for a chain factory. The
 * factory is used to connect an input pipe to the first pipe in the
 * chain, and an output pipe to the last pipe in the chain. This will
 * allow easy construction for buffer based io like services to for
 * API centered IO while abstracting the input and output to simple
 * data passing.
 * To use this class, you should derive a class which implements the
 * <code>build</code> method.
 */
class LLChainIOFactory
{
public:
	// Constructor
	LLChainIOFactory();

	// Destructor
	virtual ~LLChainIOFactory();

	/** 
	 * @brief Build the chian with in as the first and end as the last
	 *
	 * The caller of the LLChainIOFactory is responsible for managing
	 * the memory of the in pipe. All of the chains generated by the
	 * factory will be ref counted as usual, so the caller will also
	 * need to break the links in the chain.
	 * @param chain The chain which will have new pipes appended
	 * @param context A context for use by this factory if you choose
	 * @retrun Returns true if the call was successful.
	 */
	virtual bool build(LLPumpIO::chain_t& chain, LLSD context) const = 0;

protected:
};

/** 
 * @class LLSimpleIOFactory
 * @brief Basic implementation for making a factory that returns a
 * 'chain' of one object
 */
template<class Pipe>
class LLSimpleIOFactory : public LLChainIOFactory
{
public:
	virtual bool build(LLPumpIO::chain_t& chain, LLSD context) const
	{
		chain.emplace_back(LLIOPipe::ptr_t(new Pipe));
		return true;
	}
};

/** 
 * @class LLCloneIOFactory
 * @brief Implementation for a facory which copies a particular pipe.
 */
template<class Pipe>
class LLCloneIOFactory : public LLChainIOFactory
{
public:
	LLCloneIOFactory(Pipe* original) :
		mHandle(original),
		mOriginal(original) {}

	virtual bool build(LLPumpIO::chain_t& chain, LLSD context) const
	{
		chain.emplace_back(LLIOPipe::ptr_t(new Pipe(*mOriginal)));
		return true;
	}

protected:
	LLIOPipe::ptr_t mHandle;
	Pipe* mOriginal;
};

#endif // LL_LLCHAINIO_H
