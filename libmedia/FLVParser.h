// FLVParser.h:  Flash Video file format parser, for Gnash.
//
//   Copyright (C) 2007, 2008 Free Software Foundation, Inc.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
//


// Information about the FLV format can be found at http://osflash.org/flv

#ifndef __FLVPARSER_H__
#define __FLVPARSER_H__

#include "dsodefs.h"
#include "MediaParser.h" // for inheritance

#include <vector>
#include <memory>
#include <map>

#include <boost/thread/mutex.hpp>

namespace gnash {
namespace media {

/// The FLVParser class parses FLV streams
class DSOEXPORT FLVParser : public MediaParser
{

public:

	enum tagType
	{
		FLV_AUDIO_TAG = 0x08,
		FLV_VIDEO_TAG = 0x09,
		FLV_META_TAG = 0x12
	};


	/// \brief
	/// Create an FLV parser reading input from
	/// the given IOChannel
	//
	/// @param lt
	/// 	IOChannel to use for input.
	/// 	Ownership transferred.
	///
	FLVParser(std::auto_ptr<IOChannel> lt);

	/// Kills the parser...
	~FLVParser();

	// see dox in MediaParser.h
	virtual bool seek(boost::uint32_t&);

	// see dox in MediaParser.h
	virtual bool parseNextChunk();

	// see dox in MediaParser.h
	boost::uint64_t getBytesLoaded() const;

	// see dox in MediaParser.h
	bool indexingCompleted() const
	{
		return _indexingCompleted;
	}

private:

	/// Parses next tag from the file
	//
	/// Returns true if something was parsed, false otherwise.
	/// Sets _parsingComplete=true on end of file.
	///
	bool parseNextTag();

	bool indexNextTag();

	/// Parses the header of the file
	bool parseHeader();

	// Functions used to extract numbers from the file
	inline boost::uint32_t getUInt24(boost::uint8_t* in);

	/// The position where the parsing should continue from.
	/// Will be reset on seek, and will be protected by the _streamMutex
	boost::uint64_t _lastParsedPosition;

	/// Position of next tag to index
	boost::uint64_t _nextPosToIndex;

	/// Audio frame cursor position 
	//
	/// This is the video frame number that will
	/// be referenced by nextVideoFrame and nextVideoFrameTimestamp
	///
	size_t _nextAudioFrame;

	/// Video frame cursor position 
	//
	/// This is the video frame number that will
	/// be referenced by nextVideoFrame and nextVideoFrameTimestamp
	///
	size_t _nextVideoFrame;

	/// Audio stream is present
	bool _audio;

	/// Audio stream is present
	bool _video;

	std::auto_ptr<EncodedAudioFrame> readAudioFrame(boost::uint32_t dataSize, boost::uint32_t timestamp);

	std::auto_ptr<EncodedVideoFrame> readVideoFrame(boost::uint32_t dataSize, boost::uint32_t timestamp);

	/// Position in input stream for each cue point
	/// first: timestamp
	/// second: position in input stream
	typedef std::map<boost::uint64_t, long> CuePointsMap;
	CuePointsMap _cuePoints;

	bool _indexingCompleted;
};

} // end of gnash::media namespace
} // end of gnash namespace

#endif // __FLVPARSER_H__
