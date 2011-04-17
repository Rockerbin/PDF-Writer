#include "InputPredictorPNGUpStream.h"

#include "Trace.h"


using namespace IOBasicTypes;

InputPredictorPNGUpStream::InputPredictorPNGUpStream(void)
{
	mSourceStream = NULL;
	mBuffer = NULL;
	mIndex = NULL;
	mBufferSize = 0;
	mUpValues = NULL;

}

InputPredictorPNGUpStream::~InputPredictorPNGUpStream(void)
{
	delete[] mBuffer;
	delete[] mUpValues;
	delete mSourceStream;
}

InputPredictorPNGUpStream::InputPredictorPNGUpStream(IByteReader* inSourceStream,IOBasicTypes::LongBufferSizeType inColumns)
{	
	mSourceStream = NULL;
	mBuffer = NULL;
	mIndex = NULL;
	mBufferSize = 0;
	mUpValues = NULL;

	Assign(inSourceStream,inColumns);
}


LongBufferSizeType InputPredictorPNGUpStream::Read(Byte* inBuffer,LongBufferSizeType inBufferSize)
{
	LongBufferSizeType readBytes = 0;
	

	// exhaust what's in the buffer currently
	while(mBufferSize > (LongBufferSizeType)(mIndex - mBuffer) && readBytes < inBufferSize)
	{
		DecodeNextByte(inBuffer[readBytes]);
		++readBytes;
	}

	// now repeatedly read bytes from the input stream, and decode
	while(readBytes < inBufferSize && mSourceStream->NotEnded())
	{
		memcpy(mUpValues,mBuffer,mBufferSize);

		if(mSourceStream->Read(mBuffer,mBufferSize) != mBufferSize)
		{
			TRACE_LOG("InputPredictorPNGUpStream::Read, problem, expected columns number read. didn't make it");
			readBytes = 0;
			break;
		}
		mIndex = mBuffer+1; // skip the first tag

		while(mBufferSize > (LongBufferSizeType)(mIndex - mBuffer) && readBytes < inBufferSize)
		{
			DecodeNextByte(inBuffer[readBytes]);
			++readBytes;
		}
	}
	return readBytes;
}

bool InputPredictorPNGUpStream::NotEnded()
{
	return mSourceStream->NotEnded() || (LongBufferSizeType)(mIndex - mBuffer) < mBufferSize;
}

void InputPredictorPNGUpStream::DecodeNextByte(Byte& outDecodedByte)
{
	outDecodedByte = (Byte)((char)mUpValues[mIndex-mBuffer] + (char)*mIndex);

	*mIndex = outDecodedByte; // saving the encoded value back to the buffer, for later copying as "Up value"
	++mIndex;
}

void InputPredictorPNGUpStream::Assign(IByteReader* inSourceStream,IOBasicTypes::LongBufferSizeType inColumns)
{
	mSourceStream = inSourceStream;

	delete[] mBuffer;
	delete[] mUpValues;
	mBufferSize = inColumns + 1;
	mBuffer = new Byte[mBufferSize];
	memset(mBuffer,0,mBufferSize);
	mUpValues = new Byte[mBufferSize];
	memset(mUpValues,0,mBufferSize); // that's less important
	mIndex = mBuffer + mBufferSize;

}
