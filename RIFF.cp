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

#include "RIFF.h"
#include <iostream>
#include <cstring>

// Constructor to create a RIFF wave file
RIFFReader::RIFFReader(const std::string& fileName) throw (std::exception)
    : std::ifstream(fileName.c_str()) {
    // Ensure the basic file is good.
    if (!good()) {
        throw std::runtime_error("Unable to open file for reading");
    }
    // Next try and read the RIFF header
    if (!read(reinterpret_cast<char*>(&riffHeader),sizeof(riffHeader)).good()) {
        throw std::runtime_error("Error reading RIFF header.");
    }
    // Ensure that the magic constants are as expected -- this
    // indicates it is most likely a good RIFF file.
    if ((riffHeader.riff != 0x46464952) || (riffHeader.format != 0x45564157)) {
        throw std::runtime_error("The file does not appear to be RIFF file");
    }
    // Next try and read the Format header
    if (!read(reinterpret_cast<char*>(&format), sizeof(format)).good()) {
        throw std::runtime_error("Error reading format header.");
    }
    // Ensure that the magic constants are expected -- this indicates
    // we have the expected format.
    if ((format.chunkHeader != 0x20746D66) || (format.chunkSize != 16) ||
        (format.format != 1)) {
        throw std::runtime_error("Error reading metadata. The file is not "
                                 "a valid RIFF file or it is not a PCM file");
    }
    if (format.numChannels != 1) {
        std::cerr << "Warning: The file is not a mono PCM (could be stereo) "
                  << "and only the first channel will be read.\n";
    }
    // Finally read the data header.
    if (!read(reinterpret_cast<char*>(&audio), sizeof(audio)).good()) {
        throw std::runtime_error("Error reading data header.");
    }
    // Verify the data header has the necessary magic constant.
    if (audio.header != 0x61746164) {
        throw std::runtime_error("The data header is invalid. The file is not "
                                 "a valid PCM RIFF file.");
    }
}

// Reads appropriate number of bytes from the underlying file into the
// given parameter.
RIFFReader&
RIFFReader::operator>>(Sample& s) {
    if (good() && !eof()) {
        char buffer[format.sampleSize];
        if (read(buffer, sizeof(buffer))) {
            s = 0;
            std::memcpy(reinterpret_cast<char*>(&s), buffer,
                        format.bitsPerSample / 8);
        }
    }
    return *this;
}

// Constructor to create a wave file to be written.
RIFFWriter::RIFFWriter(const std::string& fileName, int sampleRate,
                       short bitsPerSample) throw (std::exception) 
    : std::ofstream(fileName.c_str()) {
    // Ensure the basic file is good.
    if (!good()) {
        throw std::runtime_error("Unable to open file for writing");
    }
    // Setup and write the initial set of headers. These headers are
    // updated in the close method.
    riffHeader.riff      = 0x46464952;
    riffHeader.chunkSize = 0;  // To be filled-in in close() method.
    riffHeader.format    = 0x45564157;

    if (!write(reinterpret_cast<char*>(&riffHeader), sizeof(riffHeader))) {
        throw std::runtime_error("Unable to write RIFF header to file");
    }
    // Set setup and write the initial format header
    format.chunkHeader   = 0x20746D66; // Fixed constant for 4 characters "fmt "
    format.chunkSize     = sizeof(format) - (sizeof(int) * 2);
    format.format        = 1; // Fixed for PCM
    format.numChannels   = 1; // Only mono for now
    format.sampleRate    = sampleRate;
    format.byteRate      = sampleRate * format.numChannels * bitsPerSample / 8;
    format.sampleSize    = format.numChannels * bitsPerSample / 8;
    format.bitsPerSample = bitsPerSample;
    // Write the initial format header to the WAV file
    if (!write(reinterpret_cast<char*>(&format), sizeof(format))) {
        throw std::runtime_error("Unable to write format header to file");
    }
    // Finally setup the data header 
    audio.header = 0x61746164;
    audio.size   = 0; // To be filled-in later in the close method.
    // Write data header to file.
    if (!write(reinterpret_cast<char*>(&audio), sizeof(audio))) {
        throw std::runtime_error("Unable to write data header to file");
    }    
}

RIFFWriter::~RIFFWriter() {
    close();
}

// This method updates the necessary information in the header and
// closes the stream.
void
RIFFWriter::close() {
    if (good()) {
        // Update header's file size entry as we know file size now.
        riffHeader.chunkSize = sizeof(format) + sizeof(audio) + audio.size + 4;
        seekp(0); // Reset to beginning of file.
        // Rewrite headers.
        if (!write(reinterpret_cast<char*>(&riffHeader), sizeof(riffHeader))) {
            throw std::runtime_error("Unable to rewrite RIFF header to file");
        }
        if (!write(reinterpret_cast<char*>(&format), sizeof(format))) {
            throw std::runtime_error("Unable to rewrite format header to file");
        }
        if (!write(reinterpret_cast<char*>(&audio), sizeof(audio))) {
            throw std::runtime_error("Unable to rewrite data header to file");
        }    
    }
    std::ofstream::close();
}

// The stream insertion operator that only writes the necessary bytes
// to the underlying file stream.
RIFFWriter&
RIFFWriter::operator<<(const Sample s) {
    if (good()) {
        char buffer[format.sampleSize];
        std::memcpy(buffer, reinterpret_cast<const char*>(&s),
                    format.bitsPerSample / 8);
        write(buffer, sizeof(buffer));
        audio.size += format.sampleSize;
    }
    return *this;
}
