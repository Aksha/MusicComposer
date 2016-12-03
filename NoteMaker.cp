#include "NoteMaker.h"
#include <cmath>

NoteMaker::NoteMaker(int tempo, int noteLength, int octave, int volume) {
    this->tempo      = tempo;
    this->noteLength = noteLength;
    this->octave     = octave;
    this->volume     = volume;
}

int
NoteMaker::note2Freq(const int note) const {
    double freq = 0;
    if (note > 0) {
        freq = 440 * exp(log(2) * (note - 46) / 12);
    }
    return (int) freq;
}

int
NoteMaker::getNoteTime(int noteLen, int dots) const {
    int    len  = noteLen;   // Default duration
    double time = 1.0 / len; // Default length based on temp
    // Account for dots
    while (dots--) {
        len  *= 2;
        time += (1.0 / len);
    }
    return (int) (time * 60000.0 * 4.0 / tempo);
}

AudioSignal
NoteMaker::operator()(const std::string& entry) throw (std::exception) {
    if ((entry[0] >= 'A') && (entry[0] <= 'G')) {
        // Obtain the note number and length (1 to 64) for the note length.
        std::pair<int, int> noteNlen = convert(entry);
        return AudioSignal(note2Freq(noteNlen.first), noteNlen.second, volume);
    }
    // Process entry as a meta note and obtain length of silence
    const int quietLen = processMetaNote(entry);
    return AudioSignal(1, (quietLen == 0 ? 0 : getNoteTime(quietLen)), 0);
}

std::pair<int, int>
NoteMaker::convert(const std::string& noteStr) const throw (std::exception) {
    // Convenience array to map characters to semitones
    const int SemiTones[7]  = { 9, 11, 0, 2, 4, 5, 7 }; // A,B,C,D,E,F,G
    const int NoteAdjust[3] = {1, 1, -1}; // Map #, +, and - to notes
    const std::string HPM   = "#+-";      // Hash-Plus-Minus
    // The note and note-length values determined by this method
    int note = (octave * 12) + SemiTones[noteStr[0] - 'A'] + 1;
    int len  = noteLength, dots = 0;
    // Index to track the next character to be processed.
    size_t nxtIdx = 1, sharpFlat = 0;
    // If there is another character and it is "#", "+", "-" use it.
    if ((nxtIdx < noteStr.size()) &&
        ((sharpFlat = HPM.find(noteStr[nxtIdx])) != std::string::npos)) {
        note += NoteAdjust[sharpFlat];
        nxtIdx++;
    }
    // Process note length value if specified.
    if ((nxtIdx < noteStr.size()) && (isdigit(noteStr[nxtIdx]))) {
        std::string lenStr(noteStr.substr(nxtIdx, noteStr.find('.')));
        len = toInt(lenStr);
        nxtIdx += lenStr.size();
    }
    // Finally handle any periods in the note
    if ((nxtIdx < noteStr.size()) &&
        (noteStr.substr(nxtIdx) == std::string(noteStr.size() - nxtIdx, '.'))) {
        dots = noteStr.size() - nxtIdx;
    }
    if (nxtIdx < noteStr.size()) {
        std::string msg("The following music note is invalid: ");
        msg += noteStr;
        throw std::runtime_error(msg);
    }
    return std::pair<int, int>(note, getNoteTime(len, dots));
}

int
NoteMaker::processMetaNote(const std::string& metaStr)
    throw (std::exception) {
    int pause = 0;
    switch (metaStr[0]) {
    case 'L' : noteLength = toInt(metaStr.substr(1)); break;
    case 'O' : octave     = toInt(metaStr.substr(1)); break;
    case 'T' : tempo      = toInt(metaStr.substr(1)); break;
    case 'P' : pause      = toInt(metaStr.substr(1)); break;
    case 'V' : volume     = toInt(metaStr.substr(1)); break;        
    case '<' : octave--; break;
    case '>' : octave++; break;
    case 'M' : break; // Currently MF, MB, MN, ML, MS are not handled
    }
    // note-length for a pause
    return pause;
}

int
NoteMaker::toInt(const std::string& str) throw (std::exception) {
    const char* cStr = str.c_str();
    char *endPtr     = NULL;
    int retVal = strtol(cStr, &endPtr, 10);
    if ((endPtr == NULL) || (*endPtr != '\0')) {
        std::string msg("Invalid string passed to toInt() method: ");
        msg.append(str);
        throw std::runtime_error(msg);
    }
    return retVal;
}

double
NoteMaker::toDouble(const std::string& str) throw (std::exception) {
    const char* cStr = str.c_str();
    char *endPtr     = NULL;
    double retVal    = strtod(cStr, &endPtr);
    if ((endPtr == NULL) || (*endPtr != '\0')) {
        std::string msg("Invalid string passed to toDouble() method: ");
        msg.append(str);
        throw std::runtime_error(msg);
    }
    return retVal;
}
