#include "MusicComposer.h"
#include <algorithm>
#include <iterator>
#include <iostream>
#include <functional>

AudioSignal&
MusicComposer::operator()(const std::string& ent) {
    std::string entry(ent);
    if (entry[0] == '*') {
        // Control note (not to be passed to note maker).
        process(entry);
    } else {
        // Generate signal from note maker. But first convert to upper case.
        std::transform(entry.begin(), entry.end(), entry.begin(), toupper);
        tracks[currTrack] &= nm(entry);
    }
    return tracks[currTrack];
}

void
MusicComposer::process(const std::string& ctrlNote) throw (std::exception) {
    switch(ctrlNote[1]) {
    case 't': currTrack = NoteMaker::toInt(ctrlNote.substr(2)) - 1;
        tracks.resize(currTrack + 1);
        break;
    case 'l': {
        RIFFReader data(ctrlNote.substr(2));
        tracks[currTrack] = AudioSignal(data);
        break;
    }
    case 'e': echo(NoteMaker::toInt(ctrlNote.substr(2)));
        break;
    case '=':
    case '~': equalize(NoteMaker::toInt(ctrlNote.substr(2)), ctrlNote[1]== '=');
        break;
    case '*': tracks[currTrack] *= NoteMaker::toDouble(ctrlNote.substr(2));
        break;
    case '.': getMusic();
        break;
    }
}

void
MusicComposer::echo(const int delay) {
    AudioSignal echo = (tracks[currTrack] >> delay);
    echo *= 0.25;
    tracks[currTrack] = tracks[currTrack] + echo;
}

void
MusicComposer::equalize(const int trackNum, bool max) {
    // Obtain the durations of all the audio signals in a separate vector.
    std::vector<int> durations(tracks.size());
    std::transform(tracks.begin(), tracks.end(), durations.begin(),
                   std::mem_fun_ref(&AudioSignal::duration));
    // Find the longest duration.
    const int netDuration = (max ? *max_element(durations.begin(), durations.end()) : *min_element(durations.begin(), durations.end()));
    // Change duration of specified track to maximum duration
    tracks[trackNum - 1].changeDurationTo(netDuration);
}

const AudioSignal&
MusicComposer::getMusic() {
    // Update music score by combining track 1 and track 2.
    AudioSignal combined;
    music   &= std::accumulate(tracks.begin(), tracks.end(), combined);
    // Clear out the tracks in preparation for next fragment
    tracks.clear();
    tracks.push_back(AudioSignal());
    return music;
}

int main(int argc, char *argv[]) {
    MusicComposer mc;
    std::istream_iterator<std::string> noteStream(std::cin), eof;
    mc = std::for_each(noteStream, eof, mc);
    mc.getMusic().write(argv[1]);
    return 0;
}
