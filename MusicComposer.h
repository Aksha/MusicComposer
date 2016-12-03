#include "NoteMaker.h"

class MusicComposer {
private:
    NoteMaker   nm;
    AudioSignal music;
    int         currTrack;
    std::vector<AudioSignal> tracks;

public:
    MusicComposer() : currTrack(0), tracks(1) {}
    ~MusicComposer() {}
    AudioSignal& operator()(const std::string& entry);
    const AudioSignal& getMusic();

protected:
    void process(const std::string& ctrlNote) throw (std::exception);
    void echo(const int delay);
    void equalize(const int trackNum, bool max);
};
