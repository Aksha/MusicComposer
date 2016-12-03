//--------------------------------------------------------------------
//
// This file is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published
// by the Free Software Foundation, either version 3 of the License,
// or (at your option) any later version.
// 
// This file is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with PEACE.  If not, see <http://www.gnu.org/licenses/>.
// 
// Miami University makes no representations or warranties about the
// suitability of the software, either express or implied, including
// but not limited to the implied warranties of merchantability,
// fitness for a particular purpose, or non-infringement.  Miami
// University shall not be liable for any damages suffered by licensee
// as a result of using, result of using, modifying or distributing
// this software or its derivatives.
//
// By using or copying this Software, Licensee agrees to abide by the
// intellectual property laws, and all other applicable laws of the
// U.S., and the terms of GNU General Public License (version 3).
//
// Authors:   Dhananjai M. Rao          raodm@muohio.edu
//
//---------------------------------------------------------------------

#include <stdexcept>
#include <fstream>

/** \file RIFF.h Declearations for data structures and streams to read
	and write ".wav"/RIFF files.

	<p>This file contains various data structures and class
	definitions that provide a convenient interface to read and write
	audio files. The audio data is read and written in RIFF (Resource
	Interchange File Format) format into ".wav" (or Waveform Audio
	File Format). Refer to the following Wikipedia article for
	additional details: <a
	href="http://en.wikipedia.org/wiki/WAV">http://en.wikipedia.org/wiki/WAV</a>
	</p>

	<p>Note that currently this class provides a rather simple
	implementation of processing ".wav"/RIFF files.  Currently, the
	use of these classes is limited to:

	<ul>
	
	<li>It only handles Mono ".wav" files. For multi-channel files,
	only the first channel is used and rest of the channels are
	ignored.</li>

	<li>It assumes that the ".wav" file contains only a single audio
	stream and is in LPCM format.</li>

	</ul>

	</p>
*/

/** \typedef short Sample

	A custom type-definition which is just an alias for the C++ data
	type short. This custom type is introduced to provide a convenient
	stream extraction and stream insertion operators to read and write
	data using RIFFReader and RIFFWriter classes. 
*/
typedef short Sample;

/** \struct RIFFHeader
    
   \brief A simple C-structure to conveniently read the initial header
   from a ".wav" file.
   
   A simple C-structure to conveniently read the initial header from a
   ".wav" file.  This structure is used internally by RIFFReader and
   RIFFWriter classes.
*/
struct RIFFHeader {
    /**
       Contains ASCII for letters "RIFF" (0x46464952)
     */
    int riff;

    /**
       Essentially size of the entrie file in bytes minus 8.
    */
    int chunkSize;

    /**
       Contains ASCII for letters "WAVE" (0x45564157)
    */
    int format;
};

/** \struct Format
    
    \brief A convenient C-structure to read and process the "format" section
    of a RIFF file
    
    
    This header contains core metadata about the audio signal stored
    in a given file.  This structure is used internally by the
    RIFFReader and RIFFWriter classes.
*/
struct Format {
    /// Contains ASCII for letters "fmt " (0x20746D66)
    int   chunkHeader;
    /// Size of rest of this format header
    int   chunkSize;
    /// Audio format (must be 1 for PCM)
    short format;
    /// Mono = 1, Stereo = 2 etc.
    short numChannels;
    /// Samples per Second. 800, 44100 etc.
    int   sampleRate;
    /// sampleRate * numChannels * bitsPerSample / 8
    int   byteRate;
    /// Size of 1 sample (numChannels * BitsPerSample / 8)
    short sampleSize;
    /// Number of bits per sample
    short bitsPerSample; 
};

/** \struct Audio
    
    \brief A very simple C-structure to read the header information
    about the actual audio wave data stored in a given ".wav"
    file.

    This structure is used internally by the RIFFReader and RIFFWriter
    classes.
*/
struct Audio {
    /**
       Contains ASCII for letters "data" (0x61746164)
    */
    int  header;

    /**
       Number of bytes of wave form (actual audio signal) data.
    */
    int  size;
};

/**
   \brief A simple input stream to read wave data from a given ".wav" file.

   This class can be used to read audio wave data from a given ".wav"
   file.  Here is a simple example (without neccessary sanity checks)
   illustrating the use of this class:

   \code
   
   RIFFReader wavFile("downloaded_test_file.wav");
   Sample data;
   while ((wavFile >> data)) {
       std::cout << data << std::endl;
   }

   \endcode
*/
class RIFFReader : public std::ifstream {
public:
    /**
       Constructor to create an input stream to read audio wave data
       from a given ".wav" file

       \param[in] fileName The name of the ".wav" file from where the
       audio information is to be read.

       \exception std::exception This method throws exceptions if the
       given file could not be read or is not a valid ".wav" file.
    */
    RIFFReader(const std::string& fileName) throw (std::exception);

