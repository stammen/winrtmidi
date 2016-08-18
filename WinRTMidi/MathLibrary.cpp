#include "stdafx.h"
#include "MathLibrary.h"
#include <ppltasks.h>

using namespace Windows::Foundation;
using namespace Platform;
using namespace concurrency;

namespace MathLibrary
{
	void MathFunctions::primesAsync(int start, int end, const std::function<void(double)>& callback)
	{
		auto asyncAction = mPrimes->ComputePrimesWithProgressAsync(start, end);
		asyncAction->Progress = ref new AsyncActionProgressHandler<double>([callback](IAsyncActionWithProgress<double>^ act, double progress)
		{
			callback((int)progress);
		});

	}

	void MathFunctions::cancel()
	{
		mPrimes->Cancel();
	}
}