    /**
       Stream extraction operator to read one 8-bit or 16-bit sample.

       This method reads the next audio sample (if not end-of-file)
       from the wave file and place the sample into the given
       parameter.  This method reads either 8-bit or 16-bit sample
       depending on the sample size set for this wave file.
       
       \param[out] s This parameter is updated to reflect the value of
       the next sample read from this wave file.
    */
    RIFFReader& operator>>(Sample& s);

    /** Obtain the sample rate for this wave file.

        This method returns the sample rate set for this wave file (in
        the RIFF format header).  The sample rate determines the
        number of samples that constitute one second of audio.  For
        example if the sample rate is 22050, 22050 audio wave values
        constitute one physical second of audio.
        
        \return The sample rate for this wave file.
    */
    inline int getSampleRate() const { return format.sampleRate; }

    /** Obtain the number of bits each audio wave sample occupies.

        This method returns the number of bits of data associated with
        each audio wave sample (based on the data in the RIFF format
        header).  The sample rate is either 8-bits (that is each
        sample occupies one byte) or 16-bits (that is each sample
        occupies two bytes).  The number of bits per sample
        essentially determines the quality of aduio reproduction.
        Typically 8-bits are sufficient for low-quality human voice
        recording.  However at least 16-bits are needed for higher
        quality and is often sufficient for most practical
        applications. Read more information about this topic of
        quality via Wikipedia article: <a href="http://en.wikipedia.org/wiki/Audio_bit_depth">http://en.wikipedia.org/wiki/Audio_bit_depth</a>.

        \return The number of bits per sample of audio.
    */
    inline int getBitsPerSample() const { return format.bitsPerSample; }
    
protected:
    // Currently this class does not have any protected members
    
private:
    /**
       The actual RIFF header data read from a valid RIFF file. This
       header is populated in the constructor and never changes after
       that.
    */
    struct RIFFHeader riffHeader;

    /**
       The actual RIFF Format header data read from a RIFF file. This
       header contains core metadata about the audio signal stored in
       a given file.  This header is pouplated in the constructor and
       is never changed after that.
    */
    struct Format format;

    /**
       The header about the RIFF "data" section. This header is
       populated in the constructor and is never changed after that.
    */
    struct Audio audio;       
};

/**
   \brief  A simple output stream to write wave data to a given ".wav" file.

   A simple output stream to write wave data to a given ".wav" file.
   
   This class can be used to write audio wave data to a given ".wav"
   file.  Here is a simple example (without neccessary sanity checks)
   illustrating the use of this class:
   
   \code
   
   RIFFWriter wavFile("random.wav", 22050, 16);
   for(int i = 0; (i < 22050); i++) {
     wavFile << (Sample) (50 - (i % 100));
   }
   \endcode
*/
class RIFFWriter : public std::ofstream {
public:
    /**
       Constructor to create an output stream to write audio wave data
       to a given ".wav" file

       This method opens the given file, writes the necessary ".wav"
       headers and prepares the object for writing a Mono audio data.
       
       \param[in] fileName The name of the ".wav" file to where the
       audio information is to be written.

       \param[in] sampleRate The sampling rate to be used for the wave
       file written to this file.

       \param[in] bitsPerSample The number of bits per sample. This
       value must be either 8 or 16.  Depending on this value either 1
       byte or 2 bytes are written by operator<<() in this class.
       
       \exception std::exception This method throws exceptions if the
       given file could not be written or there are issues writing the
       necessary ".wav" headers.
    */
    RIFFWriter(const std::string& fileName, int sampleRate,
               short bitsPerSample) throw (std::exception);

    /**
       The destructor.

       The destructor calls the close() method to close the stream.
    */
    ~RIFFWriter();

    /** Method to close the stream.

        This method finally rewrites the RIFF headers to update the
        necessary file size entries in the header and closes the
        underlying file stream.
    */
    void close();

    /** Stream insertion operator to write one 8-bit or 16-bit sample.
       
       This method write the given audio sample to the wave file.
	   This method write either 8-bits or 16-bits of the given sample
	   depending on the sample size set for this wave file when it
	   was constructed.
       
       \param[in] s The audio sample to be written to this wave
	   file.
    */
    RIFFWriter& operator<<(const Sample s);
    
protected:
	// The class does not have any protected members
    
private:
    /**
       The actual RIFF header to be used to write the necessary header
       information to the wave file.
    */
    struct RIFFHeader riffHeader;

    /**
       The actual RIFF Format header to be used to write the necessary
       header information to the wave file.   This header is initially
       written in the constructor and then later on updted in the
       close() method.
    */
    struct Format format;

    /**
       The header about the RIFF "data" section. This header is
       partially pouplated and written by the constructor. It is later
       on updated and rewritten in the close() method.
    */
    struct Audio audio;
};
